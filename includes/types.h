#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include "colors.h"


enum {
	WORD24_BYTES = 3,
	WORD24_MAX_VALUE = 0xffffff
};

#ifndef uint24_t
struct uint24_t {
	uint8_t b0;
	uint8_t b1;
	uint8_t b2;

	uint24_t() : b0(0), b1(0), b2(0) {}
	uint24_t(uint32_t value)
		: b0(static_cast<uint8_t>(value))
		, b1(static_cast<uint8_t>(value >> 8))
		, b2(static_cast<uint8_t>(value >> 16))
	{
	}
	operator uint32_t() const {
		return static_cast<uint32_t>(b0) |
			(static_cast<uint32_t>(b1) << 8) |
			(static_cast<uint32_t>(b2) << 16);
	}
};
static_assert(sizeof(uint24_t) == WORD24_BYTES, "uint24_t must occupy exactly three bytes");
#endif
#ifndef int24_t
struct int24_t {
	uint8_t b0;
	uint8_t b1;
	uint8_t b2;

	int24_t() : b0(0), b1(0), b2(0) {}
	int24_t(int32_t value) {
		uint32_t raw = static_cast<uint32_t>(value);
		b0 = static_cast<uint8_t>(raw);
		b1 = static_cast<uint8_t>(raw >> 8);
		b2 = static_cast<uint8_t>(raw >> 16);
	}
	operator int32_t() const {
		uint32_t raw = static_cast<uint32_t>(b0) |
			(static_cast<uint32_t>(b1) << 8) |
			(static_cast<uint32_t>(b2) << 16);
		return (raw & 0x800000u)
			? static_cast<int32_t>(raw) - 0x1000000
			: static_cast<int32_t>(raw);
	}
};
static_assert(sizeof(int24_t) == WORD24_BYTES, "int24_t must occupy exactly three bytes");
#endif

typedef uint8_t		BYTE;
typedef uint16_t	WORD;
typedef uint24_t	WORD24;
typedef uint32_t	DWORD;


#endif //__TYPES_H__
