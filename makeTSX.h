#ifndef __MAIN_H__
#define __MAIN_H__


#define MAKETSX_VER			"0.6b"
#define RELEASE_DATE		"2017.06.07"
#define MAKETSX_TEXTBLOCK	"Tape ripped by MAKETSX v" MAKETSX_VER " @ishwin74"

void showInfo();
void showUsage();
const char* getError();

void doTsxMode();
void doWavMode();


#endif //__MAIN_H__
