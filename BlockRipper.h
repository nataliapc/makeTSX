#ifndef __BLOCK_RIPPER_H__
#define __BLOCK_RIPPER_H__

#include "types.h"
#include "stdint.h"

#include "TZX_Blocks.h"
#include "WAV.h"


using namespace TZX_Blocks;


namespace WAV_Class {

	// ============================================================================================
	// Class BlockRipper

	#define Z80HZ			((uint32_t)3500000)		// ZX Standard 3.5 Mhz
	#define WAVSampleRate	(header->nSamplesPerSec)
	#define WAVTIME(pos)	"[" << ((float)pos/WAVSampleRate) << "s] "


	class BlockRipper
	{
	public:
		const BYTE	STATE_LOW  = 1;
		const BYTE	STATE_MID  = 2;
		const BYTE	STATE_HIGH = 3;

		BlockRipper(WAV *);
		BlockRipper(const BlockRipper& other);
		~BlockRipper();

		virtual	bool detectBlock() = 0;
		DWORD	getPulseWidth(DWORD posini);
		bool 	detectSilence();
		DWORD	skipSilence();
		DWORD	skipToNextSilence();
		Block*	getDetectedBlock();
		DWORD	getPos();
		void	incPos();
		bool	eof();

	protected:
		BYTE getState(int i);
		bool isLow(int i);
		bool isHigh(int i);
		bool isSilence(int i);

		const static DWORD THRESHOLD_SILENCE = 150;

		int8_t threshold = 5;

		WAV::Header *header;	//Pointer to the WAV header
		int8_t *data;			//Pointer to the WAV data
		size_t size;			//Size of the WAV data
		static size_t pos;		//Index with the current data byte in *data

		Block *block;			//Block detected
	};

}

#endif //__BLOCK_RIPPER_H__