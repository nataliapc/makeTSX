#ifndef __TZX_H__
#define __TZX_H__

#include "types.h"
#include "ByteBuffer.h"
#include "TZX_Blocks.h"
#include <vector>
#include <string>


using namespace std;
using namespace TZX_Blocks;

namespace TZX_Class
{
	// ============================================================================================
	// Constants

	#define TZX_LIB		"NPC_TZX Lib v0.2b"
	#define TZX_VER_HI	1
	#define TZX_VER_LO	21
	#define TZX_VER		"1.21"
	#define TZX_MAGIC	"ZXTape!\x1a"

	// ============================================================================================
	// Class TZX

	class TZX
	{
	public:
		TZX();
		TZX(string filename);
		TZX(const TZX& other);
		~TZX();

		void clear();
		void showInfo();
		void hexDump();
		void hexCharDump();
		bool loadFromFile(string filename);
		bool saveToFile(string filename);

		size_t getNumBlocks();
		Block* getBlock(size_t index);
		Block* getLastBlock();
		void   addBlock(Block *b);
		void   removeLastBlock();

		TZX& operator=(const TZX& tzx);
		
	protected:
		void genericShowInfo(int);

	private:
		BYTE majorVer;
		BYTE minorVer;
		vector<Block*> *blocks;
	};

}

#endif //__TZX_H__
