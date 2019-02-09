#ifndef __B12_PURETONE_RIPPER_H__
#define __B12_PURETONE_RIPPER_H__

#include "types.h"
#include "BlockRipper.h"


using namespace WAV_Class;


namespace Rippers {

	/**
	 * @class B12_PureTone_Ripper
	 * @author NataliaPC
	 * @date 27/07/17
	 * @file B12_PureTone_Ripper.h
	 * 
	 * Class to detect pulse tones alone
	 * 
	 */
	class B12_PureTone_Ripper : public BlockRipper
	{
	public:
		B12_PureTone_Ripper(WAV *wav);
		B12_PureTone_Ripper(const B12_PureTone_Ripper& other);
		~B12_PureTone_Ripper();
		bool detectBlock();
	protected:
		bool detectSilence(DWORD posIni) override;
		WORD findTone(DWORD posIni);

	protected:
		const static DWORD THRESHOLD_PILOT = 5;
	};

}

#endif//__B12_PURETONE_RIPPER_H__
