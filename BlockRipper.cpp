
#include "BlockRipper.h"
#include "TZX_Blocks.h"

#include <cmath>


using namespace std;
using namespace TZX_Blocks;
using namespace WAV_Class;


size_t BlockRipper::pos = 0;


BlockRipper::BlockRipper(WAV *wav)
{
	bool newWav = (this->data != wav->data);

	this->header = wav->header;
	this->data = wav->data;
	this->size = wav->size;
	block = NULL;
	if (newWav) {
		pos = 0;
		initializeStatesVector();
	}
}

BlockRipper::BlockRipper(const BlockRipper& other)
{
	this->header = other.header;
	this->data = other.data;
	this->size = other.size;
	block = new Block(*(other.block));
}

BlockRipper::~BlockRipper()
{
	this->header = NULL;
	this->data = NULL;
}

void BlockRipper::initializeStatesVector()
{
	DWORD lastStatePos = 0;
	BYTE currState = getState(0);
	BYTE newState;
	DWORD samplesCnt = 0;

	states.clear();
	samples.clear();
	for (DWORD i=0; i<size; i++) {
		newState = getState(i);
		if (newState != STATE_NOCHANGE) {
			if (newState != currState && i-lastStatePos>1) {
//cout << WAVTIME(i) << i << " cambio " << (int)(i-lastStatePos) << endl;
				samples.push_back(samplesCnt);
				states.push_back(i-lastStatePos);
				samplesCnt += i-lastStatePos;
				lastStatePos = i;
				currState = newState;
			}
		}
//cout << WAVTIME(i) << i << " \t " << std::dec << (unsigned int)(data[i]&0xff) << " \t " << (int)newState << " old:" << (int)currState << endl;
	}
}

bool BlockRipper::detectSilence()
{
	return !eof() && states[pos] >= THRESHOLD_SILENCE;
}

bool BlockRipper::detectSilence(DWORD position)
{
	return !eof() && states[position] >= THRESHOLD_SILENCE;
}

DWORD BlockRipper::skipSilence()
{
	DWORD silenceCount = 0;

	while (!eof() && detectSilence()) {
		silenceCount += states[pos];
		pos++;
	}
	return silenceCount;
}

DWORD BlockRipper::skipToNextSilence()
{
	DWORD skipCount = 0;

	while (!eof() && !detectSilence()) {
		skipCount += states[pos];
		pos++;
	}
	return skipCount;
}

Block* BlockRipper::getDetectedBlock()
{
	return block;
}

DWORD BlockRipper::getPos()
{
	return pos;
}

void BlockRipper::incPos()
{
	if (!eof()) pos++;
}

bool BlockRipper::eof()
{
	return pos >= states.size();
}

BYTE BlockRipper::getState(int i)
{
	if (isLow(i)) return STATE_LOW;
	if (isHigh(i)) return STATE_HIGH;
	return STATE_NOCHANGE;
}

bool BlockRipper::isLow(int i)
{
	return data[i] <= -threshold;
}

bool BlockRipper::isHigh(int i)
{
	return data[i] >= threshold;
}
