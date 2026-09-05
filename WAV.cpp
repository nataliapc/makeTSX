
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdexcept>


#include "WAV.h"


using namespace WAV_Class;
using namespace Utility;


// ============================================================================================
// Constructors / Destructor

WAV::WAV()
{
	header = new Header();
	data = NULL;
	size = 0;
}

WAV::WAV(string filename)
{
	header = new Header();
	data = NULL;
	size = 0;
	loadFromFile(filename);
}

WAV::WAV(const WAV& other) : WAV()
{
	std::memcpy((void *)header, (const void*)other.header, sizeof(Header));
	size = other.size;
	phase = other.phase;
	if (size > 0) {
		data = new int8_t[size];
		std::memcpy((void *)data, (const void*)other.data, size);
	}
}

WAV::~WAV()
{
	if (data!=NULL) delete[] data;
	delete header;
}


// ============================================================================================
// Getters / Setters

size_t WAV::getSize()
{
	return size;
}

// ============================================================================================
// Private methods

// ============================================================================================
// Public inteface

void WAV::showInfo()
{
	string riff, wave, fmt;
	riff.assign((char*)(header->riffId), 4);
	wave.assign((char*)(header->waveId), 4);
	fmt.assign((char*)(header->fmtId), 4);
	cout << std::dec;
	cout << "RiffID:       " << riff << endl;
	cout << "RiffSize:     " << header->riffSize << endl;
	cout << "WaveId:       " << wave << endl;
	cout << "FmtId:        " << fmt << endl;
	cout << "FmtSize:      " << header->fmtSize << " bytes" << endl;
	cout << "FmtTag:       " << header->wFormatTag << endl;
	cout << "Channels:     " << (header->nChannels==1 ? "mono" : (header->nChannels==2 ? "stereo" : std::to_string(header->nChannels))) << endl;
	cout << "SamplesxSec:  " << header->nSamplesPerSec << " Hz" << endl;
	cout << "AvgBytesxSec: " << header->nAvgBytesPerSec << endl;
	cout << "BlockAlign:   " << header->nBlockAlign << endl;
	cout << "BitsxSample:  " << header->wBitsPerSample << " bits" << endl;
	cout << "DATA Size:    " << size << endl;
/*	cout << "     AvgHigh: " << avgHigh << endl;
	cout << "     AvgZero: " << avgZero << endl;
	cout << "     AvgLow:  " << avgLow) << endl;
	cout << "     MaxValue:" << maxValue << endl;
	cout << "     MinValue:" << minValue << endl;*/
}

void WAV::clear()
{
	if (data!=NULL) delete[] data;
	data = NULL;
	size = 0;
	std::fill_n((char*)header, sizeof(Header), 0);
}

bool WAV::loadFromFile(string filename)
{
	clear();
	const auto fail = [this](const char *message) -> bool {
		cout << message;
		clear();
		return false;
	};
	const auto readLE16 = [](const char *bytes) -> uint16_t {
		return static_cast<uint16_t>(static_cast<uint8_t>(bytes[0])) |
			(static_cast<uint16_t>(static_cast<uint8_t>(bytes[1])) << 8);
	};
	const auto readLE32 = [](const char *bytes) -> uint32_t {
		return static_cast<uint32_t>(static_cast<uint8_t>(bytes[0])) |
			(static_cast<uint32_t>(static_cast<uint8_t>(bytes[1])) << 8) |
			(static_cast<uint32_t>(static_cast<uint8_t>(bytes[2])) << 16) |
			(static_cast<uint32_t>(static_cast<uint8_t>(bytes[3])) << 24);
	};

	std::ifstream ifs(filename, std::ifstream::in | std::ios::binary);
	if (!ifs.is_open()) return false;

	ifs.seekg(0, std::ios::end);
	const std::streamoff fileSize = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	if (!ifs || fileSize < 12) return fail("Bad WAV header...");

	char riffHeader[12];
	if (!ifs.read(riffHeader, sizeof(riffHeader)) ||
		std::memcmp(riffHeader, MAGIC_RIFFID, 4) != 0 ||
		std::memcmp(riffHeader + 8, MAGIC_WAVEID, 4) != 0) {
		return fail("Bad WAV header...");
	}

	const uint32_t riffSize = readLE32(riffHeader + 4);
	const uint64_t riffEnd = static_cast<uint64_t>(riffSize) + 8;
	if (riffEnd < sizeof(riffHeader) || riffEnd > static_cast<uint64_t>(fileSize)) {
		return fail("Invalid WAV RIFF size...");
	}
	std::memcpy(header->riffId, riffHeader, 4);
	header->riffSize = riffSize;
	std::memcpy(header->waveId, riffHeader + 8, 4);

	bool gotFmt = false;
	bool gotData = false;
	std::streamoff dataPosition = 0;
	while (!gotFmt || !gotData) {
		const std::streamoff chunkPosition = ifs.tellg();
		if (chunkPosition < 0 ||
			static_cast<uint64_t>(chunkPosition) + 8 > riffEnd) {
			return fail("WAV file has no valid fmt/data chunks...");
		}

		char chunkHeader[8];
		if (!ifs.read(chunkHeader, sizeof(chunkHeader))) {
			return fail("Unexpected end of WAV file...");
		}
		const uint32_t chunkSize = readLE32(chunkHeader + 4);
		const uint64_t payloadPosition = static_cast<uint64_t>(chunkPosition) + 8;
		const uint64_t paddedChunkSize = static_cast<uint64_t>(chunkSize) + (chunkSize & 1u);
		if (payloadPosition > riffEnd || paddedChunkSize > riffEnd - payloadPosition) {
			return fail("Invalid WAV chunk size...");
		}
		const uint64_t chunkEnd = payloadPosition + paddedChunkSize;

		if (!gotFmt && std::memcmp(chunkHeader, MAGIC_FMTID, 4) == 0) {
			if (chunkSize < 16) return fail("Invalid WAV fmt chunk...");
			char fmtData[16];
			if (!ifs.read(fmtData, sizeof(fmtData))) {
				return fail("Unexpected end of WAV fmt chunk...");
			}
			std::memcpy(header->fmtId, chunkHeader, 4);
			header->fmtSize = chunkSize;
			header->wFormatTag = readLE16(fmtData);
			header->nChannels = readLE16(fmtData + 2);
			header->nSamplesPerSec = readLE32(fmtData + 4);
			header->nAvgBytesPerSec = readLE32(fmtData + 8);
			header->nBlockAlign = readLE16(fmtData + 12);
			header->wBitsPerSample = readLE16(fmtData + 14);
			gotFmt = true;
		} else if (!gotData && std::memcmp(chunkHeader, MAGIC_DATAID, 4) == 0) {
			std::memcpy(header->dataId, chunkHeader, 4);
			header->dataSize = chunkSize;
			dataPosition = static_cast<std::streamoff>(payloadPosition);
			gotData = true;
		}

		ifs.seekg(static_cast<std::streamoff>(chunkEnd), std::ios::beg);
		if (!ifs) return fail("Invalid WAV chunk position...");
	}

	if (header->wFormatTag != 1 || header->nChannels != 1 ||
		(header->wBitsPerSample != 8 && header->wBitsPerSample != 16) ||
		header->nSamplesPerSec == 0) {
		return fail("Only PCM mono 8/16-bit WAVs are supported...");
	}
	const WORD expectedBlockAlign = header->wBitsPerSample / 8;
	if (header->nBlockAlign != expectedBlockAlign ||
		header->dataSize % expectedBlockAlign != 0) {
		return fail("Invalid WAV block alignment...");
	}

	ifs.seekg(dataPosition, std::ios::beg);
	if (!ifs) return fail("Invalid WAV data position...");

	// Preserve the existing unsigned intermediate sample representation used by the filters.
	if (header->wBitsPerSample == 8) {
		size = header->dataSize;
		if (size > 0) {
			data = new int8_t[size];
			if (!ifs.read((char *)data, static_cast<std::streamsize>(size))) {
				return fail("Unexpected end of WAV data...");
			}
		}
	} else {
		size = header->dataSize / 2;
		if (size > 0) data = new int8_t[size];
		for (size_t i=0; i<size; i++) {
			char sample[2];
			if (!ifs.read(sample, sizeof(sample))) {
				return fail("Unexpected end of WAV data...");
			}
			const uint16_t raw = readLE16(sample);
			const int32_t value16 = (raw & 0x8000u)
				? static_cast<int32_t>(raw) - 0x10000
				: static_cast<int32_t>(raw);
			data[i] = (value16/256 - 0x80) & 0xFF;
		}
	}

	if (phase) {
		for (uint32_t i=0; i<size; i++) {
			data[i] = -data[i];
		}
	}

	return true;
}

bool WAV::saveToFile(string filename)
{
	if (phase) {
		for (uint32_t i=0; i<size; i++) {
			data[i] = -data[i];
		}
	}

	std::ofstream ofs (filename, std::ofstream::out | std::ios::binary);
	if (!ofs.is_open()) return false;

	WORD bits = header->wBitsPerSample;
	header->wBitsPerSample = 8;

	ofs.write((char*)header, sizeof(Header));
	ofs.write((char*)data, size);

	header->wBitsPerSample = bits;

	ofs.close();
	return true;
}

void WAV::normalize()
{
	BYTE* bdata = reinterpret_cast<BYTE*>(data);
	DWORD  pos = 0;
	DWORD  len = header->nSamplesPerSec * (0.25f / 1000);	//0.25 ms segments
	int16_t min, max;
	int16_t v;

	while (pos < size) {
		max = -128;
		min = 127;
		for (DWORD i=pos; i<pos+len && i<size; i++) {
			v = (int16_t)bdata[i] - 0x80;
			if (v > max && v > 5) { max = v; }
			if (v < min && v <-5) { min = v; }
		}
		for (DWORD i=pos; i<pos+len && i<size; i++) {
			v = (int16_t)bdata[i] - 0x80;
			if (v > 0) {
				v = v * 127 / max;
			}
			if (v < 0) {
				v = v * -127 / min;
			}
			bdata[i] = (BYTE)(v + 0x80);
		}
		pos += len;
	}
}

void WAV::envelopeCorrection()
{
	const int32_t deviation = 20;

	for (DWORD i=0; i<size; i++) {
		data[i] = (int16_t)(((uint16_t)data[i]&0xff)-0x80);
	}

	int32_t avg;
	for (DWORD i=1; i<size-1; i++) {
		avg = ( ( (int32_t)data[i-1] + (int32_t)data[i] + (int32_t)data[i+1] ) / 3);
		if (std::abs(avg - data[i-1]) < deviation && std::abs(avg - data[i]) < deviation && std::abs(avg - data[i+1]) < deviation) {
			data[i-1] = data[i] = data[i+1] = (int8_t)avg;
		}
	}
	for (DWORD i=1; i<size-1; i++) {
		data[i] = (int8_t)((0.5f * (float)data[i-1] +
							1.0f * (float)data[i] +
							2.0f * (float)data[i+1]) / 3.5f);
	}
}
