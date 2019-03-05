#ifndef __B13_PULSESEQUENCE_RIPPER_H__
#define __B13_PulseSequence_Ripper_H__

#include "types.h"
#include "BlockRipper.h"


using namespace WAV_Class;


namespace Rippers {

	/**
	 * @class B13_PulseSequence_Ripper
	 * @author NataliaPC
	 * @date 27/07/17
	 * @file B13_PulseSequence_Ripper.h
	 * 
	 * Class to detect pulse sequences
	 * 
	 */
	class B13_PulseSequence_Ripper : public BlockRipper
	{
	public:
		B13_PulseSequence_Ripper(WAV *wav);
		B13_PulseSequence_Ripper(const B13_PulseSequence_Ripper& other);
		~B13_PulseSequence_Ripper();
		bool detectBlock();
	protected:
		bool detectSilence(DWORD posIni) override;

	protected:
		const static DWORD THRESHOLD_PILOT = 5;
	};

}

#endif//__B13_PULSESEQUENCE_RIPPER_H__
