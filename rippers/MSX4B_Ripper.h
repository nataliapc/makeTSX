#ifndef __MSX4B_RIPPER_H__
#define __MSX4B_RIPPER_H__

#include "types.h"
#include "BlockRipper.h"


using namespace WAV_Class;


namespace Rippers {

	#define WINDOW1ST				0.25f
	#define WINDOW					0.16f

	#define MSX_PULSE(bauds)		((float)Z80HZ/(bauds*4))						// T-states for a pulse at 'bauds' (1200:~729.16667f 2400:364,58333)
	#define bytesPerPulse(bauds)	((float)MSX_PULSE(bauds)*WAVSampleRate/Z80HZ)	// Bytes per pulse for bauds parameter on a WAV frequency
	#define bytesPerBit(bauds)		(bytesPerPulse(bauds)*4)						// Wav Bytes for every Data bit for bauds parameter

	#define ONE_PULSE				(bytesPerPulse(bauds))
	#define ZERO_PULSE				(bytesPerPulse(bauds)*2)

	#define NEXTPULSES2(pos)		" [" << states[pos] << " " << states[pos+1] << "]"
	#define NEXTPULSES(pos)			" [" << states[pos] << " " << states[pos+1] << " " << states[pos+2] << " " << states[pos+3] << "]"

	/**
	 * @class MSX4B_Ripper
	 * @author NataliaPC
	 * @date 10/03/17
	 * @file MSX4B_Ripper.h
	 * 
	 * Class to detect Kansas City Standard Blocks used by MSX Computers
	 * 
	 * Header to define bauds of the data block:
	 *  Short 1200 bauds ... 3840 cycles  (7680 pulses)  ~ 1.5 sec
	 *  Large 1200 bauds ... 15360 cycles (30720 pulses) ~ 6.1 sec
	 *  Short 2400 bauds ... 7936 cycles  (15872 pulses) ~ 1.6 sec
	 *  Large 2400 bauds ... 31744 cycles (63488 pulses) ~ 6.3 sec
	 * 
	 * Bits encoded at 1200 bauds:
	 *  0 ________""""""""  1 cycles at 1200Hz
	 *  1 ____""""____""""  2 cycles at 2400Hz
	 * 
	 * Bits encoded at 2400 bauds:
	 *  0 ____""""  1 cycles at 2400Hz
	 *  1 __""__""  2 cycles at 4800Hz
	 * 
	 * Each Byte is encoded with 11 bits with 1 starter bit(0) and 2 stop bits(1):
	 *  0 b0 b1 b2 b3 b4 b5 b6 b7 1 1
	 */
	class MSX4B_Ripper : public BlockRipper
	{
	public:
		MSX4B_Ripper(WAV *wav);
		MSX4B_Ripper(const MSX4B_Ripper& other);
		~MSX4B_Ripper();
		bool detectBlock() override;
	protected:
		bool  detectSilence(DWORD posIni) override;
		DWORD predictiveBitsForward(DWORD posIni, int8_t currentBit, bool bitChoice, bool useStartBit);
		DWORD checkBit0(DWORD posIni);
		DWORD checkBit1(DWORD posIni);
		virtual DWORD checkPilot(DWORD posIni);
		virtual WORD  getByte();
	private:
		BYTE askUserForBitValue(DWORD posIni);

	protected:
		const static DWORD THRESHOLD_HEADER = 500;

		struct BlockInfo {
			WORD pausems;			//Pause after this block in milliseconds
			WORD pilot;				//Duration of a PILOT pulse in T-states {same as ONE pulse}
			WORD pulses;			//Number of pulses in the PILOT tone
			WORD bit0len;			//Duration of a ZERO pulse in T-states {=2*pilot}
			WORD bit1len;			//Duration of a ONE pulse in T-states {=pilot}
			BYTE bitcfg = 0x24;		//Default MSX bitcfg values
			BYTE bytecfg = 0x54;	//Default MSX bytecfg values
		} blockInfo;
		DWORD bauds = 0;
	};

}

#endif//__MSX4B_RIPPER_H__
