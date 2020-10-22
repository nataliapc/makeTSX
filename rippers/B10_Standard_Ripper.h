#ifndef __B10_STANDARD_RIPPER_H__
#define __B10_STANDARD_RIPPER_H__

#include "types.h"
#include "BlockRipper.h"


using namespace WAV_Class;


namespace Rippers {

	/**
	 * @class B10_Standard_Ripper
	 * @author NataliaPC
	 * @date 27/07/17
	 * @file B10_Standard_Ripper.h
	 * 
	 * Class to detect Block #10
	 * 
	 * PILOT Pulse:		2168 Tstates (27.316/29.732 samples @8bits:44k1/48kHz) 8063/3223 pulses
	 * SYNC#1 Pulse:	 667 Tstates ( 8.404/ 9.147 samples @8bits:44k1/48kHz)
	 * SYNC#2 Pulse:	 735 Tstates ( 9.261/10.080 samples @8bits:44k1/48kHz)
	 * ZERO bit Pulse:	 855 Tstates (10.773/12.137 samples @8bits:44k1/48kHz)
	 * ONE bit Pulse:	1710 Tstates (21.546/23,451 samples @8bits:44k1/48kHz)
	 */
	class B10_Standard_Ripper : public BlockRipper
	{
	public:
		B10_Standard_Ripper(WAV *wav);
		B10_Standard_Ripper(const B10_Standard_Ripper& other);
		~B10_Standard_Ripper();
		bool detectBlock() override;
	protected:
		bool  detectSilence(DWORD posIni) override;
		DWORD checkPilot(DWORD posIni);
		bool  checkBit0(DWORD posIni);
		bool  checkBit1(DWORD posIni);
		WORD  getByte();
	private:
		bool  askUserForSyncBits(DWORD posIni);
		BYTE  askUserForBitValue(DWORD posIni);

	protected:
		const static DWORD THRESHOLD_PILOT = 5;
		float modif;
	};

}

#endif//__B10_STANDARD_RIPPER_H__
