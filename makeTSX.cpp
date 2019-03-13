
#include <stdio.h>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <math.h>

#include "colors.h"
#include "makeTSX.h"
#include "TZX.h"
#include "TZX_Blocks.h"
#include "WAV.h"
#include "BlockRipper.h"
#include "rippers/B10_Standard_Ripper.h"
#include "rippers/B12_PureTone_Ripper.h"
#include "rippers/B13_PulseSequence_Ripper.h"
#include "rippers/B15_DirectRecording_Ripper.h"
#include "rippers/B20_Silence_Ripper.h"
#include "rippers/MSX4B_Ripper.h"
#include "rippers/Opera4B_Ripper.h"


using namespace std;
using namespace TZX_Class;
using namespace WAV_Class;
using namespace Rippers;

#define getError() TXT_RED << TXT_BLINK << "[ERROR]" << TXT_RESET

const char* START_TAG  = ">>================ ";
const char* FINISH_TAG = "<<================ ";
const char* BEGIN_TAG  = ">>#################### ";
const char* END_TAG    = "<<-------------------- ";

string tsxfile;
string wavfile;
bool tsxmode = false, wavmode = false;
bool onlyBlock15 = false;
bool tsxinfo = false;
bool tsxdump = false;
bool tsxhexchr = false;
bool wavnormalize = true;
bool wavenveloppe = true;
bool outnormalize = false;
bool outenveloppe = false;


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
	cout.precision(4);

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
		if (!strcasecmp(argv[i], "-v")) {
			wavmode = true;
			BlockRipper::setVerboseMode(true);
		} else
		if (!strcasecmp(argv[i], "-b15")) {
			wavmode = true;
			onlyBlock15 = true;
		} else
		if (!strcasecmp(argv[i], "-dn")) {
			wavmode = true;
			wavnormalize = false;
			outnormalize = false;
		} else
		if (!strcasecmp(argv[i], "-de")) {
			wavmode = true;
			wavenveloppe = false;
			outenveloppe = false;
		} else
		if (!strcasecmp(argv[i], "-di")) {
			wavmode = true;
			BlockRipper::setInteractiveMode(false);
		} else
		if (!strcasecmp(argv[i], "-dp")) {
			wavmode = true;
			BlockRipper::setPredictiveMode(false);
		} else
		if (!strcasecmp(argv[i], "-outn")) {
			wavmode = true;
			outnormalize = true;
			wavnormalize = true;
		} else
		if (!strcasecmp(argv[i], "-oute")) {
			wavmode = true;
			outenveloppe = true;
			wavenveloppe = true;
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
		} else {
			cout << getError() << " Unknown switch: " << argv[i] << endl << endl;
			showUsage();
			exit(1);
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
	cout << TXT_B_YELLOW;
	cout << "====================================================" << endl;
	cout << " makeTSX v" << MAKETSX_VER << " - WAV to TSX(~TZX 1.21) " << RELEASE_DATE << endl;
	cout << " Using " << TZX_LIB << " by NataliaPC" << endl;
	cout << "====================================================" << endl;
	cout << TXT_RESET;
	cout << endl;
}

/**
 * @brief 
 */
void showUsage() {
	cout << TXT_GREEN;
	cout << "Usage:" << endl;
	cout << TXT_RESET;
	cout << "       makeTSX [switchesWAV] -wav <WAV_IN_FILE> -tsx <TSX_OUT_FILE>" << endl;
	cout << "       makeTSX [switchesTSX] -tsx <TSX_IN_FILE>" << endl;
	cout << TXT_GREEN;
	cout << "SwitchesWAV:" << endl;
	cout << TXT_RESET;
	cout << "       -v    Verbose mode." << endl;
	cout << "       -di   Disable interactive mode." << endl;
	cout << "       -dp   Disable predictive mode." << endl;
	cout << "       -dn   Disable normalize audio." << endl;
	cout << "       -de   Disable enveloppe filter correction." << endl;
	cout << "       -b15  Use only blocks #15(Direct Recording) & #20(Pause)." << endl;
	cout << "       -outn Save the file 'wav_normalized.wav'." << endl;
	cout << "       -oute Save the file 'wav_envelopped.wav'." << endl;
	cout << TXT_GREEN;
	cout << "SwitchesTSX:" << endl;
	cout << TXT_RESET;
	cout << "       -i    Show TSX/TZX verbose blocks info." << endl;
	cout << "       -c    Dump TSX/TZX data blocks in hex-char format." << endl;
	cout << "       -d    Dump TSX/TZX data blocks in hexadecimal format." << endl;
	cout << endl;
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
		cout << TXT_GREEN;
		cout << "TSX/TZX Blocks info:" << endl;
		cout << "====================" << endl;
		cout << TXT_RESET;
		tsx->showInfo();
		cout << endl;
	}
	if (tsxhexchr) {
		cout << TXT_GREEN;
		cout << "TSX/TZX Blocks Hex-Char dump" << endl;
		cout << "=============================" << endl;
		cout << TXT_RESET;
		tsx->hexCharDump();
		cout << endl;
	}
	if (tsxdump) {
		cout << TXT_GREEN;
		cout << "TSX/TZX Blocks dump" << endl;
		cout << "====================" << endl;
		cout << TXT_RESET;
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
		"[Software house/publisher]",
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
	TZX  *tsx = new TZX();
	WAV  *wav = new WAV(wavfile);
	bool  msxload = false;
	float currentTime;

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
	if (wavnormalize) {
		cout << "Normalize signal..." << endl;
		wav->normalize();
		if (outnormalize) {
			cout << "Saving WAV file 'wav_normalized.wav'..." << endl;
			wav->saveToFile("wav_normalized.wav");
		}
	}

	//Envelop correction
	if (wavenveloppe) {
		cout << "Enveloppe filter correction..." << endl;
		wav->envelopeCorrection();
		if (outenveloppe) {
			cout << "Saving WAV file 'wav_envelopped.wav'..." << endl;
			wav->saveToFile("wav_envelopped.wav");
		}
	}

	//Ripppers
	Block                      *last;
	B10_Standard_Ripper        *std10 = new B10_Standard_Ripper(wav);
	B12_PureTone_Ripper        *ton12 = new B12_PureTone_Ripper(wav);
	B13_PulseSequence_Ripper   *seq13 = new B13_PulseSequence_Ripper(wav);
	B15_DirectRecording_Ripper *drc15 = new B15_DirectRecording_Ripper(wav);
	B20_Silence_Ripper         *sil20 = new B20_Silence_Ripper(wav);
	MSX4B_Ripper               *msx4b = new MSX4B_Ripper(wav);
	//Opera4B_Ripper             *ops4b = new Opera4B_Ripper(wav);
	//Dragon4B_Ripper            *drg4b = new Dragon4B_Ripper(wav);

	cout << "Detecting pulse lengths..." << endl << endl;

	cout << TXT_GREEN << START_TAG << "START RIPPING =========================" << TXT_RESET << endl;
	cout << TXT_GREEN << BEGIN_TAG << "START DETECTING BLOCK" << TXT_RESET << endl;
	WORD initialBlocks = tsx->getNumBlocks();
	while (!sil20->eof()) {
		// =========================================================
		// Block 20 Silence
		if (sil20->detectBlock()) {

			Block20 *b = (Block20*) sil20->getDetectedBlock();
			if (tsx->getNumBlocks() == initialBlocks) {
				if (!msx4b->eof()) cout << TXT_GREEN << END_TAG << "SKIP SILENCE: IS FIRST BLOCK" << TXT_RESET << endl;
			} else {
				last = tsx->getLastBlock();
				//Add silence to previous blocks if support it
				if (last!=NULL && dynamic_cast<WithPause*>(last)) {
					((WithPause*)last)->addPause(b->getPause());
					cout << TXT_GREEN << END_TAG << "SILENCE ADDED TO LAST BLOCK" << TXT_RESET << endl;
				} else {
					cout << TXT_GREEN << END_TAG << "BLOCK #" << std::hex << (int)(b->getId()) << " SILENCE RIPPED" << TXT_RESET << endl;
					tsx->addBlock(b);
				}
			}
			if (!sil20->eof()) cout << TXT_GREEN << BEGIN_TAG << "START DETECTING BLOCK" << TXT_RESET << endl;

		} else
		// =========================================================
		// Block 15 Pure Tone
		if (onlyBlock15 && drc15->detectBlock()) {

			Block15 *b = (Block15*) drc15->getDetectedBlock();
			cout << TXT_GREEN << END_TAG << "BLOCK #" << std::hex << (int)(b->getId()) << " DIRECT RECORDING BLOCK RIPPED" << TXT_RESET << endl;
			tsx->addBlock(b);
			if (!drc15->eof()) cout << TXT_GREEN << BEGIN_TAG << "START DETECTING BLOCK" << TXT_RESET << endl;

		} else
		// =========================================================
		// Block 12 Pure Tone
		if (ton12->detectBlock()) {

			Block12 *b = (Block12*) ton12->getDetectedBlock();
			cout << TXT_GREEN << END_TAG << "BLOCK #" << std::hex << (int)(b->getId()) << " PURE TONE BLOCK RIPPED" << TXT_RESET << endl;
			tsx->addBlock(b);
			if (!ton12->eof()) cout << TXT_GREEN << BEGIN_TAG << "START DETECTING BLOCK" << TXT_RESET << endl;

		} else
		// =========================================================
		// Block 10 Standard Speed Data
		if (std10->detectBlock()) {

			Block10 *b = (Block10*) std10->getDetectedBlock();
			cout << TXT_GREEN << END_TAG << "BLOCK #" << std::hex << (int)(b->getId()) << " STANDARD SPEED BLOCK RIPPED" << TXT_RESET << endl;
			tsx->addBlock(b);
			if (!std10->eof()) cout << TXT_GREEN << BEGIN_TAG << "START DETECTING BLOCK" << TXT_RESET << endl;

		} else
		// =========================================================
		// Block 4B KCS (MSX specific implementation)
		if (msx4b->detectBlock()) {
			
			Block4B *b = (Block4B*) msx4b->getDetectedBlock();
			cout << TXT_GREEN << END_TAG << "BLOCK #" << std::hex << (int)(b->getId()) << " MSX KCS RIPPED" << TXT_RESET << endl;
			//If first 4B block we must add a 0x35 block
			if (b->getId()==0x4b && !msxload) {
				if (b->getFileType() != 0xff) {
					tsx->addBlock(new Block35("MSXLOAD", b->getFileTypeLoad()));
				}
				msxload = true;
			}
			//Add the block
			tsx->addBlock(b);
			if (!msx4b->eof()) cout << TXT_GREEN << BEGIN_TAG << "START DETECTING BLOCK" << TXT_RESET << endl;

		} else
		// =========================================================
		// Block 4B OPERA (OPERA Soft Protected block)
/*		if (ops4b->detectBlock()) {
			
			Block4B *b = (Block4B*) ops4b->getDetectedBlock();
			cout << TXT_GREEN "<<------------------- BLOCK #" << std::hex << (int)(b->getId()) << " OPERA SOFT RIPPED" TXT_RESET << endl;
			tsx->addBlock(b);
			if (!ops4b->eof()) cout << TXT_GREEN ">>------------------- START DETECTING BLOCK" TXT_RESET << endl;

		} else*/
		// =========================================================
		// Block 13 Pulse Sequence
		if (seq13->detectBlock()) {

			Block13 *b = (Block13*) seq13->getDetectedBlock();
			cout << END_TAG << "BLOCK #" << std::hex << (int)(b->getId()) << " PULSE SEQUENCE RIPPED" << endl;
			tsx->addBlock(b);
			if (!seq13->eof()) cout << BEGIN_TAG << "START DETECTING BLOCK" << endl;

		} else
		// =========================================================
		// Nothing detected
		{
			if (!msx4b->eof()) {
				msx4b->incPos();
				currentTime = msx4b->getTime(-1);
				if (currentTime == floor(currentTime)) {
					cout << "[" << currentTime << "s] none found" << endl;
				}
			}
		}
	}

	//If last block have a pause remove it
	last = tsx->getLastBlock();
	if (last!=NULL && dynamic_cast<WithPause*>(last) && last->getId()!=0x20) {
		((WithPause*)last)->addPause(0);
		cout << TXT_GREEN << END_TAG << "SILENCE REMOVED FROM LAST BLOCK" << TXT_RESET << endl;
	}

	//Save TSX file
	tsx->saveToFile(tsxfile);
	cout << TXT_GREEN << FINISH_TAG << "END RIPPING ==========================" << TXT_RESET << endl;

	exit(0);
}
