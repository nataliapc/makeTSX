
#include <cmath>
#include <vector>
#include <cstring>

#include "B20_Silence_Ripper.h"


using namespace std;
using namespace WAV_Class;
using namespace Rippers;


#define SILENCE_PULSE			(WAVSampleRate/1000)


B20_Silence_Ripper::B20_Silence_Ripper(WAV *wav) : BlockRipper(wav)
{
}

B20_Silence_Ripper::B20_Silence_Ripper(const B20_Silence_Ripper& other) : BlockRipper(other)
{
}

B20_Silence_Ripper::~B20_Silence_Ripper()
{
}

bool B20_Silence_Ripper::detectSilence(DWORD pos)
{
	return (!eof(pos) && states[pos] >= SILENCE_PULSE);
}

WORD B20_Silence_Ripper::findGlitches(DWORD posIni)
{
	WORD len = 2000.f * WAVSampleRate / Z80HZ;
	if (states[posIni]<len && states[posIni+1]>=len) return 2;
	if (states[posIni]<len && states[posIni+1]<len && states[posIni+2]>=len) return 3;
	if (states[posIni]<len && states[posIni+1]<len && states[posIni+2]<len && states[posIni+3]>=len) return 4;
/*	float pulseSum = states[posIni++];
	float pulses = 1;
	float pulseLen;
	while (posIni < states.size() && posIni < states.size()) {
		pulseLen = pulseSum / pulses;
		if (ABS(states[posIni], pulseLen) > pulseLen*0.25f) break;
		pulseSum += states[posIni++];
		pulses++;
	}
	//At least 5 pilot pulses
	if (pulses <= THRESHOLD_PILOT) return pulses;
*/	return 0;
}

bool B20_Silence_Ripper::detectBlock()
{
	block = NULL;
	DWORD silence = 0;
	DWORD posIni = pos, posAux = pos;

	//Check for silences and glitches
	while (!eof(posIni) && (detectSilence(posIni) || findGlitches(posIni))) {
		silence += states[posIni];
		posIni++;
	}

	pos = posIni;

	//If > 0 ms then add silence block
	if (silence*1000/WAVSampleRate > 0) {
		cout << WAVTIME(posAux) << "Detected #20 Silence Block ("<< std::dec << (silence/(float)WAVSampleRate) << "sec)" << endl;
		block = new Block20(silence*1000/WAVSampleRate);
		return true;
	}

	return false;
}
