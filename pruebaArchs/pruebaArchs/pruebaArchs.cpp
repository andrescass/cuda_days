// pruebaArchs.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include "pch.h"
#include "csv.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <vector>
#include <sstream>
#include <time.h>

//#include "Workbook.h"


using namespace csv;
//using namespace SimpleXlsx;
using namespace std;

std::mutex mu;

static const string APP_VERSION = "1.1.0";


class dayClass
{
public:
	std::string day;
	std::vector<std::string> hour;
	std::vector<float> open;
	std::vector<float> high;
	std::vector<float> low;
	std::vector<float> close;

	dayClass(std::string d)
		:day(d)
	{}

	void dayPushback(std::string a, float o, float h, float l, float c)
	{
		hour.push_back(a);
		open.push_back(o);
		high.push_back(h);
		low.push_back(l);
		close.push_back(c);
		return;
	};

};

void showHelpMsg()
{
	cout << "version " << APP_VERSION << endl << endl;
	cout << "Usage: daycomp.exe -i file1.csv file2.csv -o output -t [min/sec] " << endl << endl;
	cout << "\t -i input csv files' names. Files' names must not contain white spaces" << endl;
	cout << "\t -h help. Show this message " << endl << endl;

	return;
}

void showHelpMsgAlt()
{
	cout << "version " << APP_VERSION << endl << endl;
	cout << "Usage: daycomp.exe -i file1.csv file2.csv -o output -t [min/sec] " << endl << endl;
	cout << "\t -i input csv files' names. \t If it is only one, one file mode is selected," << endl;
	cout << "\t \t If ther are more than one files, multifile mode is selected." << endl;
	cout << "\t -o outputs file name base. If no name is introduced, outputs files will be named output_low.xlsx and output_high.xlsx" << endl;
	cout << "\t -t type of processing. Options are min for minute comparing and sec for seconds comparing." << endl;
	cout << "\t \t if none is selected, software will try to determine by itself" << endl << endl;

	return;
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> result;
	std::stringstream ss(s);
	std::string item;

	while (getline(ss, item, delim)) {
		result.push_back(item);
	}

	return result;
}

vector<string> getFilenames(vector<dayClass> days, string type)
{
	vector<string> fileNames;
	for (dayClass& day : days)
	{
		string outFileName = split(day.day, ':')[1];;
		outFileName.append("_").append(split(day.day, ':')[2]).append(type).append(".csv");
		fileNames.push_back(outFileName);
	}
	return fileNames;
}

vector<string> getFilenames(vector<dayClass> days, string type, char delimiter)
{
	vector<string> fileNames;
	for (dayClass& day : days)
	{
		string outFileName = split(day.day, delimiter)[1];;
		outFileName.append("_").append(split(day.day, delimiter)[2]).append(type).append(".csv");
		fileNames.push_back(outFileName);
	}
	return fileNames;
}

void parseVector(dayClass day)
{
	ofstream outLowFile;
	ofstream outHighFile;
	string outLowFileName = split(day.day, ':')[1];
	string outHighFileName = split(day.day, ':')[1];
	outLowFileName.append("_").append(split(day.day, ':')[2]).append("low").append(".csv");
	outHighFileName.append("_").append(split(day.day, ':')[2]).append("high").append(".csv");
	outLowFile.open(outLowFileName);
	outHighFile.open(outHighFileName);
	outLowFile << fixed;
	outHighFile << fixed;
	outLowFile << "Input,Low\n";
	outHighFile << "Input,High\n";
	for (int i = 0; i < (day.hour.size()-1); i++)
	{
		for (int j = (i + 1); j < day.hour.size(); j++)
		{
			outLowFile << day.hour[i] << "-" << day.hour[j] << "," << setprecision(2) << day.low[i] - day.low[j] << "\n";
			outHighFile << day.hour[i] << "-" << day.hour[j] << "," << setprecision(2) << day.high[i] - day.high[j] << "\n";
		}
		
	}
	outLowFile.close();
	outHighFile.close();
	cout << "file " << outLowFileName << endl;
}

vector<string> createDateVector(bool type)
{
	stringstream currDate;
	vector<string> result;
	if (!type) // min type
	{
		for (int i = 0; i < 24; i++)
		{
			for (int j = 0; j < 60; j++)
			{
				if (i < 10)
				{
					currDate << "0";

				}
				currDate << i << ":";
				if (j < 10)
				{
					currDate << "0";
				}
				currDate << j << ":00";
				result.push_back(currDate.str());
				stringstream().swap(currDate);
			}
		}
	}
	else // sec type
	{
		for (int i = 0; i < 24; i++)
		{
			for (int j = 0; j < 60; j++)
			{
				for (int k = 0; k < 60; k++)
				{
					if (i < 10)
					{
						currDate << "0";

					}
					currDate << i << ":";
					if (j < 10)
					{
						currDate << "0";
					}
					currDate << j << ":";
					if (k < 10)
					{
						currDate << "0";
					}
					currDate << k;
					result.push_back(currDate.str());
					stringstream().swap(currDate);
				}
			}
		}
	}

	return result;
}

void writeOutputFile(vector<dayClass> days, string low_high, vector<string> dates)
{
	//CWorkbook book;
	//CWorksheet &sheet = book.AddSheet(low_high);

	ofstream outFile;
	ofstream outFilterFile;
	string resultFileName = "result_";
	resultFileName.append(low_high).append(".csv");
	string resultFilterFileName = "result_";
	resultFilterFileName.append(low_high).append("_filtered_").append(".csv");
	outFile.open(resultFileName);
	outFilterFile.open(resultFilterFileName);

	bool saveLine = false;
	bool saveFilterLine = false;
	bool isHigh = (low_high.compare("high") == 0) ? true : false;

	cout << "Creating results files\n";
	int secC = 0;

	/*vector<CellDataStr> headers;
	vector<CellDataStr> values;
	CellDataStr auxCell;
	CellDataDbl auxDouCell;
	auxCell.value = "input";
	headers.push_back(auxCell);
	for (dayClass& day : days)
	{
		auxCell.value = day.day;
		headers.push_back(auxCell);
	}
	sheet.AddRow(headers);*/

	// Insert rows
	stringstream ss;
	ss.clear();

	// csv readers
	vector<string> filenames = getFilenames(days, low_high, '-');
	//vector<CSVReader> readers;
	vector<ifstream> readers;
	vector<int> readSt; // indicate if the files contains remaining values. 1 values remaining, 0 end of file

	for (string& filen : filenames)
	{
		cout << filen << endl;
		readers.push_back(ifstream(filen));
		readSt.push_back(1);
		//CSVReader reada(filen, format);
		//readers.push_back(reada);
		//readers[readers.size() - 1].read_row(rowa);
		//rows.push_back(rowa);
	}

	// get first time
	string firstT = "23:59:59";
	int firstTI = 0;
	string dummyStr;
	vector<string> lines;
	for (int i = 0; i < readers.size(); i++)
	{
		getline(readers[i], dummyStr); // pop headers
		getline(readers[i], dummyStr); // first lines
		lines.push_back(dummyStr);
	}

	// parse files

	string cmpStr;
	string outStr;
	for (long int i = 0; i < dates.size() - 1; i++)
	{
		for (long int j = i + 1; j < dates.size(); j++)
		{
			cmpStr = dates[i];
			cmpStr.append("-").append(dates[j]);
			outStr = cmpStr;
			//sheet.BeginRow();
			//auxCell.value = cmpStr;
			//sheet.AddCell(auxCell);
			for (int k = 0; k < readers.size(); k++)
			{
				if (readSt[k] == 1)
				{
					if (cmpStr.compare(split(lines[k], ',')[0]) == 0) // file has the date
					{
						saveLine = true;
						/*if ((isHigh && (atof(split(lines[k], ',')[1].c_str()) > -4)) ||
							(!isHigh && (atof(split(lines[k], ',')[1].c_str()) < 4)))
							saveFilterLine = true;*/
						//auxDouCell.value = atof(split(lines[k], ',')[1].c_str());
						//sheet.AddCell(auxDouCell);
						//cout << cmpStr << endl;
						outStr.append(",").append(split(lines[k], ',')[1]);
						if (getline(readers[k], dummyStr))
						{
							lines[k] = dummyStr;
						}
						else
						{
							readSt[k] = 0;
						}
					}
					else // file not have the date
					{
						outStr.append(",").append(" ");
					}
				}
			}
			//sheet.EndRow(); //row ended
			outStr.append("\n");
			if (saveLine)
			{
				outFile << outStr;
				saveLine = false;
			}
			/*if (saveFilterLine)
			{
				outFilterFile << outStr;
				saveFilterLine = false;
			}*/
			outStr = "";
		}


		if (i % 60 == 0)
		{
			mu.lock();
			cout << low_high << " " << dates[i] << endl;
			mu.unlock();
			
		}


	}

	//for (CSVRow& row : readers[firstTI])
	/*while (getline(readers[firstTI], dummyStr))
	{
		auxCell = split(dummyStr, ',')[0];
		sheet.BeginRow();
		sheet.AddCell(auxCell);
		sheet.EndRow();
		lineI++;
		if (lineI % 1000 == 0)
			cout << "line " << lineI << endl;
	}*/

	//string resultFileName = "result_";
	//resultFileName.append(low_high).append(".xlsx");
	cout << "Writing file " << resultFileName << " to disk;\n";
	//bool bRes = book.Save(resultFileName);
	//if (bRes)   cout << "The book " << resultFileName << " has been saved successfully\n";
	//else        cout << "Saving the book " << resultFileName << " has been failed\n";
	outFile.close();
	outFilterFile.close();
}

void parseVectorDel(dayClass day, char delimiter)
{
	ofstream outLowFile;
	ofstream outHighFile;
	string outLowFileName = split(day.day, delimiter)[1];
	string outHighFileName = split(day.day, delimiter)[1];
	outLowFileName.append("_").append(split(day.day, delimiter)[2]).append("low").append(".csv");
	outHighFileName.append("_").append(split(day.day, delimiter)[2]).append("high").append(".csv");
	outLowFile.open(outLowFileName);
	outHighFile.open(outHighFileName);
	outLowFile << fixed;
	outHighFile << fixed;
	outLowFile << "Input,Low\n";
	outHighFile << "Input,High\n";
	float lowF;
	float highF;
	for (int i = 0; i < (day.hour.size() - 1); i++)
	{
		for (int j = (i + 1); j < day.hour.size(); j++)
		{
			lowF = day.low[i] - day.low[j];
			highF = day.high[i] - day.high[j];
			if(lowF < 4)
				outLowFile << day.hour[i] << "-" << day.hour[j] << "," << setprecision(2) << lowF << "\n";
			if(highF > -4)
				outHighFile << day.hour[i] << "-" << day.hour[j] << "," << setprecision(2) << highF << "\n";
		}

	}
	outLowFile.close();
	outHighFile.close();
	mu.lock();
	cout << "file " << outLowFileName << endl;
	mu.unlock();
}

void parseOneFile(string fileName)
{
	vector<dayClass> oneDay;
	CSVFormat format;
	format.delimiter(',');
	format.variable_columns(false); // Short-hand
	//format.trim({ ' ', '\t' });
	//format.variable_columns(VariableColumnPolicy::IGNORE);
	try
	{
		CSVReader reader(fileName, format);
		std::cout << "file readed \n";

		CSVRow row;
		std::string currentDay = "";
		std::vector< std::string> parsStamp;
		int dayIdx = 0;

		for (CSVRow& row : reader)
		{
			if (row[" Open Price"].is_num())//(row[" Open Price"].get<string>().compare(" None") != 0) //
			{
				parsStamp = split(row[" Time"].get<string>(), 'T');
				if (parsStamp[0].compare(currentDay) != 0) // nuevo día
				{
					currentDay = parsStamp[0];
					oneDay.push_back(dayClass(parsStamp[0]));
					dayIdx++;
					cout << currentDay << endl;
				}
				oneDay[dayIdx - 1].dayPushback(parsStamp[1], atof(row[" Open Price"].get().c_str())
					, atof(row[" High Price"].get().c_str())
					, atof(row[" Low Price"].get().c_str())
					, atof(row[" Close Price"].get().c_str()));
				
			}
		}

		// all parsed, now compare
		/*for (dayClass& d : oneDay)
			parseVectorDel(d, '-');*/
		std::vector<std::thread> t_threads;

		for (dayClass& day : oneDay)
		{
			t_threads.push_back(std::thread(parseVectorDel, day, '-'));
		}

		for (std::thread& t : t_threads)
		{
			t.join();
		}
		std::cout << "\n created " << dayIdx << " set of files \n";

		vector<string> dates = createDateVector(true);

		thread lowTh(writeOutputFile, oneDay, "low", dates);
		thread HighTh(writeOutputFile, oneDay, "high", dates);

		lowTh.join();
		HighTh.join();
	}
	catch (const char* e)
	{
		cout << e;
		exit(1);
	}

}

int main(int argc, char **argv)
{
	//std::vector<dayClass> days;
	vector<string> inputFileNames;
	string inputFileName;
	string outputFilenameBase = "output";
	bool minOrSec = true; // false for min, tru for sec
	int argCounter = 0;
	time_t t = time(NULL);
	struct tm *startT = gmtime(&t);
	struct tm *endT;

	// Argument parsing
	if (argc > 1)
	{
		for (int argi = 1; argi < argc; argi++)
		{
			if (strcmp(argv[argi], "-i") == 0)
			{
				// input files
				argi++;
				if ((argi < argc) && (argv[argi][0] != '-'))
				{
					while ((argi < argc) && (argv[argi][0] != '-'))
					{
						inputFileNames.push_back(argv[argi]);
						argi++;
					}

					if (argi < argc)
					{
						argi--;
					}
				}
				else
				{
					cout << "Error in input file name. Use -h for help" << endl;
					return 0;
				}
			}
			else if (strcmp(argv[argi], "-o") == 0)
			{
				argi++;
				if ((argi < argc) && (argv[argi][0] != '-'))
				{
					outputFilenameBase = argv[argi];
				}
				else
				{
					cout << "Error in output file name. Use -h for help" << endl;
					return 0;
				}
			}
			else if (strcmp(argv[argi], "-t") == 0)
			{
				argi++;
				if ((argi < argc))
				{
					if (strcmp(argv[argi], "min") == 0)
						minOrSec = false;
					else if (strcmp(argv[argi], "sec") == 0)
						minOrSec = true;
					else
					{
						cout << "Error in type. Use -h for help" << endl;
						return 0;
					}

				}
				else
				{
					cout << "Error in type. Use -h for help" << endl;
					return 0;
				}
			}
			else if (strcmp(argv[argi], "-h") == 0)
			{
				showHelpMsg();
				return 0;
			}
		}
	}
	else
	{
		//inputFileName = "./the super boring stuf1.csv";
		showHelpMsg();
		return 0;
	}

	if (inputFileNames.size() == 0)
	{
		cout << "There must be entered at least one input file name" << endl;
		cout << "Use -h for help" << endl;
		return 0;
	}

	//vector<string> dates = createDateVector(minOrSec);

	std::cout << "Start processing at " << startT->tm_hour << ":" << startT->tm_min << ":" << startT->tm_sec << endl;

	// One file process
	/*if (inputFileNames.size() == 1)
	{
		CSVFormat format;
		format.delimiter(',');
		format.variable_columns(false); // Short-hand
		//format.variable_columns(VariableColumnPolicy::IGNORE);
		try
		{
			CSVReader reader(inputFileName, format);
			cout << "file readed \n";

			CSVRow row;
			std::string currentDay = "";
			std::vector< std::string> parsStamp;
			int dayIdx = 0;

			for (CSVRow& row : reader)
			{
				parsStamp = split(row["Timestamp"].get<string>(), '-');
				if (parsStamp[0].compare(currentDay) != 0) // nuevo día
				{
					currentDay = parsStamp[0];
					days.push_back(dayClass(parsStamp[0]));
					dayIdx++;
					cout << currentDay << endl;
				}
				days[dayIdx - 1].dayPushback(parsStamp[1], atof(row["Open"].get().c_str()), atof(row["High"].get().c_str()), atof(row["Low"].get().c_str())
					, atof(row["Close"].get().c_str()));
			}

			// all parsed, now compare
			std::vector<std::thread> t_threads;

			for (dayClass& day : days)
			{
				t_threads.push_back(std::thread(parseVector, day));
			}

			for (std::thread& t : t_threads)
			{
				t.join();
			}
			std::cout << "\n created " << dayIdx << " set of files \n";

			// Result file. One thread per file
			thread lowTh(writeOutputFile, days, "low", dates);
			thread HighTh(writeOutputFile, days, "high", dates);

			lowTh.join();
			HighTh.join();

		}
		catch (const char* e)
		{
			cout << e;
			return 1;
		}
	}
	else //multi file */
	{
		vector<dayClass> oneDay;

		CSVFormat format;
		format.delimiter(',');
		format.variable_columns(false); // Short-hand
		int dayIdx = 0;
		//format.trim({ ' ', '\t' });
		//format.variable_columns(VariableColumnPolicy::IGNORE);
		for (string& filen : inputFileNames)
		{
			try
			{
				CSVReader reader(filen, format);
				

				CSVRow row;
				std::string currentDay = "";
				std::vector< std::string> parsStamp;
				

				for (CSVRow& row : reader)
				{
					if (row[" Open Price"].is_num())//(row[" Open Price"].get<string>().compare(" None") != 0) //
					{
						parsStamp = split(row[" Time"].get<string>(), 'T');
						if (parsStamp[0].compare(currentDay) != 0) // nuevo día
						{
							currentDay = parsStamp[0];
							oneDay.push_back(dayClass(parsStamp[0]));
							dayIdx++;
						}
						oneDay[dayIdx - 1].dayPushback(parsStamp[1], atof(row[" Open Price"].get().c_str())
							, atof(row[" High Price"].get().c_str())
							, atof(row[" Low Price"].get().c_str())
							, atof(row[" Close Price"].get().c_str()));

					}
				}

				std::cout << filen << " file readed \n";

				// all parsed, now compare
				/*for (dayClass& d : oneDay)
					parseVectorDel(d, '-');*/
				
			}
			catch (const char* e)
			{
				cout << "File error " << filen << e;
				exit(1);
			}
			//dayIdx++;
		}
		cout << "loaded " << endl;
		for (dayClass& d : oneDay)
		{
			cout << d.day << " with " << d.hour.size() << " entries " << endl;
		}
		std::vector<std::thread> t_threads;

		for (dayClass& day : oneDay)
		{
			t_threads.push_back(std::thread(parseVectorDel, day, '-'));
		}

		for (std::thread& t : t_threads)
		{
			t.join();
		}
		//std::cout << "\n created " << dayIdx << " set of files \n";

		/*vector<string> dates = createDateVector(true);

		thread lowTh(writeOutputFile, oneDay, "low", dates);
		thread HighTh(writeOutputFile, oneDay, "high", dates);

		lowTh.join();
		HighTh.join();*/

		endT = gmtime(&t);
		std::cout << "Start processing at " << startT->tm_hour << ":" << startT->tm_min << ":" << startT->tm_sec << endl;
		std::cout << "End processing at " << endT->tm_hour << ":" << endT->tm_min << ":" << endT->tm_sec << endl;
		// Parse and process each file
		/*std::vector<std::thread> t_threads;

		for (string& filen : inputFileNames)
		{
			t_threads.push_back(std::thread(parseOneFile, filen));
		}

		for (std::thread& t : t_threads)
		{
			t.join();
		}*/

		// Result file. One thread per file
		/*thread lowTh(writeOutputFile, days, "low", dates);
		thread HighTh(writeOutputFile, days, "high", dates);

		lowTh.join();
		HighTh.join();*/
	}



	//in.read_header(io::ignore_extra_column, "TimeStamp", "Open", "High", "Low", "Close");
	//std::string TimeStamp; double Open; double High; double Low; double Close;
	//in.read_header(io::ignore_extra_column, "TimeStamp", "Open", "High", "Low", "Close");
	//in.read_row(TimeStamp, Open, High, Low, Close);
	//std::cout << "t = " << TimeStamp ;
}

