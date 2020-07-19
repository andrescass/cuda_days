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
#include "Workbook.h"


using namespace csv;
using namespace SimpleXlsx;
using namespace std;

std::mutex mu;


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

void writeOutputFile(vector<dayClass> days, string low_high, vector<string> dates)
{
	CWorkbook book;
	CWorksheet &sheet = book.AddSheet(low_high);

	cout << "Creating results file\n";

	vector<CellDataStr> headers;
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
	sheet.AddRow(headers);

	// Insert rows
	stringstream ss;
	ss.clear();

	// csv readers
	vector<string> filenames = getFilenames(days, low_high);
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
	string firstT = "23:59:00";
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
	for (int i = 0; i < dates.size()-1; i++)
	{
		for (int j = i+1; j < dates.size(); j++)
		{
			cmpStr = dates[i];
			cmpStr.append("-").append(dates[j]);
			sheet.BeginRow();
			auxCell.value = cmpStr;
			sheet.AddCell(auxCell);
			for (int k = 0; k < readers.size(); k++)
			{
				if (readSt[k] == 1)
				{
					if (cmpStr.compare(split(lines[k], ',')[0]) == 0) // file has the date
					{
						auxDouCell.value = atof(split(lines[k], ',')[1].c_str());
						sheet.AddCell(auxDouCell);
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
						auxCell.value = "";
						sheet.AddCell(auxCell);
					}
				}
			}
			sheet.EndRow(); //row ended
		}
		/*
		mu.lock();
		cout << low_high << " " << dates[i] << endl;
		mu.unlock();
		*/
			
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
	
	string resultFileName = "result_";
	resultFileName.append(low_high).append(".xlsx");
	cout << "Writing file " << resultFileName << " to disk;\n";
	bool bRes = book.Save(resultFileName);
	if (bRes)   cout << "The book " << resultFileName << " has been saved successfully\n";
	else        cout << "Saving the book " << resultFileName << " has been failed\n";
}

vector<string> createDateVector()
{
	stringstream currDate;
	vector<string> result;
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

	return result;
}

int main(int argc, char **argv)
{
	std::vector<dayClass> days;
	vector<string> dates = createDateVector();
	
    std::cout << "Start processing\n"; 

	string inputFileName;
	if (argc > 1)
	{
		inputFileName = argv[1];
	}
	else
	{
		inputFileName = "./the super boring stuf1.csv";
	}

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
		std::cout << "\n created "<< dayIdx << " set of files \n";

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



	//in.read_header(io::ignore_extra_column, "TimeStamp", "Open", "High", "Low", "Close");
	//std::string TimeStamp; double Open; double High; double Low; double Close;
	//in.read_header(io::ignore_extra_column, "TimeStamp", "Open", "High", "Low", "Close");
	//in.read_row(TimeStamp, Open, High, Low, Close);
	//std::cout << "t = " << TimeStamp ;
}

