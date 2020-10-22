#ifndef __B11_CUSTOM_RIPPER_H__
#define __B11_CUSTOM_RIPPER_H__

#include "types.h"
#include "BlockRipper.h"


using namespace WAV_Class;


namespace Rippers {

	/**
	 * @class B11_Custom_Ripper
	 * @author NataliaPC
	 * @date 18/10/2020
	 * @file B11_Custom_Ripper.h
	 * 
	 * Class to detect Block #11
	 * 
	 * PILOT Pulse:		2168 Tstates (27.316/29.732 samples @8bits:44k1/48kHz) 8063/3223 pulses
	 * SYNC#1 Pulse:	 667 Tstates ( 8.404/ 9.147 samples @8bits:44k1/48kHz)
	 * SYNC#2 Pulse:	 735 Tstates ( 9.261/10.080 samples @8bits:44k1/48kHz)
	 * ZERO bit Pulse:	 855 Tstates (10.773/12.137 samples @8bits:44k1/48kHz)
	 * ONE bit Pulse:	1710 Tstates (21.546/23,451 samples @8bits:44k1/48kHz)
	 *
	 * Enterprise 64/128 t-states:
	 * 		Pilot:1750 Sync:2800 One:1400 Zero:2100
	 * 		Pilot:742  Sync:1217 One:602  Zero:882
	 */
	class B11_Custom_Ripper : public BlockRipper
	{
	public:
		B11_Custom_Ripper(WAV *wav, WORD pl, WORD sl1, WORD sl2, WORD zl, WORD ol, bool nopilot);
		B11_Custom_Ripper(const B11_Custom_Ripper& other);
		~B11_Custom_Ripper();
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
		float modif = 1.0f;

		struct BlockInfo {
			bool nopilot;			//No check for a pilot tone
			WORD pausems;			//Pause after this block in milliseconds
			WORD pilotPulseLength;	//Pilot pulse length
			WORD sync1PulseLength;	//Sync1 pulse length
			WORD sync2PulseLength;	//Sync2 pulse length
			WORD zeroPulseLength;	//Zero bit pulse length
			WORD onePulseLength;	//One but pulse length
		} blockInfo;
	};

}

#endif//__B11_CUSTOM_RIPPER_H__
