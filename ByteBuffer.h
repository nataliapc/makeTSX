/*
 * 
 * Based in:
 * https://gist.github.com/dtalley/2234745
 *
 */
#ifndef __BYTEBUFFER_H__
#define __BYTEBUFFER_H__

#include <limits.h>
#include <stdint.h>
#include <iostream>
#include <string>

#include "types.h"


#if CHAR_BIT != 8
#error "unsupported char size"
#endif

namespace Utility
{
	enum
	{
		O32_LITTLE_ENDIAN = 0x03020100ul,
		O32_BIG_ENDIAN = 0x00010203ul,
		O32_POP_ENDIAN = 001000302ul
	};
	//For detecting Host endian type
	static const union
	{
		unsigned char bytes[4];
		uint32_t value;
	}
	o32_host_order = { { 0, 1, 2, 3 } };
	#define O32_HOST_ORDER (o32_host_order.value)

	//Enum for endian types
	enum ByteEndianness {
		ORDER_LITTLE_ENDIAN,
		ORDER_BIG_ENDIAN,
		ORDER_POP_ENDIAN
	};

	class ByteBuffer
	{
	public:
		ByteBuffer(char*, size_t);
		ByteBuffer(const ByteBuffer&);
		~ByteBuffer();

		void SetEndian(ByteEndianness);
		const char* getBytes();
		size_t getSize();
		bool isEOF();

		uint64_t ReadUInt64();
		int64_t ReadInt64();

		uint32_t ReadUInt32();
		int32_t ReadInt32();

		uint24_t ReadUInt24();
		int24_t ReadInt24();

		uint16_t ReadUInt16();
		int16_t ReadInt16();

		double ReadDouble();
		float ReadSingle();

		uint8_t ReadUByte();
		int8_t ReadByte();
		std::string ReadString(unsigned int);

		bool WriteUInt64(uint64_t);
		bool WriteInt64(int64_t);

		bool WriteUInt32(uint32_t);
		bool WriteInt32(int32_t);

		bool WriteUInt24(uint24_t);
		bool WriteInt24(int24_t);

		bool WriteUInt16(uint16_t);
		bool WriteInt16(int16_t);

		bool WriteDouble(double);
		bool WriteSingle(float);

		bool WriteUByte(uint8_t);
		bool WriteByte(int8_t);
		bool WriteString(std::string);

		bool ReadRawData(char *, size_t);
		bool WriteRawData(char *, size_t);
		bool WriteRawData(std::istream &, size_t);

		void StoreBytes(char*, char*, size_t);

		long unsigned int Tell();
		void Seek(long unsigned int);

		ByteBuffer& operator=(const ByteBuffer &);

	private:
		char* data;
		size_t size;
		ByteEndianness endian;
		long unsigned int pointer;

		template <class T> T ReadAny();
		template <class T> bool WriteAny(T);
	};
}

#endif //__BYTEBUFFER_H__
