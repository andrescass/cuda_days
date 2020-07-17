// pruebaArchs.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include "pch.h"
#include "csv.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include "xlsstream.h"

using namespace csv;

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

void parseVector(dayClass day)
{
	mu.lock();
	std::cout << "trhead: " << day.day;
	mu.unlock();
}

int main()
{
	std::vector<dayClass> days;
    std::cout << "Hello World!\n"; 

	xlsstream xls("important.xls");
	xls.select_sheet("Hoja 1");

	xls << "AAA";

	CSVFormat format;
	format.delimiter(',');
	CSVReader reader("./stuf.csv", format);
	CSVRow row;
	std::string currentDay = "";
	std::vector< std::string> parsStamp;
	int dayIdx = 0;

	for (CSVRow& row : reader)
	{
		parsStamp = split(row["Timestamp"], '-');
		if (parsStamp[0].compare(currentDay) != 0) // nuevo día
		{
			currentDay = parsStamp[0];
			days.push_back(dayClass(parsStamp[0]));
			dayIdx++;
		}
		days[dayIdx - 1].dayPushback(parsStamp[1], row["Open"].get<float>(), row["High"].get<float>(), row["Low"].get<float>()
			, row["Close"].get<float>());
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
	std::cout << dayIdx;



	//in.read_header(io::ignore_extra_column, "TimeStamp", "Open", "High", "Low", "Close");
	//std::string TimeStamp; double Open; double High; double Low; double Close;
	//in.read_header(io::ignore_extra_column, "TimeStamp", "Open", "High", "Low", "Close");
	//in.read_row(TimeStamp, Open, High, Low, Close);
	//std::cout << "t = " << TimeStamp ;
}

