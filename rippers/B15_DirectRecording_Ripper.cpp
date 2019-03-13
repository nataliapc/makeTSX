
#include <cmath>
#include <vector>
#include <cstring>

#include "B15_DirectRecording_Ripper.h"


using namespace std;
using namespace WAV_Class;
using namespace Rippers;


#define SILENCE_MILIS			5


B15_DirectRecording_Ripper::B15_DirectRecording_Ripper(WAV *wav) : BlockRipper(wav)
{
}

B15_DirectRecording_Ripper::B15_DirectRecording_Ripper(const B15_DirectRecording_Ripper& other) : BlockRipper(other)
{
}

B15_DirectRecording_Ripper::~B15_DirectRecording_Ripper()
{
}

bool B15_DirectRecording_Ripper::detectSilence(DWORD pos)
{
	return !eof(pos) && (bytes2milis(states[pos])>=SILENCE_MILIS);
}

bool B15_DirectRecording_Ripper::detectBlock()
{
	block = NULL;
	vector<BYTE> outSamples;
	DWORD posIni = pos;
	DWORD currentSample;
	BYTE currentState = 0;
	BYTE currentByte = 0;
	BYTE rbits = 0;

	//Check for pulse sequence
	currentState = isHigh(samples[posIni]);
	while (!eof(posIni) && !detectSilence(posIni) && outSamples.size()<0xFFFFFF) {
		currentSample = (DWORD)states[posIni++];
		if (verboseMode) cout << WAVTIME(posIni) << "  " << currentSample << " -> ";
		while (currentSample--) {
			if (currentState) {
				currentByte |= 1 << (7-rbits);
				if (verboseMode) cout << '*';
			}
			else if (verboseMode) cout << '-';
			if (++rbits==8) {
				outSamples.push_back(currentByte);
				rbits = 0;
				currentByte = 0;
				if (verboseMode) cout << '|';
			}
		}
		currentState = !currentState;
		if (verboseMode) cout << endl;
	}
	if (rbits) outSamples.push_back(currentByte);

	//If samples then add silence block
	if (outSamples.size() >= 1) {
		cout << WAVTIME(pos) << TXT_B_GREEN << "Detected #15 Direct Recording Block ("<< std::dec << outSamples.size() << " samples)" << TXT_RESET << endl;
		if (verboseMode) cout << WAVTIME(pos) << TXT_B_WHITE << "T-states/Sample: " << std::dec << statesPerSample << TXT_RESET << endl <<
		                         WAVTIME(pos) << TXT_B_WHITE << "Samples:         " << outSamples.size() << TXT_RESET << endl;

		cout << WAVTIME(posIni) << TXT_B_GREEN << "Adding #15 Direct Recording Block" << TXT_RESET << endl;
		pos = posIni;
		block = new Block15(statesPerSample, 0, (rbits==0 ? 8 : rbits), (char*)outSamples.data(), outSamples.size());
		return true;
	}

	return false;
}
