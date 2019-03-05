#ifndef __OPERA4B_RIPPER_H__
#define __OPERA4B_RIPPER_H__

#include "types.h"
#include "BlockRipper.h"
#include "MSX4B_Ripper.h"


using namespace WAV_Class;


namespace Rippers {

	/**
	 * @class Opera4B_Ripper
	 * @author NataliaPC
	 * @date 02/09/17
	 * @file Opera4B_Ripper.h
	 * 
	 * Class to detect Kansas City Standard Blocks used by Opera Soft protection 
	 * loader (e.g. 'The Last Mission').
	 * 
	 * No header in this blocks. Instead 256 MSX modulated bytes are added at
	 * start of data: 255 bytes '0xFF' and 1 byte '0x00' to define the start of
	 * real data.
	 * 
	 * Bits encoded at 1200 bauds:
	 *  0 ________""""""""  1 cycles at 1200Hz
	 *  1 ____""""____""""  2 cycles at 2400Hz
	 * 
	 * Each Byte is encoded with 11 bits with 1 starter bit(0) and 2 stop bits(1):
	 *  0 b0 b1 b2 b3 b4 b5 b6 b7 1 1
	 */
	class Opera4B_Ripper : public MSX4B_Ripper
	{
	public:
		Opera4B_Ripper(WAV *wav);
		Opera4B_Ripper(const Opera4B_Ripper& other);
		~Opera4B_Ripper();
		bool detectBlock() override;
	protected:
		DWORD checkPilot(DWORD posIni) override;
		WORD  getByte() override;
	};

}

#endif//__OPERA4B_RIPPER_H__
