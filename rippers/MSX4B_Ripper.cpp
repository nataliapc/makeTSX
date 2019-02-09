
#include <cmath>
#include <vector>
#include <cstring>

#include "MSX4B_Ripper.h"


using namespace std;
using namespace WAV_Class;
using namespace Rippers;


#define WINDOW					1.5f
#define TOLERANZE				0.05f
#define MSX_PULSE(bauds)		((float)Z80HZ/(bauds*4))						// T-states for a pulse at 'bauds' (1200:~729.16667f 2400:364,58333)
#define bytesPerPulse(bauds)	((float)MSX_PULSE(bauds)*WAVSampleRate/Z80HZ)	// Bytes per pulse for bauds parameter on a WAV frequency
#define bytesPerBit(bauds)		(bytesPerPulse(bauds)*4)						// Wav Bytes for every Data bit for bauds parameter


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
	if (checkPulseWidth(pulseLen, 1, 1200)) bauds = 1200;
	if (checkPulseWidth(pulseLen, 1, 2400)) bauds = 2400;
	if (bauds!=1200 && bauds!=2400) {
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
		WORD maxWidth = bytesPerPulse(bauds);
		blockInfo.pulses = 0;
		while (pos2 < states.size()) {
			pulseLen = states[pos2];
//cout << (int)states[pos2] << " ";
			if (pulseLen > maxWidth*WINDOW) break;
			if (!checkPulseWidth(pulseLen, 1, bauds)) break;
			if (pulseLen > maxWidth) maxWidth = pulseLen;
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
		cout << WAVTIME(pos) << "Header pulses: " << std::dec << blockInfo.pulses << " in " << headerSecs << " seconds" << endl;
		if (blockInfo.pulses < 3000) {
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
bool MSX4B_Ripper::checkPulseWidth(DWORD pulseWidth, WORD pulses, DWORD bauds)
{
	return pulseWidth <= bytesPerPulse(bauds)*pulses*WINDOW;
}

bool MSX4B_Ripper::checkHeader(DWORD posIni)
{
	WORD pulses = 0;
	WORD maxWidth = bytesPerPulse(bauds);

	while (pulses++ < THRESHOLD_HEADER) {
		if (states[posIni] > maxWidth*WINDOW) return false;
		if (!checkPulseWidth(states[posIni], 1, bauds)) return false;
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
	if (!checkPulseWidth(states[posAux++], 1, bauds)) return 0;
	//First High state
//cout<<std::dec<<(signed int)states[posAux]<<endl;
	if (!checkPulseWidth(states[posAux++], 1, bauds)) return 0;
	//2nd Low state
//cout<<std::dec<<(signed int)states[posAux]<<endl;
	if (!checkPulseWidth(states[posAux++], 1, bauds)) return 0;
	//2nd High state
//cout<<std::dec<<(signed int)states[posAux]<<endl;
	if (!checkPulseWidth(states[posAux++], 1, bauds)) return 0;

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
	if (!checkPulseWidth(states[posAux++], 2, bauds)) return 0;
	//Long High state
//cout<<std::dec<<(signed int)states[posAux]<<endl;
	if (!checkPulseWidth(states[posAux++], 2, bauds)) return 0;

//cout<<posAux<<" check 0 OK"<<endl;
	return posAux - posIni;
}

WORD MSX4B_Ripper::getByte()
{
	DWORD posIni = pos;
	DWORD bitLen;
	BYTE value = 0;
	BYTE mask = 1;

	//Check 0
	if (!(bitLen=checkBit0(posIni))) {
		if (detectSilence(posIni) || detectSilence(posIni+1)) {
			cout << WAVTIME(pos) << "WARNING: Bad start bit: Silence detected. Ending block read..." << endl;
			return 0xFFFF;
		}
		cout << WAVTIME(pos) << "WARNING: Bad start bit: Assuming value 0 and continue..." << endl;
	}
	posIni += bitLen;
	//Bit #0-7
	for (int i=0; i<8; i++) {
		if ((bitLen=checkBit1(posIni))) {
			posIni += bitLen;
			value |= mask;
		} else
		if ((bitLen=checkBit0(posIni))) {
			posIni += bitLen;
		} else {
			cout << WAVTIME(pos) << "WARNING: Bad byte data in bit #" << i << ": Ending block read..." << endl;
			return 0xFFFF;
		}
		mask <<= 1;
	}
	//Check 1
	if (!(bitLen=checkBit1(posIni))) {
		if (detectSilence(posIni) || detectSilence(posIni+1) || detectSilence(posIni+2) || detectSilence(posIni+3)) {
			cout << WAVTIME(pos) << "WARNING: Bad 1st stop bit: Silence detected. Ending block read..." << endl;
			return 0xFFFF;
		}
		cout << WAVTIME(pos) << "WARNING: Bad 1st stop bit. Assuming value 1 and continue..." << endl;
	}
	posIni += bitLen;
	//Check 1
	if (!(bitLen=checkBit1(posIni))) {
		if (detectSilence(posIni) || detectSilence(posIni+1) || detectSilence(posIni+2) || detectSilence(posIni+3)) {
			cout << WAVTIME(pos) << "WARNING: Bad 2nd stop bit: Silence detected. Ending block read..." << endl;
			return 0xFFFF;
		}
		cout << WAVTIME(pos) << "WARNING: Bad 2nd stop bit. Assuming value 1 and continue..." << endl;
	}
	posIni += bitLen;

	pos = posIni;
	return value;
}
