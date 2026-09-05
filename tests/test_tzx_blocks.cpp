#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "ByteBuffer.h"
#include "TZX_Blocks.h"

using TZX_Blocks::Block;
using TZX_Blocks::Block11;
using TZX_Blocks::Block14;
using TZX_Blocks::Block15;
using TZX_Blocks::Block40;
using Utility::ByteBuffer;
using Utility::ORDER_BIG_ENDIAN;

namespace
{
	int failures = 0;

	void check(bool condition, const std::string &message)
	{
		if (!condition) {
			std::cerr << "FAIL: " << message << std::endl;
			failures++;
		}
	}

	template <class Exception, class Action>
	void checkThrows(Action action, const std::string &message)
	{
		try {
			action();
			check(false, message);
		} catch (const Exception &) {
		} catch (...) {
			check(false, message + " threw the wrong exception");
		}
	}

	void checkBytes(Block &block, const std::vector<uint8_t> &expected, const std::string &name)
	{
		check(block.getSize() == expected.size(), name + " size");
		if (block.getSize() != expected.size()) {
			return;
		}

		const char *actual = block.getBytes();
		for (size_t i = 0; i < expected.size(); i++) {
			if (static_cast<uint8_t>(actual[i]) != expected[i]) {
				std::cerr << "FAIL: " << name << " byte " << i
					<< " expected " << static_cast<unsigned>(expected[i])
					<< " got " << static_cast<unsigned>(static_cast<uint8_t>(actual[i]))
					<< std::endl;
				failures++;
				return;
			}
		}
	}

	std::string bodyWithSentinel(Block &block)
	{
		std::string body(block.getBytes() + 1, block.getSize() - 1);
		body.push_back(static_cast<char>(0x55));
		return body;
	}

	void checkSentinel(std::istringstream &input, const std::string &name)
	{
		check(input.get() == 0x55, name + " consumed the next block byte");
	}

	void testUInt24()
	{
		ByteBuffer buffer(NULL, WORD24_BYTES);
		check(buffer.WriteUInt24(uint24_t(0x123456)), "WriteUInt24 succeeds");
		check(buffer.Tell() == WORD24_BYTES, "WriteUInt24 advances exactly three bytes");
		const char *bytes = buffer.getBytes();
		check(static_cast<uint8_t>(bytes[0]) == 0x56 &&
			static_cast<uint8_t>(bytes[1]) == 0x34 &&
			static_cast<uint8_t>(bytes[2]) == 0x12,
			"WriteUInt24 writes little-endian bytes");
		buffer.Seek(0);
		check(static_cast<uint32_t>(buffer.ReadUInt24()) == 0x123456,
			"ReadUInt24 restores the value");

		ByteBuffer bigEndian(NULL, WORD24_BYTES);
		bigEndian.SetEndian(ORDER_BIG_ENDIAN);
		check(bigEndian.WriteUInt24(uint24_t(0x123456)), "big-endian WriteUInt24 succeeds");
		check(static_cast<uint8_t>(bigEndian.getBytes()[0]) == 0x12 &&
			static_cast<uint8_t>(bigEndian.getBytes()[1]) == 0x34 &&
			static_cast<uint8_t>(bigEndian.getBytes()[2]) == 0x56,
			"WriteUInt24 respects explicit big-endian order");
		bigEndian.Seek(0);
		check(static_cast<uint32_t>(bigEndian.ReadUInt24()) == 0x123456,
			"ReadUInt24 respects explicit big-endian order");

		ByteBuffer signedBuffer(NULL, WORD24_BYTES);
		check(signedBuffer.WriteInt24(int24_t(-2)), "WriteInt24 succeeds");
		check(static_cast<uint8_t>(signedBuffer.getBytes()[0]) == 0xfe &&
			static_cast<uint8_t>(signedBuffer.getBytes()[1]) == 0xff &&
			static_cast<uint8_t>(signedBuffer.getBytes()[2]) == 0xff,
			"WriteInt24 writes 24-bit two's complement");
		signedBuffer.Seek(0);
		check(static_cast<int32_t>(signedBuffer.ReadInt24()) == -2,
			"ReadInt24 sign-extends the value");

		ByteBuffer tooSmall(NULL, WORD24_BYTES - 1);
		check(!tooSmall.WriteUInt24(uint24_t(1)), "WriteUInt24 rejects a short buffer");
		check(tooSmall.Tell() == 0, "failed WriteUInt24 does not advance");
		check(!tooSmall.WriteUInt32(1), "generic writes reject a short buffer");
		try {
			tooSmall.ReadUInt24();
			check(false, "ReadUInt24 rejects a short buffer");
		} catch (const std::out_of_range &) {
		}
	}

	void testBlock11()
	{
		char data[] = { static_cast<char>(0xaa), static_cast<char>(0xbb), static_cast<char>(0xcc) };
		Block11 generated(0x0102, 0x0304, 0x0506, 0x0708, 0x090a, 0x0b0c, 8, 0x0d0e, data, sizeof(data));
		std::vector<uint8_t> expected = {
			0x11, 0x02, 0x01, 0x04, 0x03, 0x06, 0x05, 0x08, 0x07,
			0x0a, 0x09, 0x0c, 0x0b, 0x08, 0x0e, 0x0d, 0x03, 0x00,
			0x00, 0xaa, 0xbb, 0xcc
		};
		checkBytes(generated, expected, "Block11 generated");

		std::istringstream input(bodyWithSentinel(generated));
		Block11 parsed(input);
		checkBytes(parsed, expected, "Block11 parsed");
		checkSentinel(input, "Block11");

		std::string truncated(generated.getBytes() + 1, generated.getSize() - 2);
		checkThrows<std::runtime_error>([&truncated]() {
			std::istringstream shortInput(truncated);
			Block11 shortBlock(shortInput);
		}, "Block11 rejects truncated data");
	}

	void testBlock14()
	{
		char data[] = { static_cast<char>(0xaa), static_cast<char>(0xbb), static_cast<char>(0xcc) };
		Block14 generated(0x0102, 0x0304, 7, 0x0506, data, sizeof(data));
		std::vector<uint8_t> expected = {
			0x14, 0x02, 0x01, 0x04, 0x03, 0x07, 0x06, 0x05,
			0x03, 0x00, 0x00, 0xaa, 0xbb, 0xcc
		};
		checkBytes(generated, expected, "Block14 generated");

		std::istringstream input(bodyWithSentinel(generated));
		Block14 parsed(input);
		checkBytes(parsed, expected, "Block14 parsed");
		checkSentinel(input, "Block14");

		std::string truncated(generated.getBytes() + 1, generated.getSize() - 2);
		checkThrows<std::runtime_error>([&truncated]() {
			std::istringstream shortInput(truncated);
			Block14 shortBlock(shortInput);
		}, "Block14 rejects truncated data");
	}

	void testBlock15()
	{
		char data[] = { static_cast<char>(0xaa), static_cast<char>(0xbb), static_cast<char>(0xcc) };
		Block15 generated(0x0102, 0x0304, 6, data, sizeof(data));
		std::vector<uint8_t> expected = {
			0x15, 0x02, 0x01, 0x04, 0x03, 0x06, 0x03, 0x00,
			0x00, 0xaa, 0xbb, 0xcc
		};
		checkBytes(generated, expected, "Block15 generated");

		std::istringstream input(bodyWithSentinel(generated));
		Block15 parsed(input);
		checkBytes(parsed, expected, "Block15 parsed");
		checkSentinel(input, "Block15");

		std::string truncated(generated.getBytes() + 1, generated.getSize() - 2);
		checkThrows<std::runtime_error>([&truncated]() {
			std::istringstream shortInput(truncated);
			Block15 shortBlock(shortInput);
		}, "Block15 rejects truncated data");
	}

	void testBlock40()
	{
		const char raw[] = {
			0x01, 0x03, 0x00, 0x00,
			static_cast<char>(0xaa), static_cast<char>(0xbb), static_cast<char>(0xcc),
			0x55
		};
		std::istringstream input(std::string(raw, sizeof(raw)));
		Block40 parsed(input);
		std::vector<uint8_t> expected = {
			0x40, 0x01, 0x03, 0x00, 0x00, 0xaa, 0xbb, 0xcc
		};
		checkBytes(parsed, expected, "Block40 parsed");
		checkSentinel(input, "Block40");

		const char truncatedRaw[] = {
			0x01, 0x03, 0x00, 0x00,
			static_cast<char>(0xaa), static_cast<char>(0xbb)
		};
		checkThrows<std::runtime_error>([&truncatedRaw]() {
			std::istringstream shortInput(std::string(truncatedRaw, sizeof(truncatedRaw)));
			Block40 shortBlock(shortInput);
		}, "Block40 rejects truncated data");
	}

	void testOversizedPayloads()
	{
		std::vector<char> data(static_cast<size_t>(WORD24_MAX_VALUE) + 1, 0);
		checkThrows<std::length_error>([&data]() {
			Block11 block(1, 1, 1, 1, 1, 1, 8, 0, data.data(), data.size());
		}, "Block11 rejects data larger than its 24-bit length");
		checkThrows<std::length_error>([&data]() {
			Block14 block(1, 1, 8, 0, data.data(), data.size());
		}, "Block14 rejects data larger than its 24-bit length");
		checkThrows<std::length_error>([&data]() {
			Block15 block(1, 0, 8, data.data(), data.size());
		}, "Block15 rejects data larger than its 24-bit length");
	}
}

int main()
{
	testUInt24();
	testBlock11();
	testBlock14();
	testBlock15();
	testBlock40();
	testOversizedPayloads();

	if (failures != 0) {
		std::cerr << failures << " test(s) failed" << std::endl;
		return 1;
	}
	std::cout << "All TZX format tests passed" << std::endl;
	return 0;
}
