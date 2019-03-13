
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
		if (b->getId()==0x4b && ((Block4B*)b)->getFileType()==0xff && old!=NULL && old->getId()==0x4b && ((Block4B*)old)->getFileType()==0xd0) {
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
			case 0x10: b = new Block10(ifs); break;
			case 0x11: b = new Block11(ifs); break;
			case 0x12: b = new Block12(ifs); break;
			case 0x13: b = new Block13(ifs); break;
			case 0x14: b = new Block14(ifs); break;
			case 0x15: b = new Block15(ifs); break;
			case 0x20: b = new Block20(ifs); break;
			case 0x21: b = new Block21(ifs); break;
			case 0x22: b = new Block22(ifs); break;
			case 0x23: b = new Block23(ifs); break;
			case 0x24: b = new Block24(ifs); break;
			case 0x25: b = new Block25(ifs); break;
			case 0x26: b = new Block26(ifs); break;
			case 0x27: b = new Block27(ifs); break;
			case 0x28: b = new Block28(ifs); break;
			case 0x2A: b = new Block2A(ifs); break;
			case 0x30: b = new Block30(ifs); break;
			case 0x31: b = new Block31(ifs); break;
			case 0x32: b = new Block32(ifs); break;
			case 0x33: b = new Block33(ifs); break;
			case 0x35: b = new Block35(ifs); break;
			case 0x4B: b = new Block4B(ifs); break;
			case 0x5A: b = new Block5A(ifs); break;
			//NOT FULLY IMPLEMENTED
			case 0x18: cout << "Block #18 NOT YET IMPLEMENTED" << endl; break;
			case 0x19: b = new Block19(ifs); break;
			case 0x2B: cout << "Block #2B NOT YET IMPLEMENTED" << endl; break;
			//DEPRECATED
			case 0x16: b = new Block16(ifs); break;
			case 0x17: b = new Block17(ifs); break;
			case 0x34: b = new Block34(ifs); break;
			case 0x40: b = new Block40(ifs); break;
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

