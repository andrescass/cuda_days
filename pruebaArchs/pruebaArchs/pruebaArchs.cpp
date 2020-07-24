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
#include "sqlite3.h"

//#include "Workbook.h"


using namespace csv;
//using namespace SimpleXlsx;
using namespace std;

std::mutex mu;

static const string APP_VERSION = "1.1.0";
static const string DB_FILEH_NAME = "daysh.sqlite";
static const string DB_FILEL_NAME = "daysl.sqlite";

ofstream outHFile;
ofstream outLFile;

sqlite3 *maindbh;
sqlite3 *maindbl;

static int nullCallback(void *data, int argc, char **argv, char **azColName) {
	
	return 0;
}

static int callbackH(void *data, int argc, char **argv, char **azColName) {
	/*int i;
	fprintf(stderr, "%s: ", (const char*)data);*/

	for (int i = 0; i < argc; i++) {
		outHFile << argv[i];
		if (i == 0)
			outHFile << ',';//otf << ',';
	}
	outHFile << "\n";

	return 0;
}

static int callbackL(void *data, int argc, char **argv, char **azColName) {
	/*int i;
	fprintf(stderr, "%s: ", (const char*)data);*/

	for (int i = 0; i < argc; i++) {
		outLFile << argv[i];
		if (i == 0)
			outLFile << ',';//otf << ',';
	}
	outLFile << "\n";

	return 0;
}

static int final_callback_low(void *data, int argc, char **argv, char **azColName) {
	/*int i;
	fprintf(stderr, "%s: ", (const char*)data);*/
	string sql = "INSERT INTO LOW (day) VALUES '";
	sql.append(argv[0]);
	sql.append("');");
	sqlite3_exec(maindbl, sql.c_str(), NULL, NULL, NULL);
	/*for (int i = 0; i < argc; i++) {
		outHFile << argv[i];
		if (i == 0)
			outHFile << ',';//otf << ',';
	}
	outHFile << "\n";*/

	return 0;
}

static int final_callback_high(void *data, int argc, char **argv, char **azColName) {
	/*int i;
	fprintf(stderr, "%s: ", (const char*)data);*/
	string sql = "INSERT INTO HIGH (day) VALUES '";
	sql.append(argv[0]);
	sql.append("');");
	sqlite3_exec(maindbh, sql.c_str(), NULL, NULL, NULL);
	/*for (int i = 0; i < argc; i++) {
		outHFile << argv[i];
		if (i == 0)
			outHFile << ',';//otf << ',';
	}
	outHFile << "\n";*/

	return 0;
}

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

void writeOutputFileHigh(vector<dayClass> days, vector<string> dates)
{
	//ofstream outFile;
	//ofstream outFilterFile;
	//dfsd
	sqlite3 *auxdbh;
	char *ErrMsg = 0;
	int rc;
	stringstream sql;
	string sqlS;
	const char* data = "Callback function called";

	sqlite3_open_v2(DB_FILEH_NAME.c_str(), &auxdbh, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL);

	sql << "SELECT day, COUNT(*) FROM HIGH GROUP BY day;";
	rc = sqlite3_exec(auxdbh, sql.str().c_str(), callbackH, 0, &ErrMsg);
	stringstream().swap(sql);
	if (rc)
	{
		cout << "Error inserting " << ErrMsg << endl;
	}
	
}

void writeOutputFileLow(vector<dayClass> days, vector<string> dates)
{
	//ofstream outFile;
	//ofstream outFilterFile;
	//dfsd
	sqlite3 *auxdbl;
	char *ErrMsg = 0;
	int rc;
	stringstream sql;
	string sqlS;
	const char* data = "Callback function called";

	sqlite3_open_v2(DB_FILEL_NAME.c_str(), &auxdbl, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL);

	sql << "SELECT day, COUNT(*) FROM LOW GROUP BY day;";
	rc = sqlite3_exec(auxdbl, sql.str().c_str(), callbackL, 0, &ErrMsg);
	stringstream().swap(sql);
	if (rc)
	{
		cout << "Error inserting " << ErrMsg << endl;
	}

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
	int count;
	for (long int i = 0; i < dates.size() - 1; i++)
	{
		for (long int j = i + 1; j < dates.size(); j++)
		{
			cmpStr = dates[i];
			cmpStr.append("-").append(dates[j]);
			//outStr = cmpStr;
			count = 0;
			//sheet.BeginRow();
			//auxCell.value = cmpStr;
			//sheet.AddCell(auxCell);
			for (int k = 0; k < readers.size(); k++)
			{
				if (readSt[k] == 1)
				{
					if (cmpStr.compare(split(lines[k], ',')[0]) == 0) // file has the date
					{
						//saveLine = true;
						/*if ((isHigh && (atof(split(lines[k], ',')[1].c_str()) > -4)) ||
							(!isHigh && (atof(split(lines[k], ',')[1].c_str()) < 4)))
							saveFilterLine = true;*/
						//auxDouCell.value = atof(split(lines[k], ',')[1].c_str());
						//sheet.AddCell(auxDouCell);
						//cout << cmpStr << endl;
						//outStr.append(",").append(split(lines[k], ',')[1]);
						count++;
						if (getline(readers[k], dummyStr))
						{
							lines[k] = dummyStr;
						}
						else
						{
							readSt[k] = 0;
						}
					}
					/*else // file not have the date
					{
						outStr.append(",").append(" ");
					}*/
				}
			}
			//sheet.EndRow(); //row ended
			//outStr.append("\n");
			if(count > 0) //(saveLine)
			{
				//outStr.append("\n");
				outFile << cmpStr << "," << count << "\n";
				//saveLine = false;
				count = 0;
			}
			/*if (saveFilterLine)
			{
				outFilterFile << outStr;
				saveFilterLine = false;
			}*/
			//outStr = "";
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

void parseVectorDB(dayClass day, char delimiter, sqlite3* auxdbh, sqlite3 *auxdbl)
{
	//sqlite3 *auxdbh;
	//sqlite3 *auxdbl;
	char *zErrMsg = 0;
	int rc;
	stringstream sql;
	string sqlS;
	const char* data = "Callback function called";

	//sqlite3_open_v2(DB_FILEH_NAME.c_str(), &auxdbh, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL);
	//sqlite3_open_v2(DB_FILEL_NAME.c_str(), &auxdbl, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL);
	// Delete tables if exists and create again
	sql << "DROP TABLE HIGH;";
	rc = sqlite3_exec(auxdbh, sql.str().c_str(), nullCallback, (void*)data, &zErrMsg);
	stringstream().swap(sql);
	sql << "DROP TABLE LOW;";
	rc = sqlite3_exec(auxdbl, sql.str().c_str(), nullCallback, (void*)data, &zErrMsg);
	stringstream().swap(sql);

	sql << "CREATE TABLE IF NOT EXISTS HIGH (" <<
		"id INTEGER PRIMARY KEY," <<
		"day TEXT);";
	rc = sqlite3_exec(auxdbh, sql.str().c_str(), nullCallback, (void*)data, &zErrMsg);
	stringstream().swap(sql);
	if (rc)
		cout << "Error creating table " << sqlite3_errmsg(maindbh);

	sql << "CREATE TABLE IF NOT EXISTS LOW (" <<
		"id INTEGER PRIMARY KEY," <<
		"day TEXT);";
	rc = sqlite3_exec(auxdbl, sql.str().c_str(), nullCallback, (void*)data, &zErrMsg);
	stringstream().swap(sql);
	if (rc)
		cout << "Error creating table " << sqlite3_errmsg(maindbl);

	if (!rc)
	{
		cout << "Tables created" << endl;
	}

	sqlite3_exec(auxdbl, "PRAGMA synchronous = OFF", NULL, NULL, &zErrMsg);
	sqlite3_exec(auxdbh, "PRAGMA synchronous = OFF", NULL, NULL, &zErrMsg);


	float lowF;
	float highF;
	long int counter = 0;
	vector<string> lowDays;
	vector<string> highDays;
	string pS;
	sqlite3_stmt *stmth;
	sqlite3_stmt *stmtl;

	sqlite3_prepare_v2(auxdbh, "INSERT INTO HIGH (day) VALUES (@d)", -1, &stmth, 0);
	sqlite3_prepare_v2(auxdbl, "INSERT INTO LOW (day) VALUES (@d)", -1, &stmtl, 0);

	sqlite3_exec(auxdbl, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	sqlite3_exec(auxdbh, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	for (int i = 0; i < (day.hour.size() - 1); i++)
	{
		for (int j = (i + 1); j < day.hour.size(); j++)
		{
			lowF = day.low[i] - day.low[j];
			highF = day.high[i] - day.high[j];
			if (lowF < 4)
			{
				pS = day.hour[i];
				pS.append("-").append(day.hour[j]);
				lowDays.push_back(pS);				
			}
			if (highF > -4)
			{
				pS = day.hour[i];
				pS.append("-").append(day.hour[j]);
				highDays.push_back(pS);
			}
		}
		//if (lowDays.size() > 100000)
		{
			//mu.lock();
			
			for (auto& d : lowDays)
			{
				
				//sql << "INSERT INTO LOW (day) VALUES ('" << d << "');";
				//cout << sql.str() << endl;
				//rc = sqlite3_exec(auxdbl, sql.str().c_str(), nullCallback, 0, &zErrMsg);
				//stringstream().swap(sql);
				sqlite3_bind_text(stmtl, 1, d.c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_step(stmtl);
				sqlite3_clear_bindings(stmtl);
				sqlite3_reset(stmtl);
				if (rc)
				{
					cout << "Error inserting " << zErrMsg << " " << day.day[i] << endl;
				}
				else
				{
					//cout << "commited " << d << endl;
				}
			}
			
			//mu.unlock();
			//sqlite3_exec(auxdb, "END TRANSACTION;", NULL, NULL, NULL);
			lowDays.clear();
		}

		//if (highDays.size() > 100000)
		{
			//sqlite3_exec(auxdb, "BEGIN TRANSACTION;", NULL, NULL, NULL);
			//mu.lock();
			
			for (auto& d : highDays)
			{
				//sql << "INSERT INTO HIGH (day) VALUES ('" << d << "');";
				//cout << sql.str() << endl;
				//rc = sqlite3_exec(auxdbh, sql.str().c_str(), nullCallback, 0, &zErrMsg);
				//stringstream().swap(sql);
				sqlite3_bind_text(stmth, 1, d.c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_step(stmth);
				sqlite3_clear_bindings(stmth);
				sqlite3_reset(stmth);
			}
			
			//mu.unlock();
			highDays.clear();
		}
		if(i%60 == 0)
			cout << day.hour[i] << endl;
	}
	sqlite3_exec(auxdbl, "END TRANSACTION;", NULL, NULL, NULL);
	sqlite3_exec(auxdbh, "END TRANSACTION;", NULL, NULL, NULL);
	/*sqlite3_exec(auxdb, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	for (auto& d : lowDays)
	{
		sql << "INSERT INTO LOW (day) VALUES ('" << d << "');";
		//cout << sql.str() << endl;
		rc = sqlite3_exec(auxdb, sql.str().c_str(), nullCallback, 0, &ErrMsg);
		stringstream().swap(sql);
	}
	for (auto& d : highDays)
	{
		sql << "INSERT INTO HIGH (day) VALUES ('" << d << "');";
		//cout << sql.str() << endl;
		rc = sqlite3_exec(auxdb, sql.str().c_str(), nullCallback, 0, &ErrMsg);
		stringstream().swap(sql);
	}
	sqlite3_exec(auxdb, "END TRANSACTION;", NULL, NULL, NULL);*/

	/*sqlite3_exec(auxdb, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	for (int i = 0; i < (day.hour.size() - 1); i++)
	{
		for (int j = (i + 1); j < day.hour.size(); j++)
		{
			if (counter < 100000)
			{
				lowF = day.low[i] - day.low[j];
				highF = day.high[i] - day.high[j];
				if (lowF < 4)
				{
					sql << "INSERT INTO LOW (day) VALUES ('" << day.hour[i] << "-" << day.hour[j] << "');";
					//cout << sql.str() << endl;
					rc = sqlite3_exec(auxdb, sql.str().c_str(), nullCallback, 0, &ErrMsg);
					stringstream().swap(sql);
					if (rc)
					{
						cout << "Error inserting " << ErrMsg << " " << day.day[i] << endl;
						//return;
					}
					else
					{
						cout << "commited " << day.hour[i] << endl;
					}
				}
				if (highF > -4)
				{
					sql << "INSERT INTO HIGH (day) VALUES ('" << day.hour[i] << "-" << day.hour[j] << "');";
					rc = sqlite3_exec(auxdb, sql.str().c_str(), nullCallback, 0, &ErrMsg);
					stringstream().swap(sql);
					if (rc)
						cout << "Error inserting " << ErrMsg << endl;
				}
				counter++;
			}
			else
			{
				//mu.lock();
				sqlite3_exec(auxdb, "END TRANSACTION;", NULL, NULL, NULL);
				sqlite3_exec(auxdb, "BEGIN TRANSACTION;", NULL, NULL, NULL);
				//mu.unlock();
				counter = 0;
			}				
		}
		cout << day.hour[i] << endl;
	}
	//mu.lock();
	sqlite3_exec(auxdb, "END TRANSACTION;", NULL, NULL, NULL);*/
	//mu.unlock();
	//mu.lock();
	cout << "day " << day.day << " processed" << endl;
	sqlite3_close_v2(auxdbh);
	sqlite3_close_v2(auxdbl);
	//mu.unlock();
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
			//t_threads.push_back(std::thread(parseVectorDel, day, '-'));
		}

		for (std::thread& t : t_threads)
		{
			//t.join();
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

void writeOutputLow(vector<string> lnames)
{
	char *ErrMsg = 0;
	int rc;
	string sq;

	/*for (auto& ldb : ldbs)
	{
		sqlite3_exec(ldb, "BEGIN TRANSACTION;", NULL, NULL, NULL);
		sqlite3_exec(ldb, "SELECT* FROM day", final_callback_low, NULL, NULL);
		sqlite3_exec(ldb, "END TRANSACTION;", NULL, NULL, NULL);
	}*/
	// Attach first db
	for (auto& n : lnames)
	{
		cout << n << endl;
		sq = "ATTACH DATABASE '";
		sq.append(n).append("' as data2;");
		rc = sqlite3_exec(maindbl, sq.c_str(), NULL, NULL, &ErrMsg);
		if (rc)
			cout << ErrMsg << endl;

		sqlite3_exec(maindbl, "BEGIN TRANSACTION;", NULL, NULL, NULL);
		rc = sqlite3_exec(maindbl, "INSERT INTO LOW SELECT * FROM data2.LOW", NULL, NULL, NULL);
		if (rc)
			cout << ErrMsg << endl;
		sqlite3_exec(maindbl, "END TRANSACTION;", NULL, NULL, NULL);

		rc = sqlite3_exec(maindbl, "DETACH DATABASE data2", NULL, NULL, &ErrMsg);
		if (rc)
			cout << ErrMsg << endl;
	}

	rc = sqlite3_exec(maindbl, "SELECT day, COUNT(*) FROM LOW GROUP BY day", callbackL, NULL, &ErrMsg);
	if (rc)
	{
		cout << ErrMsg << endl;
	}
}

void writeOutputHigh(vector<string> hnames)
{
	char *ErrMsg = 0;
	int rc;
	string sq;

	/*for (auto& hdb : hdbs)
	{
		sqlite3_exec(hdb, "BEGIN TRANSACTION;", NULL, NULL, NULL);
		sqlite3_exec(hdb, "SELECT* FROM day", final_callback_high, NULL, NULL);
		sqlite3_exec(hdb, "END TRANSACTION;", NULL, NULL, NULL);
	}*/

	for (auto& n : hnames)
	{
		sq = "ATTACH DATABASE '";
		sq.append(n).append("' as data2;");
		rc = sqlite3_exec(maindbh, sq.c_str(), NULL, NULL, &ErrMsg);
		if (rc)
			cout << ErrMsg << endl;

		sqlite3_exec(maindbh, "BEGIN TRANSACTION;", NULL, NULL, NULL);
		rc = sqlite3_exec(maindbh, "INSERT INTO HIGH SELECT * FROM data2.HIGH", NULL, NULL, NULL);
		if (rc)
			cout << ErrMsg << endl;
		sqlite3_exec(maindbh, "END TRANSACTION;", NULL, NULL, NULL);

		rc = sqlite3_exec(maindbh, "DETACH DATABASE data2", NULL, NULL, &ErrMsg);
		if (rc)
			cout << ErrMsg << endl;
	}

	rc = sqlite3_exec(maindbh, "SELECT day, COUNT(*) FROM HIGH GROUP BY day", callbackH, NULL, &ErrMsg);
	if (rc)
	{
		cout << ErrMsg << endl;
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
	int stH = startT->tm_hour;
	int stM = startT->tm_min;
	int stS = startT->tm_sec;
	struct tm *endT;
	//sqlite3 *maindbh;
	//sqlite3 *maindbl;
	vector<sqlite3*> hdbs;
	vector<sqlite3*> ldbs;
	int rc;
	stringstream sq;
	char *zErrMsg = 0;
	const char* data = "Callback function called";

	rc = sqlite3_open_v2(DB_FILEH_NAME.c_str(), &maindbh, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL);
	rc = sqlite3_open_v2(DB_FILEL_NAME.c_str(), &maindbl, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL);
	if (rc)
	{
		cout << "Error opening database" << endl;
		return(0);
	}

	// Delete tables if exists and create again
	sq << "DROP TABLE HIGH;";
	rc = sqlite3_exec(maindbh, sq.str().c_str(), nullCallback, (void*)data, &zErrMsg);
	stringstream().swap(sq);
	sq << "DROP TABLE LOW;";
	rc = sqlite3_exec(maindbl, sq.str().c_str(), nullCallback, (void*)data, &zErrMsg);
	stringstream().swap(sq);

	sq << "CREATE TABLE IF NOT EXISTS HIGH (" <<
		"id INTEGER PRIMARY KEY," <<
		"day TEXT);";
	rc = sqlite3_exec(maindbh, sq.str().c_str(), nullCallback, (void*)data, &zErrMsg);
	stringstream().swap(sq);
	if (rc)
		cout << "Error creating table " << sqlite3_errmsg(maindbh);

	sq << "CREATE TABLE IF NOT EXISTS LOW (" <<
		"id INTEGER PRIMARY KEY," <<
		"day TEXT);";
	rc = sqlite3_exec(maindbl, sq.str().c_str(), nullCallback, (void*)data, &zErrMsg);
	stringstream().swap(sq);
	if (rc)
		cout << "Error creating table " << sqlite3_errmsg(maindbl);

	if (!rc)
	{
		cout << "Tables created" << endl;
	}

	//sqlite3_close_v2(maindbh);
	//sqlite3_close_v2(maindbl);


	// output files visible for all
	remove("outputHigh.csv");
	remove("outputLow.csv");
	outHFile.open("outputHigh.csv", std::ofstream::app);
	outLFile.open("outputLow.csv", std::ofstream::app);

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
		//cout << "loaded " << endl;
		for (dayClass& d : oneDay)
		{
			cout << d.day << " with " << d.hour.size() << " entries " << endl;
		}

		vector<string> ldbNames;
		vector<string> hdbNames;
		// Initialize databases
		for (auto& d : oneDay)
		{
			sqlite3* auxdbl;
			sqlite3* auxdbh;
			string dbname = d.day;
			dbname.append("_l.sqlite");
			ldbNames.push_back(dbname);
			sqlite3_open_v2(dbname.c_str(), &auxdbl, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL);
			ldbs.push_back(auxdbl);
			dbname = d.day;
			dbname.append("_h.sqlite");
			hdbNames.push_back(dbname);
			sqlite3_open_v2(dbname.c_str(), &auxdbh, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL);
			hdbs.push_back(auxdbh);
		}


		std::vector<std::thread> t_threads;

		for (int di = 0; di < oneDay.size(); di++)
		{
			t_threads.push_back(std::thread(parseVectorDB, oneDay[di], '-', hdbs[di], ldbs[di]));
		}

		for (std::thread& t : t_threads)
		{
			t.join();
		}

		for (auto& d : ldbs)
		{
			sqlite3_close(d);
		}

		/*for (dayClass& day : oneDay)
		{
			t_threads.push_back(std::thread(parseVectorDel, day, '-'));
		}

		for (std::thread& t : t_threads)
		{
			t.join();
		}*/
		//std::cout << "\n created " << dayIdx << " set of files \n";

		vector<string> dates = createDateVector(true);
		thread lowTh(writeOutputHigh, hdbNames);
		thread HighTh(writeOutputLow, ldbNames);

		lowTh.join();
		HighTh.join();

		/*thread lowTh(writeOutputFileLow, oneDay, dates);
		thread HighTh(writeOutputFileHigh, oneDay, dates);

		lowTh.join();
		HighTh.join();*/

		time_t te = time(NULL);
		endT = gmtime(&te);
		std::cout << "Start processing at " << stH << ":" << stM << ":" << stS << endl;
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

