
#include <cmath>
#include <vector>
#include <cstring>

#include "B12_PureTone_Ripper.h"


using namespace std;
using namespace WAV_Class;
using namespace Rippers;


float pulseLen = 0.f;



B12_PureTone_Ripper::B12_PureTone_Ripper(WAV *wav) : BlockRipper(wav)
{
}

B12_PureTone_Ripper::B12_PureTone_Ripper(const B12_PureTone_Ripper& other) : BlockRipper(other)
{
}

B12_PureTone_Ripper::~B12_PureTone_Ripper()
{
}

bool B12_PureTone_Ripper::detectSilence(DWORD pos)
{
	return (!eof(pos) && states[pos] >= pulseLen*5);
}

WORD B12_PureTone_Ripper::findTone(DWORD posIni)
{
	float pulseSum = states[posIni++];
	float pulses = 1.f;
	WORD  fails = 0;
	pulseLen = 0.f;
	while (!eof(posIni)) {
		pulseLen = pulseSum / pulses;
		if (ABS(states[posIni-1]+states[posIni], pulseLen*2) > pulseLen*0.15) {
			fails++;
//cout << WAVTIME(posIni) << "Fails: " << fails << endl;
		} else {
//if (fails) cout << WAVTIME(posIni) << "Reset fails" << endl;
			fails = 0;
		}
		if (fails > 3) break;
		if (detectSilence(posIni)) break;
		pulseSum += states[posIni++];
		pulses++;
	}
	//At least 10 tone pulses
//cout << WAVTIME(posIni) << "End: [" << states[posIni] << "]" << endl;
	if (pulseLen==0 || pulses < 50) return 0;
	if (!detectSilence(posIni)) return 0;
	return pulses;
}

bool B12_PureTone_Ripper::detectBlock()
{
	block = NULL;
	DWORD posIni = pos;
	DWORD pulses = 0;

	//Check for pure tone pulses
	WORD tstates = 0;
	pulses = findTone(posIni);
	if (pulses) {
		tstates = bytes2tstates(pulseLen);
		cout << WAVTIME(posIni) << TXT_B_GREEN << "Detected #12 Pure Tone Block ("<< std::dec << pulses << " pulses / " << tstates << " T-states each)" << TXT_RESET << endl;
		posIni += pulses;
	}

	pos = posIni;

	//If pulses then add silence block
	if (pulses) {
		cout << WAVTIME(pos) << TXT_B_GREEN << "Adding #12 Pure Tone Block" << TXT_RESET << endl;
		block = new Block12(tstates, pulses);
		return true;
	}

	return false;
}
