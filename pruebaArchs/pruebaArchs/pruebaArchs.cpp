// pruebaArchs.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include "pch.h"
#include "csv.h"
#include <iostream>

using namespace csv;

int main()
{
    std::cout << "Hello World!\n"; 

	CSVFormat format;
	format.delimiter(',');
	CSVReader reader("./stuf.csv", format);
	CSVRow row;
	reader.read_row(row);

	std::cout << "t " << row["Timestamp"].get_sv();
	//in.read_header(io::ignore_extra_column, "TimeStamp", "Open", "High", "Low", "Close");
	std::string TimeStamp; double Open; double High; double Low; double Close;
	//in.read_header(io::ignore_extra_column, "TimeStamp", "Open", "High", "Low", "Close");
	//in.read_row(TimeStamp, Open, High, Low, Close);
	//std::cout << "t = " << TimeStamp ;
}

// Ejecutar programa: Ctrl + F5 o menú Depurar > Iniciar sin depurar
// Depurar programa: F5 o menú Depurar > Iniciar depuración

// Sugerencias para primeros pasos: 1. Use la ventana del Explorador de soluciones para agregar y administrar archivos
//   2. Use la ventana de Team Explorer para conectar con el control de código fuente
//   3. Use la ventana de salida para ver la salida de compilación y otros mensajes
//   4. Use la ventana Lista de errores para ver los errores
//   5. Vaya a Proyecto > Agregar nuevo elemento para crear nuevos archivos de código, o a Proyecto > Agregar elemento existente para agregar archivos de código existentes al proyecto
//   6. En el futuro, para volver a abrir este proyecto, vaya a Archivo > Abrir > Proyecto y seleccione el archivo .sln
// Timestamp,Open,High,Low,Close
