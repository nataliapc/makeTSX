/*
 *
 * Based in:
 * https://gist.github.com/dtalley/2234745
 *
 */

#include <cstring>
#include <stdexcept>

#include "ByteBuffer.h"


using namespace Utility;

// ============================================================================================
// Class ByteBuffer

ByteBuffer::ByteBuffer(char* pdata, size_t psize)
	: data(0)
	, size(psize)
	, endian(ORDER_LITTLE_ENDIAN)
	, pointer(0)
{
	// TZX stores every multibyte value in little-endian order.
	data = new char[size];
	std::fill_n((char*)data, size, 0);
	if (pdata!=NULL) {
		std::memcpy((void *)data, (const void*)pdata, size);
	}
}

ByteBuffer::ByteBuffer(const ByteBuffer& other)
{
	size = other.size;
	endian = other.endian;
	pointer = other.pointer;
	data = new char[size];
	std::memcpy((void *)data, (const void*)(other.data), size);
}

ByteBuffer::~ByteBuffer()
{
	delete[] data;
}

void ByteBuffer::SetEndian(ByteEndianness end)
{
	endian = end;
}

const char* ByteBuffer::getBytes()
{
	return data;
}

size_t ByteBuffer::getSize()
{
	return size;
}

bool ByteBuffer::isEOF()
{
	return pointer>=size;
}

long unsigned int ByteBuffer::Tell()
{
	return pointer;
}

void ByteBuffer::Seek(long unsigned int target)
{
	if (target > size) {
		throw std::out_of_range("ByteBuffer seek past end");
	}
	pointer = target;
}

int8_t ByteBuffer::ReadByte()
{
	return ReadAny<int8_t>();
}

uint8_t ByteBuffer::ReadUByte()
{
	return ReadAny<uint8_t>();
}

float ByteBuffer::ReadSingle()
{
	return ReadAny<float>();
}

double ByteBuffer::ReadDouble()
{
	return ReadAny<double>();
}

int16_t ByteBuffer::ReadInt16()
{
	return ReadAny<int16_t>();
}

uint16_t ByteBuffer::ReadUInt16()
{
	return ReadAny<uint16_t>();
}

int24_t ByteBuffer::ReadInt24()
{
	uint32_t value = ReadUInt24();
	return int24_t((value & 0x800000u)
		? static_cast<int32_t>(value) - 0x1000000
		: static_cast<int32_t>(value));
}

uint24_t ByteBuffer::ReadUInt24()
{
	if (!HasRemaining(WORD24_BYTES)) {
		throw std::out_of_range("ByteBuffer read past end");
	}
	uint32_t value;
	if (endian == ORDER_LITTLE_ENDIAN) {
		value = static_cast<uint8_t>(data[pointer]) |
			(static_cast<uint32_t>(static_cast<uint8_t>(data[pointer + 1])) << 8) |
			(static_cast<uint32_t>(static_cast<uint8_t>(data[pointer + 2])) << 16);
	} else if (endian == ORDER_BIG_ENDIAN) {
		value = (static_cast<uint32_t>(static_cast<uint8_t>(data[pointer])) << 16) |
			(static_cast<uint32_t>(static_cast<uint8_t>(data[pointer + 1])) << 8) |
			static_cast<uint8_t>(data[pointer + 2]);
	} else {
		throw std::invalid_argument("24-bit POP endian order is unsupported");
	}
	pointer += WORD24_BYTES;
	return uint24_t(value);
}

int32_t ByteBuffer::ReadInt32()
{
	return ReadAny<int32_t>();
}

uint32_t ByteBuffer::ReadUInt32()
{
	return ReadAny<uint32_t>();
}

int64_t ByteBuffer::ReadInt64()
{
	return ReadAny<int64_t>();
}

uint64_t ByteBuffer::ReadUInt64()
{
	return ReadAny<uint64_t>();
}

std::string ByteBuffer::ReadString(unsigned int length)
{
	if (!HasRemaining(length)) {
		throw std::out_of_range("ByteBuffer read past end");
	}
	std::string ret((char*)(data + pointer), length);
	pointer += length;
	return ret;
}

bool ByteBuffer::WriteUInt64(uint64_t value)
{
	return WriteAny<uint64_t>(value);
}

bool ByteBuffer::WriteInt64(int64_t value) {
	return WriteAny<int64_t>(value);
}

bool ByteBuffer::WriteUInt32(uint32_t value) {
	return WriteAny<uint32_t>(value);
}

bool ByteBuffer::WriteInt32(int32_t value) {
	return WriteAny<int32_t>(value);
}

bool ByteBuffer::WriteUInt24(uint24_t value) {
	if (!HasRemaining(WORD24_BYTES)) {
		return false;
	}
	uint32_t raw = value;
	if (endian == ORDER_LITTLE_ENDIAN) {
		data[pointer] = static_cast<char>(raw);
		data[pointer + 1] = static_cast<char>(raw >> 8);
		data[pointer + 2] = static_cast<char>(raw >> 16);
	} else if (endian == ORDER_BIG_ENDIAN) {
		data[pointer] = static_cast<char>(raw >> 16);
		data[pointer + 1] = static_cast<char>(raw >> 8);
		data[pointer + 2] = static_cast<char>(raw);
	} else {
		throw std::invalid_argument("24-bit POP endian order is unsupported");
	}
	pointer += WORD24_BYTES;
	return true;
}

bool ByteBuffer::WriteInt24(int24_t value) {
	return WriteUInt24(uint24_t(static_cast<uint32_t>(static_cast<int32_t>(value))));
}

bool ByteBuffer::WriteUInt16(uint16_t value) {
	return WriteAny<uint16_t>(value);
}

bool ByteBuffer::WriteInt16(int16_t value) {
	return WriteAny<int16_t>(value);
}

bool ByteBuffer::WriteDouble(double value) {
	return WriteAny<double>(value);
}

bool ByteBuffer::WriteSingle(float value) {
	return WriteAny<float>(value);
}

bool ByteBuffer::WriteUByte(uint8_t value) {
	return WriteAny<uint8_t>(value);
}

bool ByteBuffer::WriteByte(int8_t value) {
	return WriteAny<int8_t>(value);
}

bool ByteBuffer::WriteString(std::string value) {
	if (!HasRemaining(value.length())) {
		return false;
	}
	std::memcpy((void *)(data + pointer), (const void*)(value.c_str()), value.length());
	pointer += value.length();
	return true;
}

bool ByteBuffer::ReadRawData(char *dst, size_t size) {
	if (!HasRemaining(size)) {
		return false;
	}
	std::memcpy((void *)dst, (const void *)(data + pointer), size);
	pointer += size;
	return true;
}

bool ByteBuffer::WriteRawData(char *src, size_t size) {
	if (!HasRemaining(size)) {
		return false;
	}
	std::memcpy((void *)(data + pointer), (const void *)src, size);
	pointer += size;
	return true;
}

bool ByteBuffer::WriteRawData(std::istream &is, size_t size)
{
	if (!HasRemaining(size)) {
		return false;
	}
	is.read(data + pointer, static_cast<std::streamsize>(size));
	if (static_cast<size_t>(is.gcount()) != size) {
		throw std::runtime_error("Unexpected end of input stream");
	}
	pointer += size;
	return true;
}

ByteBuffer& ByteBuffer::operator=(const ByteBuffer &other)
{
	size = other.size;
	endian = other.endian;
	pointer = other.pointer;
	data = new char[size]();
	std::memcpy((void*)data, (const void*)other.data, size);
	return *this;
}

template <class T> T ByteBuffer::ReadAny()
{
	if (!HasRemaining(sizeof(T))) {
		throw std::out_of_range("ByteBuffer read past end");
	}
	T ret = T();
	char* dst = (char*)&ret;
	char* src = (char*)&(data[pointer]);
	StoreBytes(src, dst, sizeof(T));
	pointer += sizeof(T);
	return ret;
}

template <class T> bool ByteBuffer::WriteAny(T value)
{
	if (!HasRemaining(sizeof(T))) {
		return false;
	}
	char* src = (char*)&value;
	char* dst = (char*)&(data[pointer]);
	StoreBytes(src, dst, sizeof(T));
	pointer += sizeof(T);
	return true;
}

bool ByteBuffer::HasRemaining(size_t length) const
{
	return pointer <= size && length <= size - pointer;
}

void ByteBuffer::StoreBytes(char* src, char* dst, size_t size)
{
	if (size == 1) {
		dst[0] = src[0];
		return;
	}
	for (size_t i = 0; i < size; i++) {
		if (O32_HOST_ORDER == O32_LITTLE_ENDIAN) {
			if (endian == ORDER_LITTLE_ENDIAN)
				dst[i] = src[i];
			else if (endian == ORDER_BIG_ENDIAN)
				dst[i] = src[(size-i-1)];
			else if (endian == ORDER_POP_ENDIAN)
				dst[i] = src[(i%2==0?(size-i-2):(size-i))];
		} else if (O32_HOST_ORDER == O32_BIG_ENDIAN) {
			if (endian == ORDER_BIG_ENDIAN)
				dst[i] = src[i];
			else if (endian == ORDER_LITTLE_ENDIAN)
				dst[i] = src[(size-i-1)];
			else if (endian == ORDER_POP_ENDIAN)
				dst[i] = src[(i%2==0?(i+1):(i-1))];
		} else {
			if (endian == ORDER_POP_ENDIAN)
				dst[i] = src[i];
			else if (endian == ORDER_LITTLE_ENDIAN)
				dst[i] = src[(i%2==0?(size-i-2):(size-i))];
			else if (endian == ORDER_BIG_ENDIAN)
				dst[i] = src[(i%2==0?(i+1):(i-1))];
		}
	}
}
