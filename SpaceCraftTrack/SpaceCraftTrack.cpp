#include <iostream>
#include <fstream>
#include <ctime>
#include <random>
#include "libsgp4/SGP4.h"
#define M_PI 3.14159265358979323846

using namespace std;
// Функция для добавления случайной погрешности
double AddRandomError(double value, double error_percentage) {
	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<> dis(-error_percentage, error_percentage);
	double error = dis(gen);
	return value * (1.0 + error);
}
// Функция для идентификации космического аппарата
bool IdentifySpacecraft(double x, double y, double z, double ref_x, double ref_y, double ref_z, double threshold_percentage) {
	double dx = fabs((x - ref_x) / ref_x);
	double dy = fabs((y - ref_y) / ref_y);
	double dz = fabs((z - ref_z) / ref_z);
	return (dx < threshold_percentage) && (dy < threshold_percentage) && (dz < threshold_percentage);
}
int main() {
	// Открытие входного файла
	string tle_line1, tle_line2, time_line;
	string input_file_path = "input.txt";
	ifstream input_file(input_file_path);
	if (input_file.is_open()) {
		getline(input_file, tle_line1);
		getline(input_file, tle_line2);
		getline(input_file, time_line);
	}
	//cout << tle_line1 << tle_line2 << time_line;
	input_file.close();
	Tle tle("ISS", tle_line1, tle_line2);
	SGP4 sgp4(tle);
	// Извлечение эпохи из TLE (год и день года)
	int year = std::stoi(tle_line1.substr(18, 2)) + 2000;
	double epoch_days = std::stod(tle_line1.substr(20, 12));
	// Переменная для хранения времени
	tm tm = {};
	istringstream ss(time_line);
	ss >> std::get_time(&tm, "%H:%M:%S %d.%m.%Y");
	DateTime now(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	// Расчет положения спутника ECI
	Eci eci = sgp4.FindPosition(now);
	// Преобразование ECI в геодезические координаты (широта, долгота, высота)
	CoordGeodetic geo = eci.ToGeodetic();
	const double rad2deg = 180.0 / M_PI;
	double latitude = geo.latitude * rad2deg; // Перевод из радиан в градусы
	double longitude = geo.longitude * rad2deg; // Перевод из радиан в градусы
	// Открытие выходного файла и запись положения спутника в файл
	double error_percentage = 0.05;
	ofstream output_file("output.txt");
	if (output_file.is_open()) {
		double identification_threshold = 0.04; // 4%
		double ref_x = eci.Position().x;
		double ref_y = eci.Position().y;
		double ref_z = eci.Position().z;
		double x = AddRandomError(ref_x, error_percentage);
		double y = AddRandomError(ref_y, error_percentage);
		double z = AddRandomError(ref_z, error_percentage);
		output_file << "Эталонное положение на " << now << ":" << endl;
		output_file << "X: " << ref_x << " km" << endl;
		output_file << "Y: " << ref_y << " km" << endl;
		output_file << "Z: " << ref_z << " km" << endl;
		output_file << "Latitude: " << latitude << " degrees" << endl;
		output_file << "Longitude: " << longitude << " degrees" << endl;
		output_file << "Измеренное положение :" << endl;
		output_file << "X: " << x << " km" << endl;
		output_file << "Y: " << y << " km" << endl;
		output_file << "Z: " << z << " km" << endl;
		output_file << "Latitude: " << AddRandomError(latitude, error_percentage) << " degrees" << endl;
		output_file << "Longitude: " << AddRandomError(longitude, error_percentage) << " degrees" << endl;
		bool identified = IdentifySpacecraft(x, y, z, ref_x, ref_y, ref_z, identification_threshold);
		output_file << "Spacecraft identified: " << (identified ? "Yes" : "No") << endl;
		output_file.close();
	}
	return 0;
}