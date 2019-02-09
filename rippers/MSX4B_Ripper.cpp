
#include <cmath>
#include <vector>

#include "MSX4B_Ripper.h"


using namespace std;
using namespace WAV_Class;
using namespace Rippers;


#define TOLERANZE				0.2f
#define MSX_PULSE(bauds)		((float)Z80HZ/(bauds*4))						// T-states for a pulse at 'bauds' (1200:~729.16667f 2400:364,58333)
#define bytesPerPulse(bauds)	((float)MSX_PULSE(bauds)*WAVSampleRate/Z80HZ)	// Bytes per pulse for sabed bauds on a WAV frequency


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

	//Pulse width in bytes
	DWORD pulseLen = getPulseWidth(pos);
	//Calculate bauds from pulse width
	bauds = 0;
	if (checkPulseWidth(pulseLen, 1, 1200)) bauds = 1200;
	if (checkPulseWidth(pulseLen, 1, 2400)) bauds = 2400;
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
		blockInfo.pulses = 0;
		while (pos2 < size) {
			pulseLen = getPulseWidth(pos2);
			if (checkPulseWidth(pulseLen, 2, bauds)) {
				break;
			}
			if (!checkPulseWidth(pulseLen, 1, bauds)) {
				if (pulseLen > THRESHOLD_SILENCE) return false;
				cout << WAVTIME(pos2) << "Pulse aberration found, skip & continue with header detection..." << endl;
			}
			pos2 += pulseLen;
			blockInfo.pulses++;
		}
		float headerSecs = (pos2-pos)/(float)WAVSampleRate;
		pos = pos2;
		cout << WAVTIME(pos) << "Header pulses: " << std::dec << blockInfo.pulses << " in " << headerSecs << " seconds" << endl;

		//Get Data
		WORD byte = 0;
		vector<BYTE> buff;
		while (pos < size) {
			byte = getByte();
#ifdef _DEBUG_
			cout << std::dec << pos << " -> " << std::hex << byte << endl;
#endif //_DEBUG_
			if (byte==0xFFFF) break;
			buff.push_back(byte);
		}

		//Pulses after data block?
		WORD pulsesAfterData = skipToNextSilence();
		if (pulsesAfterData > 0) {
			cout << WAVTIME(pos) << "WARNING: Skipping pulses after data block "<< std::dec << (pulsesAfterData/(float)WAVSampleRate) << "sec" << endl;
		}

		//Pause after data in ms
		blockInfo.pausems = (WORD)(skipSilence() * 1000.0f / WAVSampleRate);
		if (blockInfo.pausems > 0) {
			cout << WAVTIME(pos) << "Skip silence ("<< std::dec << blockInfo.pausems/1000.0f << "sec)" << endl;
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

DWORD MSX4B_Ripper::skipSilence()
{
	DWORD posAux = pos;
	DWORD pos2 = pos;

	//Detect if exists a pulse at 1200/2400 bauds, then return NO silence
	DWORD pulseLen = getPulseWidth(pos2);
	if (checkPulseWidth(pulseLen, 1, 1200) || checkPulseWidth(pulseLen, 1, 2400)) {
		return 0;
	}

	//Skip until state change
	BYTE state = getState(pos2);
	while (pos2 < size && getState(pos2)==state) {
		pos2++;
	}
	pos = pos2;
	return pos - posAux;
}

DWORD MSX4B_Ripper::skipToNextSilence()
{
	DWORD posAux = pos;
	DWORD pos2 = pos;
	DWORD pulseLen = 0;
	while (pos2<size && checkPulseWidth(pulseLen, 1, 1200) && checkPulseWidth(pulseLen, 1, 2400)) {
		pos2 += pulseLen;
		pulseLen = getPulseWidth(pos2);
	}
	pos = pos2;
	return pos - posAux;
}

bool MSX4B_Ripper::checkPulseWidth(DWORD pulseWidth, WORD pulses, DWORD bauds)
{
	return abs(bytesPerPulse(bauds)*pulses-pulseWidth) <= bytesPerPulse(bauds)*pulses*TOLERANZE;
}

bool MSX4B_Ripper::checkHeader(DWORD posIni)
{
	WORD pulses = 0;
	DWORD pulseLen;

	if (bauds!=1200 && bauds!=2400) {
#ifdef _DEBUG_
		cout << WAVTIME(pos) << "Bad bauds detected: " << bauds << endl;
#endif //_DEBUG_
		return false;
	}
	while (pulses++ < THRESHOLD_HEADER) {
		pulseLen = getPulseWidth(posIni);
		if (!checkPulseWidth(pulseLen, 1, bauds)) return false;
		posIni += pulseLen;
	}
	return true;
}

DWORD MSX4B_Ripper::checkBit1(DWORD posIni)
{
	DWORD posAux = posIni;
	DWORD pulseLen;
/*cout<<posAux<<" checking 1"<<endl;

for (int i=0;i<9*4;i++) 
cout<<std::dec<<(signed int)data[posAux+i]<<" ";
cout<<endl;
*/
	//First Low state
	pulseLen = getPulseWidth(posAux);
//cout<<std::dec<<(signed int)data[posAux]<<endl;
	if (!isLow(posAux) || !checkPulseWidth(pulseLen, 1, bauds)) return 0;
	posAux += pulseLen;
	//First High state
	pulseLen = getPulseWidth(posAux);
//cout<<std::dec<<(signed int)data[posAux]<<endl;
	if (!isHigh(posAux) || !checkPulseWidth(pulseLen, 1, bauds)) return 0;
	posAux += pulseLen;
	//2nd Low state
	pulseLen = getPulseWidth(posAux);
//cout<<std::dec<<(signed int)data[posAux]<<endl;
	if (!isLow(posAux) || !checkPulseWidth(pulseLen, 1, bauds)) return 0;
	posAux += pulseLen;
	//2nd High state
	pulseLen = getPulseWidth(posAux);
//cout<<std::dec<<(signed int)data[posAux]<<endl;
	if (!isHigh(posAux) || !checkPulseWidth(pulseLen, 1, bauds)) return 0;
	posAux += pulseLen;

cout<<posAux<<" check 1 OK"<<endl;
	return posAux - posIni;
}

DWORD MSX4B_Ripper::checkBit0(DWORD posIni)
{
	DWORD posAux = posIni;
	DWORD pulseLen;
/*cout<<posAux<<" checking 0"<<endl;

for (int i=0;i<9*4;i++) 
cout<<std::dec<<(signed int)data[posAux+i]<<" ";
cout<<endl;
*/
	//Long Low state
	pulseLen = getPulseWidth(posAux);
//cout<<std::dec<<(signed int)data[posAux]<<endl;
	if (!isLow(posAux) || !checkPulseWidth(pulseLen, 2, bauds)) return 0;
	posAux += pulseLen;
	//Long High state
	pulseLen = getPulseWidth(posAux);
//cout<<std::dec<<(signed int)data[posAux]<<endl;
	if (!isHigh(posAux) || !checkPulseWidth(pulseLen, 2, bauds)) return 0;
	posAux += pulseLen;

cout<<posAux<<" check 0 OK"<<endl;
	return posAux - posIni;
}

WORD MSX4B_Ripper::getByte()
{
	DWORD posIni = pos;
	DWORD bitLen;
	BYTE value = 0;
	BYTE mask = 1;

	//Check 0
	if (!(bitLen=checkBit0(posIni))) return 0xFFFF;
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
			return 0xFFFF;
		}
		mask <<= 1;
	}
	//Check 1
	if (!(bitLen=checkBit1(posIni))) return 0xFFFF;
	posIni += bitLen;
	//Check 1
	if (!(bitLen=checkBit1(posIni))) return 0xFFFF;
	posIni += bitLen;

	pos = posIni;
	return value;
}
