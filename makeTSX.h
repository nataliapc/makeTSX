#ifndef __MAIN_H__
#define __MAIN_H__

/*
	0.6b	07.06.2017
	0.6.2b	22.07.2017
	0.7b	24.07.2017
	0.8.3b	12.03.2019
	0.8.4b	05.06.2020
	0.8.5b	18.10.2020
*/
#define MAKETSX_VER			"0.8.5b"
#define RELEASE_DATE		"2020.10.18"
#define MAKETSX_TEXTBLOCK	"Tape ripped by MAKETSX v" MAKETSX_VER " @ishwin74"

enum UNITS {
	UNIT_TSTATES,
	UNIT_SAMPLES,
	UNIT_HZ
};

void showInfo();
void showUsage();
void showUsage1();
void showUsage2();
const char* getError();

void doTsxMode();
void doWavMode();


#endif //__MAIN_H__
