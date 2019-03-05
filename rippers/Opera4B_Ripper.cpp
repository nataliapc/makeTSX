
#include <cmath>
#include <vector>
#include <cstring>

#include "Opera4B_Ripper.h"
#include "MSX4B_Ripper.h"


using namespace std;
using namespace WAV_Class;
using namespace Rippers;


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

	//Calculate bauds from pilot pulses
	DWORD pilotBytes = checkPilot(pos);

	if (bauds && pilotBytes) {
		WORD roundedBauds = bauds;
		if (ABS(bauds, 1200) < 15) roundedBauds = 1200;
		if (ABS(bauds, 2400) < 15) roundedBauds = 2400;
		cout << WAVTIME(pos) << "Detected #4B Opera Block (" << std::dec << bauds << " bauds)" << endl;
		if (bauds == roundedBauds) {
			cout << WAVTIME(pos) << WARNING ": No standard baudrate!" << endl;
		}
		blockInfo.pilot = 0;
		blockInfo.pulses = 0;
		blockInfo.bit0len = MSX_PULSE(bauds)*2;
		blockInfo.bit1len = MSX_PULSE(bauds);

		//Get Data bytes
		DWORD posIni = pos;
		WORD value = 0;
		vector<BYTE> buff;
		while (!eof()) {
			value = getByte();
			if (value==0xFFFF) break;
			if (verboseMode) cout << WAVTIME(pos) << std::hex << "Pos:[0x" << buff.size() << "]  Detected BYTE #" << std::hex << value << " (" << std::dec << value << ")" << endl;
			buff.push_back(value);
		}
		cout << WAVTIME(pos) << "Extracted data: " << (buff.size()) << " bytes" << endl;

		//Pulses after data block?
		posIni = pos;
		WORD pulsesAfterData = skipToNextSilence();
		if (pulsesAfterData > 0) {
			cout << WAVTIME(posIni) << WARNING ": Skipping " << (pos-posIni) << " pulses after data block for "<< std::dec << (pulsesAfterData/(float)WAVSampleRate) << "sec" << endl;
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
			cout << WAVTIME(pos) << "Adding #4B Opera Block" << endl;
			return true;
		}
	}

	//If no OPERA block detected then return false
	return false;
}

DWORD Opera4B_Ripper::checkPilot(DWORD posIni)
{
//	DWORD sumIni = samples[posIni];
	bauds = 1200;

	float pulseSum = states[posIni+4]*44.f;
	float bytes = 1.f;
	float bytesLen = 0.f;
//	while (!eof(posIni)) {
		bytesLen = pulseSum / bytes;
		bauds = (DWORD)((float)WAVSampleRate / bytesLen * 11.f);
//		if (states[posIni] > THRESHOLD_SILENCE) break;
//cout << posIni << endl;
		posIni = predictiveBitsForward(posIni+2, -1, 0, true);
//cout << WAVTIME(posIni) << "bauds:" << std::dec << bauds << " bytes:" << (DWORD)bytes << " bytesLen:" << (DWORD)bytesLen << endl;
//		if (bytes > 200 || !posIni) break;
//		pulseSum = samples[posIni] - sumIni;
		bytes++;
//	}
	//At least 100 pilot pulses
//	if (bytesLen==0 || bytes < 200) return 0;
	return bytes;
}

WORD Opera4B_Ripper::getByte()
{
	WORD value = MSX4B_Ripper::getByte();

	if (ABS(states[pos-1], ZERO_PULSE) <= ZERO_PULSE*WINDOW &&
		ABS(states[pos], ZERO_PULSE) <= ZERO_PULSE*WINDOW) {
		pos--;
		cout << WAVTIME(pos) << WARNING ": Adjusting 1 pulse backward!" << endl;
	} else
	if (ABS(states[pos], ONE_PULSE) <= ONE_PULSE*WINDOW &&
		ABS(states[pos+1], ZERO_PULSE) <= ZERO_PULSE*WINDOW) {
		pos++;
		cout << WAVTIME(pos) << WARNING ": Adjusting 1 pulse forward!" << endl;
	}

	return value;
}

