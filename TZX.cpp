
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cstring>

#include "TZX.h"


using namespace TZX_Class;
using namespace Utility;


// ============================================================================================
// Constructors / Destructor

TZX::TZX()
{
	blocks = new vector<Block*>();
	majorVer = TZX_VER_HI;
	minorVer = TZX_VER_LO;
}

TZX::TZX(string filename)
{
	blocks = new vector<Block*>();
	majorVer = TZX_VER_HI;
	minorVer = TZX_VER_LO;
	loadFromFile(filename);
}

TZX::TZX(const TZX& other)
{
	majorVer = other.majorVer;
	minorVer = other.minorVer;
	blocks = other.blocks;
}

TZX::~TZX()
{
	blocks->clear();
	delete blocks;
}

// ============================================================================================
// Getters / Setters

size_t TZX::getNumBlocks()
{
	return blocks->size();
}

Block* TZX::getBlock(size_t index)
{
	Block *b = blocks->at(index);
	return b;
}

Block* TZX::getLastBlock()
{
	if (!getNumBlocks()) return NULL;
	Block *b = blocks->at(blocks->size()-1);
	return b;
}

void TZX::addBlock(Block *block)
{
	blocks->push_back(block);
}

// ============================================================================================
// Private methods

// ============================================================================================
// Public inteface

void TZX::clear()
{
	blocks->clear();
	majorVer = TZX_VER_HI;
	minorVer = TZX_VER_LO;
}

void TZX::showInfo()
{
	genericShowInfo(0);
}

void TZX::hexDump()
{
	genericShowInfo(1);
}

void TZX::hexCharDump()
{
	genericShowInfo(2);
}

void TZX::genericShowInfo(int typeDump)
{
	Block *b = NULL, *old;
	for (size_t i=0; i<blocks->size(); i++) {
		old = b;
		b = blocks->at(i);
		cout << TXT_B_WHITE << "[" << std::dec << i << "] " << TXT_RESET;
		if (b->getId()==B4B_MSX_KCS && ((Block4B*)b)->getFileType()==0xff && old!=NULL && 
			old->getId()==B4B_MSX_KCS && ((Block4B*)old)->getFileType()==0xd0) {
			cout << ((Block4B*)b)->toString(true) << endl;
		} else {
			cout << b->toString() << endl;
		}
		if (typeDump==1) b->hexDump();
		if (typeDump==2) b->hexCharDump();
		if (typeDump!=0) cout << "----------------" << endl << endl;
	}
}

bool TZX::loadFromFile(string filename)
{
	std::ifstream ifs (filename, std::ifstream::in | std::ios::binary);
	if (!ifs.is_open()) return false;
	clear();

	//Read Header
	char magic[8];
	ifs.read(magic, 8);
	if (strncmp((const char *)magic, TZX_MAGIC, 8)!=0) {
		cout << "Bad TZX header in file " << filename << endl;
		ifs.close();
		return false;
	}
	majorVer = ifs.get();
	minorVer = ifs.get();

	//Read Blocks
	Block *b;
	BYTE bid;
	while (true) {
		bid = ifs.get();
		if (ifs.eof()) break;
		b = NULL;
		switch (bid) {
			case B10_STD_BLOCK:		b = new Block10(ifs); break;
			case B11_TURBO_BLOCK:	b = new Block11(ifs); break;
			case B12_PURE_TONE:		b = new Block12(ifs); break;
			case B13_PULSE_SEQ:		b = new Block13(ifs); break;
			case B14_PURE_DATA:		b = new Block14(ifs); break;
			case B15_DIRECT_REC:	b = new Block15(ifs); break;
			case B20_SILENCE_BLOCK:	b = new Block20(ifs); break;
			case B21_GRP_START:		b = new Block21(ifs); break;
			case B22_GRP_END:		b = new Block22(ifs); break;
			case B23_JUMP_BLOCK:	b = new Block23(ifs); break;
			case B24_LOOP_START:	b = new Block24(ifs); break;
			case B25_LOOP_END:		b = new Block25(ifs); break;
			case B26_CALL_SEQ:		b = new Block26(ifs); break;
			case B27_RET_SEQ:		b = new Block27(ifs); break;
			case B28_SEL_BLOCK:		b = new Block28(ifs); break;
			case B2A_STOP48K:		b = new Block2A(ifs); break;
			case B2B_SIGNAL_LEVEL:	b = new Block2B(ifs); break;
			case B30_TEXT_DESCRIP:	b = new Block30(ifs); break;
			case B31_MSG_BLOCK:		b = new Block31(ifs); break;
			case B32_ARCHIVE_INFO:	b = new Block32(ifs); break;
			case B33_HARDWARE_TYPE:	b = new Block33(ifs); break;
			case B35_CUSTOM_INFO:	b = new Block35(ifs); break;
			case B4B_MSX_KCS:		b = new Block4B(ifs); break;
			case B5A_GLUE_BLOCK:	b = new Block5A(ifs); break;
			//NOT FULLY IMPLEMENTED
			case B18_CSW_REC:		b = new Block18(ifs); break;
			case B19_GEN_DATA:		b = new Block19(ifs); break;
			//DEPRECATED
			case B16_C64ROM:		b = new Block16(ifs); break;
			case B17_C64TURBO:		b = new Block17(ifs); break;
			case B34_EMUINFO:		b = new Block34(ifs); break;
			case B40_SNAPSHOT:		b = new Block40(ifs); break;
			default:   cout << "Block " << std::hex << bid << " UNKNOWN!!" << endl; break;
		}
		if (b!=NULL) {
			blocks->push_back(b);
		}
	}

	ifs.close();
	return true;
}

bool TZX::saveToFile(string filename)
{
	std::ofstream ofs (filename, std::ifstream::out | std::ios::binary);
	if (!ofs.is_open()) return false;

	//Write header
	ofs.write(TZX_MAGIC, 8);
	ofs.write((const char*)new BYTE[2]{ majorVer, minorVer }, 2);

	//Write blocks
	Block *b;
	for (size_t i=0; i<blocks->size(); i++) {
		b = blocks->at(i);
		ofs.write(b->getBytes(), b->getSize());
	}

	ofs.close();
	return true;
}

