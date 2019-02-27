#ifndef __MSX4B_RIPPER_H__
#define __MSX4B_RIPPER_H__

#include "types.h"
#include "BlockRipper.h"


using namespace WAV_Class;


namespace Rippers {

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
		WORD  calculateBaudRate(DWORD posIni);
		bool  checkPulseWidth(DWORD pulseWidth, WORD pulses, DWORD bauds, bool first);
		bool  checkHeader(DWORD posIni);
		bool  predictiveBitsForward(DWORD posIni, BYTE currentBit, bool bitChoice, bool useStartBit);
		DWORD checkBit0(DWORD posIni);
		DWORD checkBit1(DWORD posIni);
		WORD  getByte();
	private:
		BYTE askUserForBitValue(DWORD posIni);

	protected:
		const static DWORD THRESHOLD_HEADER = 1000;

		struct BlockInfo {
			WORD pausems;		//Pause after this block in milliseconds
			WORD pilot;			//Duration of a PILOT pulse in T-states {same as ONE pulse}
			WORD pulses;		//Number of pulses in the PILOT tone
			WORD bit0len;		//Duration of a ZERO pulse in T-states {=2*pilot}
			WORD bit1len;		//Duration of a ONE pulse in T-states {=pilot}
			BYTE bitcfg = 0x24;
			BYTE bytecfg = 0x54;
		} blockInfo;
		DWORD bauds = 0;
	};

}

#endif//__MSX4B_RIPPER_H__
