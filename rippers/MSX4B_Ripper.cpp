
#include <cmath>
#include <vector>
#include <cstring>

#include "MSX4B_Ripper.h"


using namespace std;
using namespace WAV_Class;
using namespace Rippers;

#define WINDOW1ST				1.75f
#define WINDOW					1.55f
#define TOLERANZE				0.05f
#define MSX_PULSE(bauds)		((float)Z80HZ/(bauds*4))						// T-states for a pulse at 'bauds' (1200:~729.16667f 2400:364,58333)
#define bytesPerPulse(bauds)	((float)MSX_PULSE(bauds)*WAVSampleRate/Z80HZ)	// Bytes per pulse for bauds parameter on a WAV frequency
#define bytesPerBit(bauds)		(bytesPerPulse(bauds)*4)						// Wav Bytes for every Data bit for bauds parameter

#define NEXTPULSES2(pos)		" [" << states[pos] << " " << states[pos+1] << "]"
#define NEXTPULSES(pos)			" [" << states[pos] << " " << states[pos+1] << " " << states[pos+2] << " " << states[pos+3] << "]"


MSX4B_Ripper::MSX4B_Ripper(WAV *wav) : BlockRipper(wav)
{
}

MSX4B_Ripper::MSX4B_Ripper(const MSX4B_Ripper& other) : BlockRipper(other)
{
}

MSX4B_Ripper::~MSX4B_Ripper()
{
	
}

bool MSX4B_Ripper::detectBlock()
{
	block = NULL;
	DWORD initialSilence = skipSilence();

	//Initialize block
	memset(&blockInfo, 0, sizeof(BlockInfo));
	blockInfo.bitcfg = 0x24;
	blockInfo.bytecfg = 0x54;

	//Pulse width in bytes
	DWORD pulseLen = states[pos];
	//Calculate bauds from pulse width
	bauds = 0;
	if (checkPulseWidth(pulseLen, 1, 1200, 0)) bauds = 1200;
	if (checkPulseWidth(pulseLen, 1, 2400, 0)) bauds = 2400;
//	if (checkPulseWidth(pulseLen, 1, 3200, 0)) bauds = 3200;
	if (checkPulseWidth(pulseLen, 1, 3675, 0)) bauds = 3675;
	if (!bauds) {
#ifdef _DEBUG_
		cout << WAVTIME(pos) << "Bad bauds detected: " << bauds << endl;
#endif //_DEBUG_
		return false;
	}

	//Check is header
//for(int i=0;i<9*4;i++) cout<<std::dec<<(signed int)data[pos+i]<<" ";
//cout<<endl<<std::dec<<pos<<"------------------- bauds:"<<bauds<<endl;
	if (checkHeader(pos)) {
		cout << WAVTIME(pos) << "Detected MSX tone (" << std::dec << bauds << " bauds)" << endl;
		blockInfo.pilot = MSX_PULSE(bauds);
		blockInfo.bit0len = MSX_PULSE(bauds)*2;
		blockInfo.bit1len = MSX_PULSE(bauds);

		//Count header pulses
		DWORD pos2 = pos;
		float maxWidth = bytesPerPulse(bauds);
		blockInfo.pulses = 0;
		while (pos2 < states.size()) {
			pulseLen = states[pos2];
//cout << (int)states[pos2] << " ";
			if ((float)pulseLen > maxWidth*WINDOW) break;
			if (checkPulseWidth(pulseLen, 2, bauds, 0)) break;
			if (!checkPulseWidth(pulseLen, 1, bauds, 0)) break;
			if ((float)pulseLen > maxWidth) maxWidth = pulseLen;
			if (pulseLen > THRESHOLD_SILENCE) {
				blockInfo.pausems = pulseLen;
				cout << WAVTIME(pos2) << "Pulse aberration found, skip & continue with header detection..." << endl;
				break;
			}
			pos2++;
			blockInfo.pulses++;
		}
		float headerSecs = (samples[pos2]-samples[pos])/(float)WAVSampleRate;
		pos = pos2;
		cout << WAVTIME(pos) << "Header pulses: " << std::dec << blockInfo.pulses << " in " << headerSecs << " seconds" << NEXTPULSES(pos) << endl;
		if (blockInfo.pulses < 2500) {
			cout << WAVTIME(pos) << "WARNING: Detected header with very few pulses... " << endl;
		}

		//Get Data
		WORD byte = 0;
		vector<BYTE> buff;
		while (!eof()) {
			byte = getByte();
#ifdef _DEBUG_
			cout << std::dec << pos << " -> " << std::hex << byte << endl;
#endif //_DEBUG_
			if (byte==0xFFFF) break;
			buff.push_back(byte);
		}
		cout << WAVTIME(pos) << "Extracted data: " << (buff.size()) << " bytes" << endl;

		//Pulses after data block?
		pos2 = pos;
		WORD pulsesAfterData = skipToNextSilence();
		if (pulsesAfterData > 0) {
			cout << WAVTIME(pos2) << "WARNING: Skipping " << (pos-pos2) << " pulses after data block for "<< std::dec << (pulsesAfterData/(float)WAVSampleRate) << "sec" << endl;
		}

		//Pause after data in ms
		pos2 = pos;
		blockInfo.pausems = (WORD)(skipSilence() * 1000.0f / WAVSampleRate);
		if (blockInfo.pausems > 0) {
			cout << WAVTIME(pos2) << "Skip silence ("<< std::dec << blockInfo.pausems/1000.0f << "sec)" << endl;
		}

		//Constructing the Block
		if (blockInfo.pulses > 0 || buff.size() > 0) {
			block = new Block4B(
							blockInfo.pausems,
							blockInfo.pilot,
							blockInfo.pulses,
							blockInfo.bit0len,
							blockInfo.bit1len,
							blockInfo.bitcfg,
							blockInfo.bytecfg,
							(char*)buff.data(),
							buff.size());
			cout << WAVTIME(pos) << "Adding MSX 4B block" << endl;
			return true;
		}
	}

	//If no MSX block detected then add silence block if exists
	if (initialSilence/WAVSampleRate > 0) {
		cout << WAVTIME(pos) << "Adding Silence block ("<< std::dec << (initialSilence/(float)WAVSampleRate) << "sec)" << endl;
		block = new Block20(initialSilence/WAVSampleRate);
		return true;
	}

#ifdef _DEBUG_
	cout << WAVTIME(pos) << "MSX4B_Ripper->detectBlock() END (false)" << endl;
#endif //_DEBUG_
	return false;
}

/**
 * 3,4,5,6						Pulse 2400 bauds
 * 7,8,9,10,11,12,13			Pulse 1200 bauds | 2*Pulse 2400 bauds
 * 14,15,16,17,18,19,20,21,22	2*Pulse 1200 bauds
 */
bool MSX4B_Ripper::checkPulseWidth(DWORD pulseWidth, WORD pulses, DWORD bauds, bool first)
{
	float window = first ? WINDOW1ST : WINDOW;
	return pulseWidth >= bytesPerPulse(bauds)*pulses/window &&
			pulseWidth <= bytesPerPulse(bauds)*pulses*window;
}

bool MSX4B_Ripper::checkHeader(DWORD posIni)
{
	WORD pulses = 0;
	WORD maxWidth = bytesPerPulse(bauds);

	while (pulses++ < THRESHOLD_HEADER) {
		if (states[posIni] > maxWidth*WINDOW) return false;
		if (!checkPulseWidth(states[posIni], 1, bauds, 0)) return false;
		if (states[posIni] > maxWidth) maxWidth = states[posIni];
		posIni++;
	}
	return true;
}

DWORD MSX4B_Ripper::checkBit1(DWORD posIni)
{
	DWORD posAux = posIni;
/*
cout<<posAux<<" checking 1"<<endl;
for (int i=0;i<9*4;i++) 
cout<<std::dec<<(signed int)states[posAux+i]<<" ";
cout<<endl;*/

	//First Low state
//cout<<std::dec<<(signed int)states[posAux]<<endl;
	if (!checkPulseWidth(states[posAux++], 1, bauds, 1)) return 0;
	//First High state
//cout<<std::dec<<(signed int)states[posAux]<<endl;
	if (!checkPulseWidth(states[posAux++], 1, bauds, 0)) return 0;
	//2nd Low state
//cout<<std::dec<<(signed int)states[posAux]<<endl;
	if (!checkPulseWidth(states[posAux++], 1, bauds, 0)) return 0;
	//2nd High state
//cout<<std::dec<<(signed int)states[posAux]<<endl;
	if (!checkPulseWidth(states[posAux++], 1, bauds, 0)) return 0;

//cout<<posAux<<" check 1 OK"<<endl;
	return posAux - posIni;
}

DWORD MSX4B_Ripper::checkBit0(DWORD posIni)
{
	DWORD posAux = posIni;
/*
cout<<posAux<<" checking 0"<<endl;
for (int i=0;i<4;i++) 
cout<<std::dec<<(signed int)states[posAux+i]<<" ";
cout<<endl;*/

	//Long Low state
//cout<<std::dec<<(signed int)states[posAux]<<endl;
	if (!checkPulseWidth(states[posAux++], 2, bauds, 1)) return 0;
	//posAux++;
	//Long High state
//cout<<std::dec<<(signed int)states[posAux]<<endl;
	if (!checkPulseWidth(states[posAux++], 2, bauds, 0)) return 0;
//	if (!checkPulseWidth(states[posAux], 2, bauds) && !checkPulseWidth(states[posAux+1], 2, bauds)) return 0;

//cout<<posAux<<" check 0 OK"<<endl;
	return posAux - posIni;
}

/**
 * @return		User choice -> 0:Bit 0 | 1:Bit 1 | 0xFF:Abort
 */
BYTE MSX4B_Ripper::askUserForBitValue(DWORD posIni) {
	cout << endl <<
		"Bit 0    ______     Next pulse lengths" << endl <<
		"        |      " << endl <<
		"  '''''''      " << endl;
	printf("   %3d    %3d      ", states[posIni], states[posIni+1]);
	for (int i=2; i<16; i++) printf("%3d ", states[posIni+i]);
	cout << endl << endl <<
		"Bit 1 ___     ___   Next pulse lengths" << endl <<
		"     |   |   |   " << endl <<
		"  ''''   '''''   " << endl;
	printf("  %3d %3d %3d %3d  ", states[posIni], states[posIni+1], states[posIni+2], states[posIni+3]);
	for (int i=4; i<18; i++) printf("%3d ", states[posIni+i]);

	char ask;
	while (1) {
		cout << endl << endl <<
			"Some Pulse lengths are out of tolerance limits (single pulse: "<< (WORD)bytesPerPulse(bauds) << ", double: "<< (WORD)(bytesPerPulse(bauds)*2) << ")." << endl << 
			"Select correct bit value from above ([0]/[1]/[A]bort)? ";
		cin >> ask;
		if (ask == '0') return 0;
		if (ask == '1') return 1;
		if (ask == 'a' || ask == 'A') return 0xFF;
	}
}

/**
 * @return		Predictive solution, true for success and false for failure.
 */
bool MSX4B_Ripper::predictiveBitsForward(DWORD posIni, BYTE currentBit, bool bitChoice, bool useStartBit)
{
	if (!predictiveMode) return false;

	posIni += bitChoice ? 4 : 2;
	if (posIni >= states.size()) return false;

	if (verboseMode) cout << WAVTIME(posIni) << "  Prediction using value " << (bitChoice ? "1" : "0") << endl;
	DWORD bitLen0, bitLen1;
	//End the byte data
	for (int i = currentBit + 1; i < 8; i++) {
		bitLen0 = checkBit0(posIni);
		bitLen1 = checkBit1(posIni);
		if ((bitLen0 && bitLen1) || (!bitLen0 && !bitLen1)) {
			return false;
		} else
		if (bitLen0) {
			if (verboseMode) cout << WAVTIME(posIni) << "    Bit#" << (i+1) << ": 0 OK" << NEXTPULSES2(posIni) << endl;
			posIni += 2;
		} else
		if (bitLen1) {
			if (verboseMode) cout << WAVTIME(posIni) << "    Bit#" << (i+1) << ": 1 OK" << NEXTPULSES(posIni) << endl;
			posIni += 4;
		} else
			return false;
		if (posIni >= states.size()) return false;
	}
	//Check 1st stop bit
	if (posIni+4 >= states.size()) return false;
	if (!checkBit1(posIni)) return false;
	if (verboseMode) cout << WAVTIME(posIni) << "    Stop bit#1 OK " << NEXTPULSES(posIni) << endl;
	posIni += 4;
	//Check 2nd stop bit
	if (posIni+4 >= states.size()) return false;
	if (!checkBit1(posIni)) return false;
	if (verboseMode) cout << WAVTIME(posIni) << "    Stop bit#2 OK " << NEXTPULSES(posIni) << endl;
	posIni += 4;
	//Check next byte Start bit
	if (useStartBit) {
		if (posIni+2 >= states.size()) return false;
		if (!checkBit0(posIni)) return false;
		if (verboseMode) cout << WAVTIME(posIni) << "    Start bit OK " << NEXTPULSES2(posIni) << endl;
		posIni += 2;
	}

	return true;
}

/**
 * @return		Byte value or 0xFFFF if error or end on block/file
 */
WORD MSX4B_Ripper::getByte()
{
	DWORD posIni = pos;
	DWORD bitLen0, bitLen1;
	bool  predictive0, predictive1;
	WORD  value = 0;
	BYTE  mask = 1;

	//Check 0
	if (verboseMode) cout << WAVTIME(posIni) << "-------------------------------" << endl;
	if (!(bitLen0=checkBit0(posIni)) || posIni+2 >= states.size()) {
		if (detectSilence(posIni) || detectSilence(posIni+1)) {
			cout << WAVTIME(posIni) << "WARNING: Bad start bit: Silence detected. Ending block read..." << endl;
			return 0xFFFF;
		}
		cout << WAVTIME(posIni) << "WARNING: Bad start bit: Assuming value 0 and continue..." << bitLen0 << endl;
		bitLen0 = 2;
	}
	if (verboseMode) cout << WAVTIME(posIni) << "  START BIT: 0" << NEXTPULSES2(posIni) << endl;
	posIni += bitLen0;
	if (posIni >= states.size()) { value=0xFFFF; goto END; }	//EOF

	//Bit #0-7
	for (int i=0; i<8 && posIni+2<states.size(); i++) {
		bitLen0 = checkBit0(posIni);
		bitLen1 = checkBit1(posIni);

		if ((bitLen0 && bitLen1) || (!bitLen0 && !bitLen1)) {
			cout << WAVTIME(posIni) << "WARNING: Bad/Ambiguous pulse data in BIT #" << (i+1) << NEXTPULSES(posIni) << endl;
			BYTE ask = 0xFF;
			//Predictive Bits Forward
			predictive0 = predictiveBitsForward(posIni, i, 0, 0);
			predictive1 = predictiveBitsForward(posIni, i, 1, 0);
			if (predictive0 && predictive1) {
				//If ambiguity persists try to check the StartBit too...
				if (verboseMode) cout << WAVTIME(posIni) << "  Inconclusive prediction. Using next Start Bit too..." << endl;
				predictive0 = predictiveBitsForward(posIni, i, 0, 1);
				predictive1 = predictiveBitsForward(posIni, i, 1, 1);
			}

			if ((predictive0 && predictive1) || (!predictive0 && !predictive1)) {
				//Ask user for action to do (choice bit 1, bit 0, or Abort)
				if (interactiveMode) ask = askUserForBitValue(posIni);
			} else {
				if (predictive0) ask = 0;
				if (predictive1) ask = 1;
				cout << WAVTIME(posIni) << "SUCCESS: Predictive Bits Forward: Solved using BIT #" << (i+1) << " like a " << (ask ? "1" : "0") << endl;
			}

			//Set predictive/interactive result to the Bit
			if (ask == 0) {
				if (verboseMode) cout << WAVTIME(posIni) << "  BIT #" << (i+1) << ": 0" << NEXTPULSES2(posIni) << endl;
				posIni += 2;
			} else
			if (ask == 1) {
				if (verboseMode) cout << WAVTIME(posIni) << "  BIT #" << (i+1) << ": 1" << NEXTPULSES(posIni) << endl;
				posIni += 4;
				value |= mask;
			} else {
				cout << WAVTIME(posIni) << "Ending block read..." << endl;
				return 0xFFFF;
			}
		} else
		if (bitLen0) {
			if (verboseMode) cout << WAVTIME(posIni) << "  BIT #" << (i+1) << ": 0" << NEXTPULSES2(posIni) << endl;
			posIni += bitLen0;
		} else
		if (bitLen1) {
			if (verboseMode) cout << WAVTIME(posIni) << "  BIT #" << (i+1) << ": 1" << NEXTPULSES(posIni) << endl;
			posIni += bitLen1;
			value |= mask;
		}
		mask <<= 1;
	}
	if (posIni >= states.size()) { value=0xFFFF; goto END; }	//EOF

	//Check 1
	if (!(bitLen1=checkBit1(posIni))) {
		if (detectSilence(posIni) || detectSilence(posIni+1) || detectSilence(posIni+2) || detectSilence(posIni+3)) {
			cout << WAVTIME(posIni) << "WARNING: Bad 1st stop bit: Silence detected. Ending block read..." << NEXTPULSES(posIni) << endl;
			return 0xFFFF;
		}
		cout << WAVTIME(posIni) << "WARNING: Bad 1st stop bit. Assuming value 1 and continue..." << NEXTPULSES(posIni) << endl;
		bitLen1 = 4;
	}
	if (verboseMode) cout << WAVTIME(posIni) << "  STOP BIT #1: 1" << NEXTPULSES(posIni) << endl;
	posIni += bitLen1;
	if (posIni >= states.size()) { value=0xFFFF; goto END; }	//EOF

	//Check 1
	if (!(bitLen1=checkBit1(posIni))) {
		BYTE idx = 0;
		if (detectSilence(posIni) || detectSilence(posIni+(++idx)) || detectSilence(posIni+(++idx)) || detectSilence(posIni+(++idx))) {
			cout << WAVTIME(posIni) << "WARNING: Bad 2nd stop bit: Silence detected..." << NEXTPULSES(posIni) << endl;
			pos = posIni + idx;
			return value;
		}
		cout << WAVTIME(posIni) << "WARNING: Bad 2nd stop bit. Assuming value 1 and continue..." << NEXTPULSES(posIni) << endl;
		bitLen1 = 4;
	}
	if (verboseMode) cout << WAVTIME(posIni) << "  STOP BIT #2: 1" << NEXTPULSES(posIni) << endl;
	posIni += bitLen1;

END:
	pos = posIni;
	return value;
}
