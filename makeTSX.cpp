
#include <stdio.h>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include "makeTSX.h"
#include "TZX.h"
#include "TZX_Blocks.h"
#include "WAV.h"
#include "rippers/MSX4B_Ripper.h"


using namespace std;
using namespace TZX_Class;
using namespace WAV_Class;
using namespace Rippers;


string tsxfile;
string wavfile;
bool tsxmode = false, wavmode = false;
bool tsxinfo = false;
bool tsxdump = false;
bool wavnomalize = false;
bool wavenvelop = false;
bool wavthreshold = false;


/**
 * @brief Main entry point
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char* argv[])
{
	//Uppercase hex digits
    cout.setf(cout.uppercase);
	//Float 2 decimals
	cout.setf(ios::fixed);
	cout.precision(3);

	showInfo();

	//Arguments
	for (int i=1; i<argc; i++) {
		if (!strcasecmp(argv[i], "-i")) {
			tsxmode = true;
			tsxinfo = true;
		} else
		if (!strcasecmp(argv[i], "-d")) {
			tsxmode = true;
			tsxdump = true;
		} else
		if (!strcasecmp(argv[i], "-n")) {
			wavmode = true;
			wavnomalize = true;
		} else
		if (!strcasecmp(argv[i], "-e")) {
			wavmode = true;
			wavenvelop = true;
		} else
		if (!strcasecmp(argv[i], "-t")) {
			wavmode = true;
			wavthreshold = true;
		} else
		if (!strcasecmp(argv[i], "-wav")) {
			wavmode = true;
			if (++i < argc) {
				wavfile = argv[i];
			}
		} else
		if (!strcasecmp(argv[i], "-tsx")) {
			if (++i < argc) {
				tsxfile = argv[i];
			}
		}
	}
	
	if (tsxmode && wavmode) {
		cout << getError() << " Incompatible switches at same time..." << endl << endl;
		showUsage();
		exit(1);
	}

	if (tsxmode) {
		if (tsxfile=="") {
			cout << getError() << " Input filename for TSX/TZX needed [use -tsx switch]..." << endl << endl;
			showUsage();
			exit(1);
		}
		doTsxMode();
	}

	if (wavmode) {
		if (wavfile=="") {
			cout << getError() << " Input filename for WAV needed [use -wav switch]..." << endl << endl;
			showUsage();
			exit(-1);
		}
		if (tsxfile=="") {
			cout << getError() << " Output filename for TSX/TZX needed [use -tsx switch]..." << endl << endl;
			showUsage();
			exit(1);
		}
		doWavMode();
	}

	showUsage();
}

/**
 * @brief 
 */
void showInfo()
{
//	cout << "\033[0;32m";
	cout << "==================================================" << endl;
	cout << " makeTSX v" << MAKETSX_VER << " - WAV to TSX(~TZX 1.21) " << RELEASE_DATE << endl;
	cout << " Using " << TZX_LIB << " by NataliaPC" << endl;
	cout << "==================================================" << endl;
//	cout << "\e[0m";
	cout << endl;
}

/**
 * @brief 
 */
void showUsage() {
//	cout << "\033[0;32m";
	cout << "Usage:" << endl;
//	cout << "\e[0m";
	cout << "       makeTSX [switchesWAV] -wav <WAV_IN_FILE> -tsx <TSX_OUT_FILE>" << endl;
	cout << "       makeTSX [switchesTSX] -tsx <TSX_IN_FILE>" << endl;
//	cout << "\033[0;32m";
	cout << "SwitchesWAV:" << endl;
//	cout << "\e[0m";
	cout << "       none in current version" << endl;
//	cout << "       -n   Normalize WAV input." << endl;
//	cout << "       -e   Envelope correction." << endl;
//	cout << "       -t   Threshold factor." << endl;
//	cout << "\033[0;32m";
	cout << "SwitchesTSX:" << endl;
//	cout << "\e[0m";
	cout << "       -i   Show TSX/TZX verbose blocks info." << endl;
	cout << "       -d   Dump TSX/TZX blocks hexadecimal data." << endl;
	cout << endl;
}

const char* getError() {
//	return (const char*)"\033[1;31m[ERROR]\e[0m";
	return (const char*)"[ERROR]";
}

/**
 * @brief 
 */
void doTsxMode()
{
	TZX *tsx = new TZX(tsxfile);
	if (!tsx || !tsx->getNumBlocks()) {
		cout << getError() << " Error loading TSX file..." << endl << endl;
		exit(1);
	}
	if (tsxinfo) {
//		cout << "\033[0;32m";
		cout << "TSX/TZX Blocks info:" << endl;
		cout << "====================" << endl;
//		cout << "\e[0m";
		tsx->showInfo();
		cout << endl;
	}
	if (tsxdump) {
//		cout << "\033[0;32m";
		cout << "TSX/TZX Blocks dump" << endl;
		cout << "====================" << endl;
//		cout << "\e[0m";
		tsx->dump();
		cout << endl;
	}
	exit(0);
}

/**
 * @brief 
 */
void doWavMode()
{
	TZX *tsx = new TZX();
	WAV *wav = new WAV(wavfile);

	if (!wav || !wav->getSize()) {
		cout << getError() << " Error loading WAV file..." << endl << endl;
		exit(1);
	}
	wav->showInfo();
	cout << endl;
	
	//Envelop correction
	wav->envelopeCorrection();
	cout << "Envelop correction done..." << endl;

	//Ripppers
	MSX4B_Ripper *msx4b = new MSX4B_Ripper(wav);
	cout << "Detecting pulses..." << endl << endl;

	cout << ">>------------------- START RIPPING ---------------------" << endl;
	cout << ">>------------------- START DETECTING BLOCK" << endl;
	while (!msx4b->eof()) {
		// =========================================================
		// Block 4B KCS (MSX specific implementation)
		if (msx4b->detectBlock()) {
			Block *b = msx4b->getDetectedBlock();
			cout << "<<------------------- BLOCK #" << std::hex << (int)(b->getId()) << " RIPPED" << endl;
			if (!msx4b->eof()) cout << ">>------------------- START DETECTING BLOCK" << endl;
			tsx->addBlock(b);
//tsx->saveToFile(tsxfile);
		} else {
			if (!msx4b->eof()) msx4b->incPos();
		}
	}
	tsx->saveToFile(tsxfile);
	cout << "<<-------------------- END RIPPING ----------------------" << endl;

	exit(0);
}

