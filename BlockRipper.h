#ifndef __BLOCK_RIPPER_H__
#define __BLOCK_RIPPER_H__

#include "types.h"
#include "stdint.h"
#include <vector>

#include "TZX_Blocks.h"
#include "WAV.h"


using namespace TZX_Blocks;


namespace WAV_Class {

	// ============================================================================================
	// Class BlockRipper

	#define Z80HZ			((uint32_t)3500000)		// ZX Standard 3.5 Mhz
	#define WAVSampleRate	(header->nSamplesPerSec)
	#define WAVTIME(pos)	"[" << ((float)samples[pos]/WAVSampleRate) << "s] "


	class BlockRipper
	{
	public:
		const BYTE	STATE_LOW      = 0;
		const BYTE	STATE_HIGH     = 1;
		const BYTE	STATE_NOCHANGE = 127;

		BlockRipper(WAV *);
		BlockRipper(const BlockRipper& other);
		~BlockRipper();

		virtual	bool detectBlock() = 0;
		bool 	detectSilence();
		bool 	detectSilence(DWORD);
		DWORD	skipSilence();
		DWORD	skipToNextSilence();
		Block*	getDetectedBlock();
		DWORD	getPos();
		void	incPos();
		bool	eof();

	protected:
		const static DWORD THRESHOLD_SILENCE = 100;
		int8_t threshold = 30;
		Block *block;					//Block detected
		WAV::Header *header;			//Pointer to the WAV header
		vector<DWORD> states;			//Vector with state change positions
		vector<DWORD> samples;			//Vector with samples from beginning
		static size_t pos;				//Index with the current data byte in vector states

	private:
		int8_t *data;					//Pointer to the WAV data
		size_t size;					//Size of the WAV data

		void initializeStatesVector();
		bool isLow(int i);
		bool isHigh(int i);
		BYTE getState(int i);
	};

}

#endif //__BLOCK_RIPPER_H__