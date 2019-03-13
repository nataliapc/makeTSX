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

	#define Z80HZ					((uint32_t)3500000)		// ZX Standard 3.5 Mhz

	#define WAVSampleRate			(header->nSamplesPerSec)
	#define bytes2tstates(bytes)	((float)bytes*Z80HZ/WAVSampleRate)
	#define bytes2milis(bytes)		((float)bytes*1000/WAVSampleRate)

	#define WAVTIME(pos)			TXT_B_BLUE << "[" << ((float)samples[std::min((DWORD)pos, (DWORD)(samples.size()-1))]/WAVSampleRate) << "s] " << TXT_RESET
	#define ABS(v1,v2)				(std::abs(((float)v1)-((float)v2)))

	#define MSG_WARNING				TXT_B_RED << "WARNING" << TXT_RESET
	#define MSG_SUCCESS				TXT_B_YELLOW << "SUCCESS" << TXT_RESET
	#define MSG_ERROR				TXT_B_RED << "ERROR" << TXT_RESET


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
		Block*	getDetectedBlock();
		DWORD	getPos();
		void	incPos();
		DWORD	getSize();
		BYTE	getState(uint32_t);
		float	getTime(uint32_t);
		bool	eof();
		bool	eof(DWORD pos);

		static	void setVerboseMode(bool mode);
		static	void setInteractiveMode(bool mode);
		static	void setPredictiveMode(bool mode);

	protected:
		virtual bool detectSilence(DWORD posIni);
		bool 	detectSilence();
		DWORD	skipSilence();
		DWORD	skipToNextSilence();
		bool	isLow(int i);
		bool	isHigh(int i);

	protected:
		const static DWORD THRESHOLD_SILENCE = 50;
		int8_t threshold = 30;
		Block *block;					//Block detected
		WAV::Header *header;			//Pointer to the WAV header
		vector<DWORD> states;			//Vector with state change positions
		vector<DWORD> samples;			//Vector with samples from beginning

		static size_t pos;				//Index with the current data byte in vector states
		static bool	  verboseMode;		//Flag for Verbose mode
		static bool	  interactiveMode;	//Flag for Interactive mode
		static bool	  predictiveMode;	//Flag for Predictive bits Forward mode

	private:
		int8_t *data;					//Pointer to the WAV data
		size_t size;					//Size of the WAV data

		void initializeStatesVector();
	};

}

#endif //__BLOCK_RIPPER_H__
