#ifndef __MSX4B_RIPPER_H__
#define __MSX4B_RIPPER_H__

#include "types.h"
#include "BlockRipper.h"


using namespace WAV_Class;


namespace Rippers {

	class MSX4B_Ripper : public BlockRipper
	{
	public:
		MSX4B_Ripper(WAV *wav);
		MSX4B_Ripper(const MSX4B_Ripper& other);
		~MSX4B_Ripper();
		bool detectBlock();
		
	protected:
		DWORD skipSilence();
		DWORD skipToNextSilence();
		bool checkPulseWidth(DWORD pulseWidth, WORD pulses, DWORD bauds);
		/*
		 * 1200 Baudios CORTA ... 3840 ciclos (7680 pulsos) ~ 1.5 segundos
		 * 1200 Baudios LARGA ... 15360 ciclos (30720 pulsos) ~ 6.1 segundos
		 * 2400 Baudios CORTA ... 7936 ciclos (15872 pulsos) ~ 1.6 segundos
		 * 2400 Baudios LARGA ... 31744 ciclos (63488 pulsos) ~ 6.3 segundos
		 */
		bool checkHeader(DWORD posIni);
		DWORD checkBit0(DWORD posIni);
		DWORD checkBit1(DWORD posIni);
		WORD getByte();

		const static DWORD THRESHOLD_HEADER = 25;

		struct BlockInfo {
			WORD pausems;		//Pause after this block in milliseconds
			WORD pilot;			//Duration of a PILOT pulse in T-states {same as ONE pulse}
			WORD pulses;		//Number of pulses in the PILOT tone
			WORD bit0len;		//Duration of a ZERO pulse in T-states {=2*pilot}
			WORD bit1len;		//Duration of a ONE pulse in T-states {=pilot}
			const BYTE bitcfg = 0x24;
			const BYTE bytecfg = 0x54;
		} blockInfo;
		DWORD bauds = 0;
	};

}

#endif//__MSX4B_RIPPER_H__
