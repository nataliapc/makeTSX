
#include <cmath>
#include <vector>
#include <cstring>

#include "B13_PulseSequence_Ripper.h"


using namespace std;
using namespace WAV_Class;
using namespace Rippers;


#define SILENCE_MILIS			40


B13_PulseSequence_Ripper::B13_PulseSequence_Ripper(WAV *wav) : BlockRipper(wav)
{
}

B13_PulseSequence_Ripper::B13_PulseSequence_Ripper(const B13_PulseSequence_Ripper& other) : BlockRipper(other)
{
}

B13_PulseSequence_Ripper::~B13_PulseSequence_Ripper()
{
}

bool B13_PulseSequence_Ripper::detectSilence(DWORD pos)
{
	return !eof(pos) && (bytes2milis(states[pos])>SILENCE_MILIS || bytes2tstates(states[pos])>65535);
}

bool B13_PulseSequence_Ripper::detectBlock()
{
	block = NULL;
	DWORD posIni = pos;
	vector<WORD> pulses;

	//Check for pulse sequence
	while (!detectSilence(posIni) && pulses.size()<255) {
		pulses.push_back(bytes2tstates(states[posIni++]));
	}

	//If pulses then add silence block
	if (pulses.size() >= 1) {
		cout << WAVTIME(pos) << "Detected #13 Pulse Sequence Block ("<< std::dec << pulses.size() << " pulses)" << endl;
		cout << WAVTIME(posIni) << "Adding #13 Pulse Sequence Block" << endl;
		pos = posIni;
		block = new Block13(pulses.size(), pulses.data());
		return true;
	}

	return false;
}
