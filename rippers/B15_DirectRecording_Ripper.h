#ifndef __B15_DIRECTRECORDING_RIPPER_H__
#define __B15_DIRECTRECORDING_RIPPER_H__

#include "types.h"
#include "BlockRipper.h"


using namespace WAV_Class;


namespace Rippers {

	#define statesPerSample		((Z80HZ+(WAVSampleRate/2))/WAVSampleRate)	// T-states per WAV sample

	/**
	 * @class B15_DirectRecording_Ripper
	 * @author NataliaPC
	 * @date 12/03/19
	 * @file B15_DirectRecording_Ripper.h
	 * 
	 * Class to create direct recording blocks
	 * 
	 */
	class B15_DirectRecording_Ripper : public BlockRipper
	{
	public:
		B15_DirectRecording_Ripper(WAV *wav);
		B15_DirectRecording_Ripper(const B15_DirectRecording_Ripper& other);
		~B15_DirectRecording_Ripper();
		bool detectBlock() override;
	protected:
		bool detectSilence(DWORD posIni) override;
	};

}

#endif//__B15_DIRECTRECORDING_RIPPER_H__
