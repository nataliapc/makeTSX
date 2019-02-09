
#include "BlockRipper.h"
#include "TZX_Blocks.h"

#include <cmath>


using namespace std;
using namespace TZX_Blocks;
using namespace WAV_Class;


size_t BlockRipper::pos = 0;


BlockRipper::BlockRipper(WAV *wav)
{
	this->header = wav->header;
	this->data = wav->data;
	this->size = wav->size;
	pos = 0;
	block = NULL;
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

bool BlockRipper::detectSilence()
{
	DWORD pos2 = pos;
	DWORD silence = 0;
	BYTE state = getState(pos2);

	while (pos2 < size && silence <= THRESHOLD_SILENCE) {
		if (getState(pos2)!=state) {
			return false;
		}
		pos2++;
		silence++;
	}
	return true;
}

DWORD BlockRipper::skipSilence()
{
	DWORD posAux = pos;
	DWORD pos2 = pos;

	//Skip until state change
	BYTE state = getState(pos2);
	while (pos2 < size && getState(pos2)==state) {
		pos2++;
	}
	pos = pos2;
	return pos - posAux;
}

DWORD BlockRipper::skipToNextSilence()
{
	uint32_t skip = 0;
	while (pos < size) {
		if (detectSilence()) return skip;
		skip++;
		pos++;
	}
	return skip;
}

DWORD BlockRipper::getPulseWidth(DWORD posIni)
{
	DWORD width = 0, pos2 = posIni;
	bool firstState = isHigh(pos2);
	
	for (; pos2 < size ; width++, pos2++) {
		if (isSilence(pos2) || isHigh(pos2)!=firstState) break;
	}
#ifdef _DEBUG_
	cout << WAVTIME(posIni) << "[" << std::dec << posIni << "] getPulseWidth() value:" << (int)(firstState?0:1) << " len:" << width << endl;
#endif //_DEBUG_
	return width;
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
	if (pos<size) pos++;
}

bool BlockRipper::eof()
{
	return pos >= size;
}

BYTE BlockRipper::getState(int i)
{
	if (isSilence(i)) return STATE_MID;
	if (isLow(i)) return STATE_LOW;
	if (isHigh(i)) return STATE_HIGH;
	return -1;
}

bool BlockRipper::isLow(int i)
{
	return data[i] < threshold;
}

bool BlockRipper::isHigh(int i)
{
	return data[i] >= threshold;
}

bool BlockRipper::isSilence(int i)
{
	return abs(data[i]) < threshold;
}
