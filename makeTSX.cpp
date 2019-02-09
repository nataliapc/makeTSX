
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
#include "BlockRipper.h"
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
bool tsxhexchr = false;
bool wavnomalize = false;
bool wavenvelop = true;
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
		if (!strcasecmp(argv[i], "-c")) {
			tsxmode = true;
			tsxhexchr = true;
		} else
/*		if (!strcasecmp(argv[i], "-n")) {
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
*/		if (!strcasecmp(argv[i], "-v")) {
			wavmode = true;
			BlockRipper::setVerboseMode(true);
		} else
		if (!strcasecmp(argv[i], "-di")) {
			wavmode = true;
			BlockRipper::setInteractiveMode(false);
		} else
		if (!strcasecmp(argv[i], "-dp")) {
			wavmode = true;
			BlockRipper::setPredictiveMode(false);
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
	cout << "       -v   Verbose mode." << endl;
	cout << "       -di  Disable interactive mode." << endl;
	cout << "       -dp  Disable predictive bits forward (don't use stop bits to predict)." << endl;
//	cout << "       -n   Normalize WAV input." << endl;
//	cout << "       -e   Envelope correction." << endl;
//	cout << "       -t   Threshold factor." << endl;
//	cout << "\033[0;32m";
	cout << "SwitchesTSX:" << endl;
//	cout << "\e[0m";
	cout << "       -i   Show TSX/TZX verbose blocks info." << endl;
	cout << "       -d   Dump TSX/TZX data blocks in hexadecimal format." << endl;
	cout << "       -c   Dump TSX/TZX data blocks in hex-char format." << endl;
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
	if (tsxhexchr) {
//		cout << "\033[0;32m";
		cout << "TSX/TZX Blocks Hex-Char dump" << endl;
		cout << "============================" << endl;
//		cout << "\e[0m";
		tsx->hexCharDump();
		cout << endl;
	}
	if (tsxdump) {
//		cout << "\033[0;32m";
		cout << "TSX/TZX Blocks dump" << endl;
		cout << "====================" << endl;
//		cout << "\e[0m";
		tsx->hexDump();
		cout << endl;
	}
	exit(0);
}


/**
 * @brief 
 */
Block32* addArchiveBlock()
{
	BYTE ids[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
	string txt[] = {
		"[Full title]",
		"[Software house/publicher]",
		"[Autor(s)]",
		"[Year of publication]",
		"[Language]",
		"[Game/utility type]",
		"[Price]",
		"[Protection scheme/loader]",
		"[Origin]"
	};
	return new Block32(9, ids, txt);
}

/**
 * @brief 
 */
void doWavMode()
{
	TZX *tsx = new TZX();
	WAV *wav = new WAV(wavfile);
	bool msxload = false;

	//Check Input WAV file
	if (!wav || !wav->getSize()) {
		cout << getError() << " Error loading WAV file..." << endl << endl;
		exit(1);
	}
	wav->showInfo();
	if (wav->header->fmtSize!=16 ||
		wav->header->wFormatTag!=1 ||
		wav->header->nChannels!=1 ||
		(wav->header->wBitsPerSample!=8 && wav->header->wBitsPerSample!=16)) {
		cout << endl << getError() << " WAV file must be in PCM/Mono and 8/16bits mode..." << endl << endl;
		exit(1);
	}
	cout << endl;
	
	//Add 1st block with text info about this ripper
	tsx->addBlock(new Block30(MAKETSX_TEXTBLOCK));
	//Add the default Archive block for this tape
	tsx->addBlock(addArchiveBlock());

	//Normalize signal
	if (wavnomalize) {
		cout << "Normalize signal..." << endl;
		wav->normalize();
	}

	//Envelop correction
	if (wavenvelop) {
		cout << "Envelop correction..." << endl;
		wav->envelopeCorrection();
	}

	//Ripppers
	MSX4B_Ripper *msx4b = new MSX4B_Ripper(wav);
	cout << "Detecting pulse lengths..." << endl << endl;

	cout << ">>------------------- START RIPPING ---------------------" << endl;
	cout << ">>------------------- START DETECTING BLOCK" << endl;
	while (!msx4b->eof()) {
		// =========================================================
		// Block 4B KCS (MSX specific implementation)
		if (msx4b->detectBlock()) {
			Block4B *b = (Block4B*) msx4b->getDetectedBlock();
			cout << "<<------------------- BLOCK #" << std::hex << (int)(b->getId()) << " RIPPED" << endl;
			if (!msx4b->eof()) cout << ">>------------------- START DETECTING BLOCK" << endl;
			if (!msxload) {
				if (b->getFileType() != 0xff) {
					tsx->addBlock(new Block35("MSXLOAD", b->getFileTypeLoad()));
				}
				msxload = true;
			}
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
