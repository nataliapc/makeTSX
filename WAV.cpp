
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
	cout << "Channels:     " << (header->nChannels==1 ? "mono" : (header->nChannels==2 ? "stereo" : ""+header->nChannels)) << endl;
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

	ofs.write((char*)header, sizeof(Header));
	ofs.write((char*)data, size);

	ofs.close();
	return true;
}

void WAV::normalize()
{
/*	int8_t max=-128, min=127, ii;
	long unsigned int cmax=0, cmin=0;
	long unsigned int count[256];
	BYTE* bdata = reinterpret_cast<BYTE*>(data);

	//Detect max and min peaks
	for (WORD i=0; i<256; i++) count[i]=0;
	for (DWORD i=0; i<size; i++) {
		count[bdata[i]]++;
//printf("%-d -> %-d\n",data[i], bdata[i]);
	}
	for (WORD i=0; i<256; i++) {
		ii = static_cast<int8_t>(i);
printf("%-d -> %-d (%lu)\n",i, ii, count[i]);
		if (count[i]>cmax && ii>max) {
printf("OldMax: %d(%lu)  NewMax: %d(%lu)\n", max, cmax, ii, count[i]);
			max = ii;
			cmax = count[i];
		} else
		if (count[i]>cmin && ii<min) {
printf("OldMin: %d(%lu)  NewMin: %d(%lu)\n", min, cmin, ii, count[i]);
			min = ii;
			cmin = count[i];
		}
//printf("%02X ", (BYTE)data[i]); cout << (signed int)data[i] << endl;
	}
cout << (signed int)min << " " << (signed int)max << endl;
	
	int16_t range = max-min;
	int16_t v;
printf("range: 0x%02X %d\n", (BYTE)range, range);
	for (DWORD i=0; i<size; i++) {
printf("%-d -> ", data[i]);
		v = ((int16_t)data[i]) - min;
printf("%-d -> ", v);
		v = v * 240 / range;
		data[i] = (int8_t)(v - 120);
printf("%-d\n", data[i]);
	}
*/}

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

/*
public class WAV {

    // ============================================================================================
    // Constantes

    // ============================================================================================
    // Variables

    int avgHigh;
    int avgZero;
    int avgLow;
    int maxValue;
    int minValue;

    // ============================================================================================
    // Constructores

    private void checkLevels() {
        int[] values = new int[256];
        maxValue = minValue = 0;
        for (int i=0; i<data.length; i++) {
            values[data[i]&0xff]++;
            if (data[i]>maxValue) maxValue = data[i];
            if (data[i]<minValue) minValue = data[i];
        }

        BigInteger sum, num;
        sum = BigInteger.valueOf(0);
        num = BigInteger.valueOf(0);
        for (int i=0; i<90; i++) {
            num = num.add(BigInteger.valueOf(values[i]));
            sum = sum.add(BigInteger.valueOf(values[i]).multiply(BigInteger.valueOf(i)));
//System.out.println("["+i+"]: "+values[i]);
        }
        avgLow = num.intValue()==0 ? 255 : sum.divide(num).intValue();

        sum = BigInteger.valueOf(0);
        num = BigInteger.valueOf(0);
        for (int i=90; i<166; i++) {
            num = num.add(BigInteger.valueOf(values[i]));
            sum = sum.add(BigInteger.valueOf(values[i]).multiply(BigInteger.valueOf(i)));
//System.out.println("["+i+"]: "+values[i]);
        }
        avgZero = num.intValue()==0 ? 128 : sum.divide(num).intValue();

        sum = BigInteger.valueOf(0);
        num = BigInteger.valueOf(0);
        for (int i=166; i<256; i++) {
            num = num.add(BigInteger.valueOf(values[i]));
            sum = sum.add(BigInteger.valueOf(values[i]).multiply(BigInteger.valueOf(i)));
//System.out.println("["+i+"]: "+values[i]);
        }
        avgHigh = num.intValue()==0 ? 0 : sum.divide(num).intValue();
    }

    // ============================================================================================
    // Getters / Setters

    public int getAvgHigh() {
        return avgHigh;
    }

    public int getAvgZero() {
        return avgZero;
    }

    public int getAvgLow() {
        return avgLow;
    }

    // ============================================================================================
    // Public interface

    public void correctEnvelope()
    {
        final int desviacion = 40;
        int avg;
        for (int i=1; i<data.length-1; i++) {
            avg = (data[i - 1] + data[i] + data[i + 1]) / 3;
            if (Math.abs(avg - data[i - 1]) < desviacion && Math.abs(avg - data[i]) < desviacion && Math.abs(avg - data[i + 1]) < desviacion) {
                data[i - 1] = data[i] = data[i + 1] = (byte) avg;
            }
        }
        for (int i=1; i<data.length-1; i++) {
            data[i] = (byte)((0.5f * (float)data[i-1] +
                              1.0f * (float)data[i] +
                              2.0f * (float)data[i+1]) / 3.5f);
        }
    }

    // ============================================================================================
    // Detect blocks

    public static final int THRESHOLD_SILENCE = 100;

    public void detectBlocks() {
        TZX tzx = new TZX();
        int threshold = 5;
        int idx = 1, iaux;

        System.out.println("Decoding audio data...");

        if (isSilence(idx, threshold)) {
            System.out.printf("[%.4f sec] Silence start", (float)idx/header.nSamplesPerSec);
            idx = skipSilence(idx, threshold);
            System.out.printf("[%.4f sec] Silence end", (float)idx/header.nSamplesPerSec);
        }
        while (idx<data.length) {
            idx++;
        }
    }

    public boolean isSilence(int index, int threshold)
    {
        int silent=0;

        while (index < data.length && silent<THRESHOLD_SILENCE) {
            if (data[index] >= threshold || data[index] <= -threshold ) return false;
            silent++;
            index++;
        }
        return true;
    }

    public int skipSilence(int index, int threshold)
    {
        while (index < data.length && (data[index] <= threshold && data[index] >= -threshold )) {
            index++;
        }
        return index;
    }

}
*/