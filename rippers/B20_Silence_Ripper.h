#ifndef __B20_SILENCE_RIPPER_H__
#define __B20_SILENCE_RIPPER_H__

#include "types.h"
#include "BlockRipper.h"


using namespace WAV_Class;


namespace Rippers {

	/**
	 * @class B20Silence_Ripper
	 * @author NataliaPC
	 * @date 27/07/17
	 * @file Silence20_Ripper.h
	 * 
	 * Class to detect silences
	 * 
	 */
	class B20_Silence_Ripper : public BlockRipper
	{
	public:
		B20_Silence_Ripper(WAV *wav);
		B20_Silence_Ripper(const B20_Silence_Ripper& other);
		~B20_Silence_Ripper();
		bool detectBlock() override;
	protected:
		bool detectSilence(DWORD posIni) override;
		WORD findGlitches(DWORD posIni);

	protected:
		const static DWORD THRESHOLD_PILOT = 5;
	};

}

#endif//__B20_SILENCE_RIPPER_H__
