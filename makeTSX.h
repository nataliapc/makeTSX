#ifndef __MAIN_H__
#define __MAIN_H__

/*
	0.6b	07.06.2017
	0.6.2b	22.07.2017
	0.7b	24.07.2017
*/
#define MAKETSX_VER			"0.8.3b"
#define RELEASE_DATE		"2019.03.12"
#define MAKETSX_TEXTBLOCK	"Tape ripped by MAKETSX v" MAKETSX_VER " @ishwin74"

void showInfo();
void showUsage();
void showUsage1();
const char* getError();

void doTsxMode();
void doWavMode();


#endif //__MAIN_H__
