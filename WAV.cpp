
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

WAV::WAV(const WAV& other)
{
	std::memcpy((void *)header, (const void*)other.header, sizeof(Header));

	size = other.size;

	data = new int8_t[other.size];
	if (data==NULL) throw std::runtime_error("Out of memory allocation WAV data...");
	std::memcpy((void *)data, (const void*)other.data, other.size);
}

WAV::~WAV()
{
	if (data!=NULL) delete data;
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
	if (data!=NULL) delete data;
	data = NULL;
	std::fill_n((char*)header, sizeof(Header), 0);
}

bool WAV::loadFromFile(string filename)
{
	std::ifstream ifs (filename, std::ifstream::in | std::ios::binary);
	if (!ifs.is_open()) return false;
	clear();

	//Read Header
	ifs.read((char*)header, sizeof(Header));
	if (strncmp(MAGIC_RIFFID, (char*)header->riffId, 4)!=0 ||
		strncmp(MAGIC_WAVEID, (char*)header->waveId, 4)!=0 ||
		strncmp(MAGIC_FMTID, (char*)header->fmtId, 4)!=0) {
		ifs.close();
		cout << "Bad WAV header...";
		return false;
	}

	//Read Data if 8 bits
	if (header->wBitsPerSample == 8) {
		size = header->dataSize;
		data = new int8_t[size];
		if (data==NULL) throw std::runtime_error("Out of memory allocation WAV data...");
		ifs.read((char *)data, size);
	} else
	//Read Data if 16 bits
	if (header->wBitsPerSample == 16) {
		int16_t value16;
		size = header->dataSize / 2;
		data = new int8_t[size];
		if (data==NULL) throw std::runtime_error("Out of memory allocation WAV data...");
		for (size_t i=0; i<size; i++) {
			ifs.read((char *)&value16, sizeof(int16_t));
			data[i] = (value16/256 - 0x80) & 0xFF;
		}
	} else {
		cout << "Only supported 8/16 bits WAVs...";
		return false;
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
	DWORD  len = header->nSamplesPerSec * 0.25f / 1000;		//Segmentos de 2ms
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
