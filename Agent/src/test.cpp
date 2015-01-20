/*
 * test.cpp
 *
 *  Created on: Jan 20, 2015
 *      Author: root
 */

#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <iostream>

using namespace std;


time_t convert_time(string s) {
	struct tm tm;
	time_t now = time(0);
	tm = *gmtime(&now);
	strptime(s.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
	time_t t = mktime(&tm);
	return mktime(&tm);
}

unsigned int losujCzas()
{
	return rand() % 4 + 1;
}

int main(int argc, char **argv)
{
	if(argc != 6)
		return 0;

	srand(time(0));

	string startD(argv[1]);
	string startT(argv[2]);
	string stopD(argv[3]);
	string stopT(argv[4]);
	string calledUrl(argv[5]);

	string curlCommand = "curl " + calledUrl;

	time_t startTime = convert_time(startD + " " + startT);
	time_t stopTime = convert_time(stopD + " " + stopT);

	int counter = 0;
	time_t now;
	while((now = time(0)) < stopTime)
	{
		system(curlCommand.c_str());
		tm tm = *localtime(&now);
		cout << curlCommand << " at " << asctime(&tm) << endl;
		counter++;
		sleep(losujCzas());
	}

	cout << "Called " << counter << " times." << endl;

	return 0;
}


