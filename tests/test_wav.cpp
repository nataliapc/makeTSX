#include <cstdio>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "WAV.h"

using WAV_Class::WAV;

namespace
{
	typedef std::vector<uint8_t> Bytes;
	typedef std::vector<int16_t> Samples;
	int failures = 0;
	unsigned fixtureNumber = 0;

	class TestWAV : public WAV
	{
	public:
		const int8_t *samples() const
		{
			return data;
		}

		size_t sampleCount() const
		{
			return size;
		}

		void setPhase(bool enabled)
		{
			phase = enabled;
		}
	};

	void check(bool condition, const std::string &message)
	{
		if (!condition) {
			std::cerr << "FAIL: " << message << std::endl;
			failures++;
		}
	}

	void appendId(Bytes &bytes, const char *id)
	{
		bytes.insert(bytes.end(), id, id + 4);
	}

	void appendLE16(Bytes &bytes, uint16_t value)
	{
		bytes.push_back(static_cast<uint8_t>(value));
		bytes.push_back(static_cast<uint8_t>(value >> 8));
	}

	void appendLE32(Bytes &bytes, uint32_t value)
	{
		bytes.push_back(static_cast<uint8_t>(value));
		bytes.push_back(static_cast<uint8_t>(value >> 8));
		bytes.push_back(static_cast<uint8_t>(value >> 16));
		bytes.push_back(static_cast<uint8_t>(value >> 24));
	}

	void setLE32(Bytes &bytes, size_t offset, uint32_t value)
	{
		for (size_t i = 0; i < 4; i++) {
			bytes[offset + i] = static_cast<uint8_t>(value >> (i * 8));
		}
	}

	Bytes beginWave()
	{
		Bytes wave;
		appendId(wave, "RIFF");
		appendLE32(wave, 0);
		appendId(wave, "WAVE");
		return wave;
	}

	void appendChunk(Bytes &wave, const char *id, const Bytes &payload)
	{
		appendId(wave, id);
		appendLE32(wave, static_cast<uint32_t>(payload.size()));
		wave.insert(wave.end(), payload.begin(), payload.end());
		if (payload.size() & 1u) wave.push_back(0);
	}

	void finishWave(Bytes &wave)
	{
		setLE32(wave, 4, static_cast<uint32_t>(wave.size() - 8));
	}

	Bytes pcmFormat(uint16_t bits, uint16_t channels, uint16_t blockAlign,
		uint16_t formatTag = 1)
	{
		const uint32_t sampleRate = 44100;
		Bytes format;
		appendLE16(format, formatTag);
		appendLE16(format, channels);
		appendLE32(format, sampleRate);
		appendLE32(format, sampleRate * blockAlign);
		appendLE16(format, blockAlign);
		appendLE16(format, bits);
		return format;
	}

	Bytes pcm8(const Samples &samples)
	{
		Bytes bytes;
		for (size_t i = 0; i < samples.size(); i++) {
			bytes.push_back(static_cast<uint8_t>(samples[i] + 0x80));
		}
		return bytes;
	}

	bool loadBytes(TestWAV &wav, const Bytes &bytes)
	{
		const std::string path = "obj/test_wav_fixture_" +
			std::to_string(++fixtureNumber) + ".wav";
		{
			std::ofstream output(path.c_str(), std::ios::out | std::ios::binary);
			if (!output.write(reinterpret_cast<const char *>(bytes.data()), bytes.size())) {
				throw std::runtime_error("Unable to write WAV test fixture");
			}
		}
		const bool loaded = wav.loadFromFile(path);
		std::remove(path.c_str());
		return loaded;
	}

	Samples sampleValues(const TestWAV &wav)
	{
		Samples samples;
		for (size_t i = 0; i < wav.sampleCount(); i++) {
			samples.push_back(wav.samples()[i]);
		}
		return samples;
	}

	void checkSamples(const TestWAV &wav, const Samples &expected, const std::string &name)
	{
		check(wav.sampleCount() == expected.size(), name + " sample count");
		if (wav.sampleCount() != expected.size() || wav.samples() == NULL) return;
		for (size_t i = 0; i < expected.size(); i++) {
			if (wav.samples()[i] != expected[i]) {
				std::cerr << "FAIL: " << name << " sample " << i
					<< " expected " << expected[i]
					<< " got " << static_cast<int>(wav.samples()[i])
					<< std::endl;
				failures++;
				return;
			}
		}
	}

	void checkSaveRoundTrip(TestWAV &wav, const Samples &expected,
		const std::string &name)
	{
		const Samples before = sampleValues(wav);
		const std::string path = "obj/test_wav_saved_" +
			std::to_string(++fixtureNumber) + ".wav";
		const bool saved = wav.saveToFile(path);
		check(saved, name + " saves");
		checkSamples(wav, before, name + " does not modify source samples");
		if (saved) {
			TestWAV reloaded;
			check(reloaded.loadFromFile(path), name + " reloads");
			checkSamples(reloaded, expected, name + " round trip");
		}
		std::remove(path.c_str());
	}

	Bytes standardWave(const Bytes &format, const Bytes &samples)
	{
		Bytes wave = beginWave();
		appendChunk(wave, "fmt ", format);
		appendChunk(wave, "data", samples);
		finishWave(wave);
		return wave;
	}

	void testStandardSamples()
	{
		TestWAV wav8;
		const Bytes samples8 = { 0x00, 0x7f, 0x80, 0xff };
		check(loadBytes(wav8, standardWave(pcmFormat(8, 1, 1), samples8)),
			"standard 8-bit WAV loads");
		checkSamples(wav8, Samples{ -128, -1, 0, 127 }, "standard 8-bit WAV");
		check(wav8.header->fmtSize == 16, "standard fmt size");
		check(wav8.header->nSamplesPerSec == 44100, "standard sample rate");

		Bytes samples16;
		appendLE16(samples16, 0x8000);
		appendLE16(samples16, 0x0000);
		appendLE16(samples16, 0x7fff);
		appendLE16(samples16, 0xffff);
		appendLE16(samples16, 0xff01);
		appendLE16(samples16, 0xff00);
		appendLE16(samples16, 0x8001);
		TestWAV wav16;
		check(loadBytes(wav16, standardWave(pcmFormat(16, 1, 2), samples16)),
			"standard 16-bit WAV loads");
		checkSamples(wav16, Samples{ -128, 0, 127, 0, 0, -1, -127 },
			"standard 16-bit WAV");
	}

	void testChunkVariants()
	{
		Bytes wave = beginWave();
		appendChunk(wave, "JUNK", Bytes{ 0x11, 0x22, 0x33 });
		Bytes extendedFormat = pcmFormat(8, 1, 1);
		extendedFormat.push_back(0);
		extendedFormat.push_back(0);
		appendChunk(wave, "fmt ", extendedFormat);
		appendChunk(wave, "LIST", Bytes{ 0x44 });
		appendChunk(wave, "data", Bytes{ 0x12, 0x34 });
		finishWave(wave);

		TestWAV extended;
		check(loadBytes(extended, wave), "unknown odd chunks and extended fmt load");
		check(extended.header->fmtSize == 18, "extended fmt size is retained");
		checkSamples(extended, Samples{ -110, -76 }, "extended WAV");

		Bytes reordered = beginWave();
		appendChunk(reordered, "data", Bytes{ 0x42 });
		appendChunk(reordered, "fmt ", pcmFormat(8, 1, 1));
		finishWave(reordered);
		TestWAV dataFirst;
		check(loadBytes(dataFirst, reordered), "data before fmt loads");
		checkSamples(dataFirst, Samples{ -62 }, "data-before-fmt WAV");
	}

	void testSampleProcessing()
	{
		TestWAV normalized;
		const Samples normalizeInput = { -64, -32, 0, 32, 64 };
		check(loadBytes(normalized,
			standardWave(pcmFormat(8, 1, 1), pcm8(normalizeInput))),
			"normalize-only fixture loads");
		normalized.normalize();
		checkSamples(normalized, Samples{ -127, -63, 0, 63, 127 },
			"normalize-only signed samples");

		TestWAV envelopeOnly;
		const Samples envelopeInput = { -100, -100, -100, 100, 100, 100 };
		check(loadBytes(envelopeOnly,
			standardWave(pcmFormat(8, 1, 1), pcm8(envelopeInput))),
			"envelope-only fixture loads");
		envelopeOnly.envelopeCorrection();
		checkSamples(envelopeOnly, Samples{ -100, -100, 14, 87, 98, 100 },
			"envelope-only signed samples");

		TestWAV complete;
		const Samples completeInput = { -64, -64, -64, 64, 64, 64 };
		check(loadBytes(complete,
			standardWave(pcmFormat(8, 1, 1), pcm8(completeInput))),
			"complete pipeline fixture loads");
		complete.normalize();
		checkSaveRoundTrip(complete,
			Samples{ -127, -127, -127, 127, 127, 127 }, "normalized output");
		complete.envelopeCorrection();
		checkSamples(complete, Samples{ -127, -127, 18, 111, 124, 127 },
			"complete signed pipeline");
		checkSaveRoundTrip(complete,
			Samples{ -127, -127, 18, 111, 124, 127 }, "enveloped output");
	}

	void testPhaseSaveIsNonDestructive()
	{
		TestWAV wav;
		wav.setPhase(true);
		const Samples input = { -64, 0, 64, 32 };
		check(loadBytes(wav, standardWave(pcmFormat(8, 1, 1), pcm8(input))),
			"phase fixture loads");
		checkSamples(wav, Samples{ 64, 0, -64, -32 }, "phase-adjusted samples");
		checkSaveRoundTrip(wav, input, "phase output");
		checkSamples(wav, Samples{ 64, 0, -64, -32 },
			"phase output preserves adjusted samples");
	}

	void testMalformedFiles()
	{
		const Bytes valid = standardWave(pcmFormat(8, 1, 1), Bytes{ 0x55, 0xaa });
		Bytes physicallyTruncated = valid;
		physicallyTruncated.pop_back();
		TestWAV wav;
		check(loadBytes(wav, valid), "valid fixture loads before state-reset test");
		check(!loadBytes(wav, physicallyTruncated), "physical truncation is rejected");
		check(wav.getSize() == 0 && wav.samples() == NULL,
			"failed load clears previous samples and size");
		check(wav.header->riffSize == 0, "failed load clears previous header");

		Bytes oversizedChunk = beginWave();
		appendId(oversizedChunk, "JUNK");
		appendLE32(oversizedChunk, 100);
		finishWave(oversizedChunk);
		TestWAV oversized;
		check(!loadBytes(oversized, oversizedChunk), "chunk crossing RIFF boundary is rejected");

		Bytes maximumOddChunk = beginWave();
		appendId(maximumOddChunk, "JUNK");
		appendLE32(maximumOddChunk, 0xffffffffu);
		finishWave(maximumOddChunk);
		TestWAV maximumOdd;
		check(!loadBytes(maximumOdd, maximumOddChunk),
			"maximum odd chunk size cannot overflow padding arithmetic");

		Bytes shortenedRiff = valid;
		setLE32(shortenedRiff, 4, 4);
		TestWAV hiddenChunks;
		check(!loadBytes(hiddenChunks, shortenedRiff),
			"chunks beyond the declared RIFF boundary are ignored and rejected");

		Bytes shortFormat = beginWave();
		appendChunk(shortFormat, "fmt ", Bytes(14, 0));
		appendChunk(shortFormat, "data", Bytes{ 0x00 });
		finishWave(shortFormat);
		TestWAV shortFmt;
		check(!loadBytes(shortFmt, shortFormat), "short fmt chunk is rejected");

		TestWAV badAlignment;
		check(!loadBytes(badAlignment,
			standardWave(pcmFormat(16, 1, 1), Bytes{ 0x00, 0x00 })),
			"invalid PCM block alignment is rejected");

		TestWAV odd16BitData;
		check(!loadBytes(odd16BitData,
			standardWave(pcmFormat(16, 1, 2), Bytes{ 0x00 })),
			"partial 16-bit sample is rejected");

		TestWAV stereo;
		check(!loadBytes(stereo,
			standardWave(pcmFormat(8, 2, 2), Bytes{ 0x00, 0x00 })),
			"stereo PCM is rejected");

		TestWAV nonPcm;
		check(!loadBytes(nonPcm,
			standardWave(pcmFormat(8, 1, 1, 3), Bytes{ 0x00 })),
			"non-PCM format is rejected");

		Bytes missingFmt = beginWave();
		appendChunk(missingFmt, "data", Bytes{ 0x00 });
		finishWave(missingFmt);
		TestWAV missing;
		check(!loadBytes(missing, missingFmt), "missing fmt chunk is rejected");
	}
}

int main()
{
	testStandardSamples();
	testChunkVariants();
	testSampleProcessing();
	testPhaseSaveIsNonDestructive();
	testMalformedFiles();

	if (failures != 0) {
		std::cerr << failures << " test(s) failed" << std::endl;
		return 1;
	}
	std::cout << "All WAV format tests passed" << std::endl;
	return 0;
}
