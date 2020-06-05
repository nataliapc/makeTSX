
#include <cmath>
#include <vector>
#include <cstring>

#include "Opera4B_Ripper.h"
#include "MSX4B_Ripper.h"


using namespace std;
using namespace WAV_Class;
using namespace Rippers;

// getByte() return values
#define RET_END					0xFFF0
#define RET_ABORT				0xFFF1
#define RET_PILOT				0xFFFE
#define RET_SILENCE				0xFFFF


Opera4B_Ripper::Opera4B_Ripper(WAV *wav) : MSX4B_Ripper(wav)
{
}

Opera4B_Ripper::Opera4B_Ripper(const Opera4B_Ripper& other) : MSX4B_Ripper(other)
{
}

Opera4B_Ripper::~Opera4B_Ripper()
{
}

bool Opera4B_Ripper::detectBlock()
{
	block = NULL;

	//Initialize block
	memset(&blockInfo, 0, sizeof(BlockInfo));
	blockInfo.bitcfg = 0x24;
	blockInfo.bytecfg = 0x54;
	DWORD posOpera = checkPilot(pos);

	if (bauds && posOpera) {
		WORD roundedBauds = bauds;
		if (ABS(bauds, 1200) < 15) roundedBauds = 1200;
		if (ABS(bauds, 2400) < 15) roundedBauds = 2400;
		cout << WAVTIME(pos) << TXT_B_GREEN << "Detected #4B Opera Block (" << std::dec << bauds << " bauds)" << TXT_RESET << endl;
		if (bauds == roundedBauds) {
			cout << WAVTIME(pos) << MSG_WARNING << ": No standard baudrate!" << endl;
		}
		blockInfo.pilot = 0;
		blockInfo.pulses = 0;
		blockInfo.bit0len = MSX_PULSE(bauds)*2;
		blockInfo.bit1len = MSX_PULSE(bauds);

		//Get Data bytes
		pos=posOpera;
		DWORD posIni = pos;
		WORD value = 0;
		vector<BYTE> buff;
		while (!eof()) {
			value = getByte();
			if (value>=RET_END) break;
				if (verboseMode) cout << WAVTIME(pos) << std::hex << "Pos:[0x" << buff.size() << "]  Detected BYTE #" << std::hex << value << " (" << std::dec << value << ")" << endl;
			buff.push_back(value);
		}
		cout << WAVTIME(pos) << "Extracted data: " << (buff.size()) << " bytes" << endl;

		//Pulses after data block?
		posIni = pos;
		WORD pulsesAfterData = skipToNextSilence();
		if (pulsesAfterData > 0) {
			cout << WAVTIME(posIni) << MSG_WARNING << ": Skipping " << (pos-posIni) << " pulses after data block for "<< std::dec << (pulsesAfterData/(float)WAVSampleRate) << "sec" << endl;
		}

		//Pause after data in ms
		posIni = pos;
		blockInfo.pausems = (WORD)(skipSilence() * 1000.0f / WAVSampleRate);
		if (blockInfo.pausems > 0) {
			cout << WAVTIME(posIni) << "Skip silence ("<< std::dec << blockInfo.pausems/1000.0f << "sec)" << endl;
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
			cout << WAVTIME(pos) << TXT_B_GREEN << "Adding #4B Opera Block" << TXT_RESET << endl;
			return true;
		}
	}

	//If no OPERA block detected then return false
	return false;
}

DWORD Opera4B_Ripper::checkPilot(DWORD posIni)
{
	float pulseSum = states[posIni++];
	float bytes = 1.f;
	float bytesLen = 0.f;
	bauds = 1200;

	while (!eof(posIni)) {
		if (MSX4B_Ripper::checkBitN(posIni,0)) {
			int blockPos=posIni;
			for (int bucle=0 ;bucle <42 ;bucle++) {
				bytesLen = pulseSum / bytes;
				bauds = (DWORD)((float)WAVSampleRate / 4.f /bytesLen);
				bytes++;
				pulseSum += states[posIni++];
			}
			return blockPos;
		} else {
			if (MSX4B_Ripper::checkBitN(posIni,1)) {
				posIni++;
			} else {
				break;
			}
		}		
	}
	return false;
}

WORD Opera4B_Ripper::getByte()
{
	WORD value = MSX4B_Ripper::getByte();

	if (ABS(states[pos-1], ZERO_PULSE) <= ZERO_PULSE*WINDOW &&
		ABS(states[pos], ZERO_PULSE) <= ZERO_PULSE*WINDOW) {
		pos--;
		cout << WAVTIME(pos) << MSG_WARNING << ": Adjusting 1 pulse backward!" << endl;
	} else
	if (ABS(states[pos], ONE_PULSE) <= ONE_PULSE*WINDOW &&
		ABS(states[pos+1], ZERO_PULSE) <= ZERO_PULSE*WINDOW) {
		pos++;
		cout << WAVTIME(pos) << MSG_WARNING << ": Adjusting 1 pulse forward!" << endl;
	}

	return value;
}

