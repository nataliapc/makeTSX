#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "rippers/B10_Standard_Ripper.h"
#include "rippers/B11_Custom_Ripper.h"

namespace
{
	typedef std::vector<DWORD> Pulses;
	typedef std::vector<BYTE> Bytes;
	const Bytes payload = { 0x96, 0x3c, 0xa5, 0x0f };
	const Bytes zeroFirstPayload = { 0x00, 0xa5, 0x5a, 0xff };
	int failures = 0;

	void check(bool condition, const std::string &message)
	{
		if (!condition) {
			std::cerr << "FAIL: " << message << std::endl;
			failures++;
		}
	}

	class SilentWAV : public WAV_Class::WAV
	{
	public:
		SilentWAV()
		{
			header->nSamplesPerSec = 44100;
			size = 4;
			data = new int8_t[size]();
		}
	};

	template<class Detector>
	class TestRipper : public Detector
	{
	public:
		template<class... Args>
		TestRipper(WAV_Class::WAV *wav, Args&&... args)
			: Detector(wav, std::forward<Args>(args)...)
		{
		}

		~TestRipper()
		{
			clearBlock();
		}

		using Detector::checkPilot;

		void load(const Pulses &pulses, size_t start = 0)
		{
			clearBlock();
			Pulses(pulses).swap(this->states);
			this->samples.clear();
			DWORD sample = 0;
			for (DWORD pulse : pulses) {
				this->samples.push_back(sample);
				sample += pulse;
			}
			this->pos = start;
		}

	private:
		void clearBlock()
		{
			if (this->block != NULL) {
				if (this->block->getId() == B10_STD_BLOCK)
					delete static_cast<TZX_Blocks::Block10 *>(this->block);
				else
					delete static_cast<TZX_Blocks::Block11 *>(this->block);
				this->block = NULL;
			}
		}
	};

	typedef TestRipper<Rippers::B10_Standard_Ripper> StandardRipper;
	typedef TestRipper<Rippers::B11_Custom_Ripper> CustomRipper;

	Pulses signal(const Pulses &sync, size_t pilots = 100, DWORD pilot = 27,
		DWORD zero = 11, DWORD one = 21, const Bytes &data = payload)
	{
		Pulses pulses(pilots, pilot);
		pulses.insert(pulses.end(), sync.begin(), sync.end());
		for (BYTE byte : data) {
			for (unsigned mask = 0x80; mask != 0; mask >>= 1) {
				pulses.insert(pulses.end(), 2, (byte & mask) ? one : zero);
			}
		}
		// Keep unrelated data-decoder and diagnostic lookahead away from EOF.
		pulses.insert(pulses.end(), 16, 100);
		return pulses;
	}

	void expectPayload(WAV_Class::BlockRipper &ripper, BYTE id, DWORD pilots,
		const std::string &name, const Bytes &expected = payload)
	{
		check(ripper.detectBlock(), name + " detects block");
		TZX_Blocks::Block *block = ripper.getDetectedBlock();
		check(block != NULL, name + " produces block");
		if (block == NULL) return;
		check(block->getId() == id, name + " block ID");
		const size_t head = block->getHeadSize();
		check(block->getSize() == head + expected.size(), name + " payload length");
		if (block->getSize() > head) {
			const BYTE *bytes = reinterpret_cast<const BYTE *>(block->getBytes());
			check(bytes[head] == expected[0], name + " first data byte");
			check(Bytes(bytes + head, bytes + block->getSize()) == expected,
				name + " complete payload");
			if (id == B11_TURBO_BLOCK) {
				check((bytes[11] | (static_cast<unsigned>(bytes[12]) << 8)) == pilots,
					name + " stored pilot count");
			}
		}
		check(ripper.getPos() == ripper.getSize(), name + " final cursor");
	}

	void expectFailure(WAV_Class::BlockRipper &ripper, const std::string &name)
	{
		const DWORD start = ripper.getPos();
		check(!ripper.detectBlock(), name + " rejects block");
		check(ripper.getDetectedBlock() == NULL, name + " produces no block");
		check(ripper.getPos() == start, name + " does not consume input");
	}

	template<class Detector>
	void testStandardTimings(TestRipper<Detector> &ripper, BYTE id,
		const std::string &name)
	{
		struct Case {
			const char *name;
			Pulses sync;
			DWORD pilots;
			bool accepted;
		};
		const Case cases[] = {
			{ "nominal", { 8, 9 }, 100, true },
			{ "plus one", { 14, 8, 9 }, 101, true },
			{ "plus one after SYNC1-only base", { 8, 7, 9 }, 101, true },
			{ "no matching pair", { 14, 14 }, 100, false },
			{ "neighbor matches SYNC1 only", { 14, 8, 20 }, 100, false },
			{ "legacy pair-sum fallback", { 5, 13 }, 100, true },
			{ "legacy bad SYNC2 acceptance", { 8, 20 }, 100, true }
		};
		for (const Case &test : cases) {
			// Start away from zero as well, so failed SYNC must preserve entry pos.
			Pulses pulses = signal(test.sync);
			pulses.insert(pulses.begin(), 3, 100);
			ripper.load(pulses, 3);
			const std::string label = name + " " + test.name;
			check(ripper.checkPilot(3) == 100, label + " real pilot boundary");
			if (test.accepted) expectPayload(ripper, id, test.pilots, label);
			else expectFailure(ripper, label);
		}
		// A valid base must win even when base+1 also matches both SYNC lengths.
		ripper.load(signal({ 8, 9 }, 100, 27, 10, 21, zeroFirstPayload));
		expectPayload(ripper, id, 100, name + " valid base wins", zeroFirstPayload);
	}

	void testCustomBoundaries(SilentWAV &wav)
	{
		{
			// SYNC1 is pilot-sized, so the real pilot detector consumes it.
			CustomRipper ripper(&wav, 2168, 2168, 667, 855, 1710, false);
			ripper.load(signal({ 27, 8 }));
			check(ripper.checkPilot(0) == 101, "B11 minus one real pilot boundary");
			expectPayload(ripper, B11_TURBO_BLOCK, 100, "B11 minus one");
		}
		{
			// Around base=101: [27,8] and [27,8] both match. The original
			// [8,27] passes legacy pair-sum validation; data begins at 103.
			CustomRipper ripper(&wav, 2168, 2168, 667, 667, 1334, false);
			ripper.load(signal({ 27, 8, 27 }, 100, 27, 8, 17, zeroFirstPayload));
			check(ripper.checkPilot(0) == 101, "B11 ambiguous real pilot boundary");
			expectPayload(ripper, B11_TURBO_BLOCK, 101, "B11 ambiguous keeps base",
				zeroFirstPayload);
		}
		{
			CustomRipper ripper(&wav, 2168, 667, 735, 855, 1710, false);
			ripper.load(signal({ 14, 8, 9 }, 65534));
			expectPayload(ripper, B11_TURBO_BLOCK, 65535, "B11 plus one at WORD limit");
			ripper.load(signal({ 14, 8, 9 }, 65535));
			expectFailure(ripper, "B11 plus one cannot overflow stored pilot count");
		}
		{
			CustomRipper ripper(&wav, 2168, 2168, 667, 855, 1710, false);
			ripper.load(signal({ 27, 8 }, 65535));
			expectPayload(ripper, B11_TURBO_BLOCK, 65535, "B11 minus one at WORD limit");
			ripper.load(signal({ 27, 8 }, 65536));
			expectFailure(ripper, "B11 minus one cannot retain oversized pilot count");
		}
	}

	template<class Detector>
	void testBounds(TestRipper<Detector> &ripper, const std::string &name)
	{
		for (const Pulses &pulses : { Pulses(), Pulses{ 27 }, Pulses{ 27, 27 } }) {
			ripper.load(pulses);
			const std::string label = name + " short input " + std::to_string(pulses.size());
			check(ripper.checkPilot(0) == 0, label + " no pilot");
			expectFailure(ripper, label);
		}
		for (size_t start : { 3u, 4u }) {
			ripper.load(Pulses(4, 27), start);
			check(ripper.checkPilot(start) == 0, name + " short remaining pilot");
			expectFailure(ripper, name + " short remaining input");
		}
		for (const Pulses &sync : { Pulses(), Pulses{ 8 }, Pulses{ 14 }, Pulses{ 14, 8 } }) {
			Pulses pulses(100, 27);
			pulses.insert(pulses.end(), sync.begin(), sync.end());
			ripper.load(pulses);
			expectFailure(ripper, name + " truncated SYNC " + std::to_string(sync.size()));
		}
	}

	void testOptions(SilentWAV &wav)
	{
		{
			CustomRipper ripper(&wav, 2168, 667, 735, 855, 1710, true);
			ripper.load(signal({ 8, 9 }, 0));
			expectPayload(ripper, B11_TURBO_BLOCK, 0, "B11 nopilot nominal");
		}
		{
			CustomRipper ripper(&wav, 2168, 667, 735, 855, 1710, true);
			ripper.load(signal({ 14, 8, 9 }, 0));
			expectFailure(ripper, "B11 nopilot must not retry plus one");
		}
		for (bool nopilot : { false, true }) {
			for (const Pulses &sync : { Pulses{ 0, 0 }, Pulses{ 0, 735 }, Pulses{ 667, 0 } }) {
				CustomRipper ripper(&wav, 2168, sync[0], sync[1], 855, 1710, nopilot);
				const DWORD pilots = nopilot ? 0 : 100;
				ripper.load(signal({}, pilots, 27, 11, 21, zeroFirstPayload));
				expectPayload(ripper, B11_TURBO_BLOCK, pilots,
					"B11 disabled SYNC " + std::to_string(sync[0]) + "/" +
					std::to_string(sync[1]) + (nopilot ? " nopilot" : " with pilot"),
					zeroFirstPayload);
			}
		}
		for (const Pulses &pulses : { Pulses(), Pulses{ 8 }, Pulses{ 14, 8 } }) {
			CustomRipper ripper(&wav, 2168, 667, 735, 855, 1710, true);
			ripper.load(pulses);
			expectFailure(ripper, "B11 nopilot truncated SYNC " + std::to_string(pulses.size()));
		}
	}
}

int main()
{
	std::ostringstream log;
	std::streambuf *output = std::cout.rdbuf(log.rdbuf());
	WAV_Class::BlockRipper::setVerboseMode(false);
	WAV_Class::BlockRipper::setInteractiveMode(false);
	WAV_Class::BlockRipper::setPredictiveMode(false);
	SilentWAV wav;
	{
		StandardRipper ripper(&wav);
		testStandardTimings(ripper, B10_STD_BLOCK, "B10");
	}
	{
		CustomRipper ripper(&wav, 2168, 667, 735, 855, 1710, false);
		testStandardTimings(ripper, B11_TURBO_BLOCK, "B11");
	}
	for (DWORD pilots : { 3223u, 8063u }) {
		StandardRipper standard(&wav);
		standard.load(signal({ 4, 17 }, pilots));
		expectPayload(standard, B10_STD_BLOCK, pilots, "B10 wider legacy pair-sum tolerance");
		CustomRipper custom(&wav, 2168, 667, 735, 855, 1710, false);
		custom.load(signal({ 4, 17 }, pilots));
		expectPayload(custom, B11_TURBO_BLOCK, pilots, "B11 wider legacy pair-sum tolerance");
	}
	{
		StandardRipper standard(&wav);
		standard.load(signal({ 14, 7, 7 }, 100, 25, 10, 20));
		expectPayload(standard, B10_STD_BLOCK, 101, "B10 calibrated plus one");
		CustomRipper custom(&wav, 2168, 667, 735, 855, 1710, false);
		custom.load(signal({ 14, 7, 7 }, 100, 25, 10, 20));
		expectPayload(custom, B11_TURBO_BLOCK, 101, "B11 calibrated plus one");
	}
	testCustomBoundaries(wav);
	WAV_Class::BlockRipper::setVerboseMode(true);
	{
		StandardRipper ripper(&wav);
		testBounds(ripper, "B10");
	}
	{
		CustomRipper ripper(&wav, 2168, 667, 735, 855, 1710, false);
		testBounds(ripper, "B11");
	}
	testOptions(wav);
	std::cout.rdbuf(output);
	if (failures != 0) {
		std::cerr << log.str();
		std::cerr << failures << " SYNC test(s) failed" << std::endl;
		return 1;
	}
	std::cout << "All SYNC tests passed" << std::endl;
	return 0;
}
