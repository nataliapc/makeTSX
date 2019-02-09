#ifndef __WAV_H__
#define __WAV_H__

#include "types.h"
#include <string>

#include "ByteBuffer.h"


using namespace std;
using namespace Utility;

namespace WAV_Class
{
	// ============================================================================================
	// Constants & Structs

	// ============================================================================================
	// Class WAV

	class WAV
	{
	public:
		WAV();
		WAV(string filename);
		WAV(const WAV& other);
		~WAV();

		void showInfo();
		void clear();
		bool loadFromFile(string filename);
		bool saveToFile(string filename);
		size_t getSize();

		void normalize();
		void envelopeCorrection();

	protected:
		const char* MAGIC_RIFFID = "RIFF";
		const char* MAGIC_WAVEID = "WAVE";
		const char* MAGIC_FMTID  = "fmt ";
		const char* MAGIC_DATAID = "data";

		struct Header {
			BYTE	riffId[4];			// "RIFF"
			DWORD	riffSize;			// Size of chunk
			BYTE	waveId[4];			// "WAVE"
			BYTE	fmtId[4];			// "fmt "
			DWORD	fmtSize;			// 16 for PCM
			WORD	wFormatTag;			// PCM:1
			WORD	nChannels;			// Mono:1 Stereo:2
			DWORD	nSamplesPerSec;		// SampleRate: 8000, 44100, ...
			DWORD	nAvgBytesPerSec;	// = SampleRate * NumChannels * BitsPerSample/8
			WORD	nBlockAlign;		// = NumChannels * BitsPerSample/8
			WORD	wBitsPerSample;		// 8:8bits 16:16bits
			BYTE	dataId[4];			// "data"
			DWORD	dataSize;			// Size of data chunk
		};

		Header *header;
		int8_t *data;
		size_t size;

		bool phase = false;

		friend class BlockRipper;
	};
}

#endif //__WAV_H__
