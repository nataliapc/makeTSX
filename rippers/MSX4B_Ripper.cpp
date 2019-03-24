
#include <cmath>
#include <vector>
#include <cstring>

#include "MSX4B_Ripper.h"


using namespace std;
using namespace WAV_Class;
using namespace Rippers;

// getByte() return values
#define RET_END					0xFFF0
#define RET_ABORT				0xFFF1
#define RET_PILOT				0xFFFE
#define RET_SILENCE				0xFFFF


/*
	@param wav		WAV object
	@param p0		Pulses in a ZERO bit {1-16 default:2}
	@param p1		Pulses in a ONE bit {1-16 default:4}
	@param lv		Leading bits value {0/1 default:0}
	@param tv		Trailing bits value {0/1 default:1}
	@param lb		Check for 'n' leading bits {0-3 default:2}
	@param tb		Check for 'n' trailing bits {0-3 default:1}
	@param sbf		Significant bits first {0:Lsb 1:Msb default:0}
*/
MSX4B_Ripper::MSX4B_Ripper(WAV *wav, BYTE p0, BYTE p1, BYTE lv, BYTE tv, BYTE lb, BYTE tb, BYTE sbf) : BlockRipper(wav)
{
	this->pulses0 = p0;
	this->pulses1 = p1;
	this->leadingValue = lv;
	this->trailingValue = tv;
	this->leadingBits = lb;
	this->trailingBits = tb;
	this->sigBitsFirst = sbf;

	isMSX = p0==2 && p1==4 && lv==0 && tv==1 && lb==1 && tb==2 && sbf==0;
}

MSX4B_Ripper::MSX4B_Ripper(const MSX4B_Ripper& other) : BlockRipper(other)
{
	this->pulses0 = other.pulses0;
	this->pulses1 = other.pulses1;
	this->leadingValue = other.leadingValue;
	this->trailingValue = other.trailingValue;
	this->leadingBits = other.leadingBits;
	this->trailingBits = other.trailingBits;
	this->sigBitsFirst = other.sigBitsFirst;
	this->isMSX = other.isMSX;
}

MSX4B_Ripper::~MSX4B_Ripper()
{
}

bool MSX4B_Ripper::detectSilence(DWORD pos)
{
	if (!eof(pos) && states[pos] >= THRESHOLD_SILENCE) return true;
	if (!eof(pos+1) && states[pos+1] >= THRESHOLD_SILENCE) return true;
	if (!eof(pos+2) && states[pos+2] >= THRESHOLD_SILENCE) return true;
	if (!eof(pos+3) && states[pos+3] >= THRESHOLD_SILENCE) return true;
	if (!eof(pos+4) && states[pos+4] >= THRESHOLD_SILENCE) return true;
	if (!eof(pos+5) && states[pos+5] >= THRESHOLD_SILENCE) return true;
	return false;
}

bool MSX4B_Ripper::detectBlock()
{
	string type;
	block = NULL;

	//Initialize block
	memset(&blockInfo, 0, sizeof(BlockInfo));
	blockInfo.bitcfg = (pulses0 << 4) | pulses1;					// Default for MSX: 0x24
	blockInfo.bytecfg = (leadingBits << 6) | (leadingValue << 5) |	// Default for MSX: 0x54
						(trailingBits << 3) | (trailingValue << 2) |
						sigBitsFirst;

	type = isMSX ? "MSX" : "KCS";

	//Disable predictive mode is not MSX mode
	if (!isMSX) setPredictiveMode(false);

	//Calculate bauds from pilot pulses
	DWORD pilots = checkPilot(pos);

	//Check if it's a pilot header
	if (this->bauds && pilots) {
		WORD roundedBauds = bauds;
		if (ABS(bauds, 1200) < 15) roundedBauds = 1200;
		if (ABS(bauds, 2400) < 15) roundedBauds = 2400;
		cout << WAVTIME(pos) << TXT_B_GREEN << "Detected #4B " << type << " Pilot tone (" << std::dec << bauds << " bauds)" << TXT_RESET << endl;
		if (isMSX && bauds == roundedBauds) {
			cout << WAVTIME(pos) << MSG_WARNING << ": No standard baudrate!" << endl;
		}
		blockInfo.pilot = MSX_PULSE(bauds);
		blockInfo.bit1len = blockInfo.pilot;
		blockInfo.bit0len = blockInfo.bit1len*2;

		//Header pulses
		DWORD posIni = pos + pilots;
		blockInfo.pulses = pilots;
		if (states[posIni] > THRESHOLD_SILENCE) {
			blockInfo.pausems = states[posIni];
			cout << WAVTIME(posIni) << "Pulse aberration found, skip & continue with header detection..." << endl;
			posIni++;
		}
		float headerSecs = (samples[posIni]-samples[pos])/(float)WAVSampleRate;
		pos = posIni;
		cout << WAVTIME(pos) << "Header pulses: " << std::dec << blockInfo.pulses << " in " << headerSecs << " seconds" << NEXTPULSES(pos) << endl;
		if (isMSX && blockInfo.pulses < 2500) {
			cout << WAVTIME(pos) << MSG_WARNING << ": Detected header with very few pulses... " << endl;
		}

		//Get Data bytes
		WORD value = 0;
		vector<BYTE> buff;
		while (!eof()) {
			value = getByte();
			if (value>=RET_END) break;
			if (verboseMode) cout << WAVTIME(pos) << std::hex << TXT_B_WHITE << "Pos:[0x" << buff.size() << "] Detected BYTE #" << std::hex << value << " (" << std::dec << value << ")" << TXT_RESET << endl;
			buff.push_back(value);
		}
		cout << WAVTIME(pos) << "Extracted data: " << (buff.size()) << " bytes" << endl;

		//Pulses after data block?
		if (value!=RET_PILOT) {
			posIni = pos;
			WORD pulsesAfterData = skipToNextSilence();
			if (pulsesAfterData > 0) {
				cout << WAVTIME(posIni) << MSG_WARNING << ": Skipping " << (pos-posIni) << " pulses after data block for "<< std::dec << (pulsesAfterData/(float)WAVSampleRate) << "sec" << endl;
			}
		}

		//Pause after data in ms
		posIni = pos;
		blockInfo.pausems = (WORD)(skipSilence() * 1000.0f / WAVSampleRate);
		if (blockInfo.pausems > 0) {
			cout << WAVTIME(posIni) << "Skip silence ("<< std::dec << blockInfo.pausems/1000.0f << "sec)" << endl;
		}

		//Constructing the Block
		if (blockInfo.pulses > 0 || buff.size() > 0) {
			block = new Block4B(
							blockInfo.pausems,
							blockInfo.pilot,
							blockInfo.pulses,
							blockInfo.bit0len,
							blockInfo.bit1len,
							blockInfo.bitcfg,
							blockInfo.bytecfg,
							(char*)buff.data(),
							buff.size());
			cout << WAVTIME(pos) << TXT_B_GREEN << "Adding #4B " << type << " Block" << TXT_RESET << endl;
			return true;
		}
	}

	//If no KCS block detected then return false
	return false;
}

DWORD MSX4B_Ripper::checkPilot(DWORD posIni)
{
	float pulseSum = states[posIni++];
	float pulses = 1.f;
	float pulseLen = 0.f;
	BYTE  startBitPulses = this->leadingValue ? this->pulses1 : this->pulses0;

	while (!eof(posIni)) {
		pulseLen = pulseSum / pulses;
		this->bauds = (DWORD)((float)WAVSampleRate / 4.f / pulseLen);
		if (states[posIni] > THRESHOLD_SILENCE) break;
		if (pulses>20 && checkBitN(posIni, this->leadingValue) && 
			(!predictiveMode || predictiveBitsForward(posIni+startBitPulses, -1, 0, true))) break;
		pulseSum += states[posIni++];
		pulses++;
	}
	//At least 500 pilot pulses
	if (pulseLen==0 || pulses < THRESHOLD_HEADER) return 0;
	if (!checkBitN(posIni, this->leadingValue)) return 0;

	return pulses;
}

DWORD MSX4B_Ripper::checkBitN(DWORD posIni, BYTE bit)
{
	DWORD total = 0;
	bool  check = true;
	BYTE  pulses = bit ? this->pulses1 : this->pulses0;
	float XXX_PULSE = bit ? ONE_PULSE : ZERO_PULSE;

	for (int i=0; i<pulses; i++) {
		total += states[posIni+i];
		if (i==0 || i==pulses-1)
			check &= ABS(states[posIni+i], XXX_PULSE) <= XXX_PULSE*WINDOW1ST*(bit?2:1);
		else
			check &= ABS(states[posIni+i], XXX_PULSE) <= XXX_PULSE*WINDOW;
	}
	if (check) return pulses;

	if (isMSX && !bit && ABS(states[posIni], ZERO_PULSE) <= ZERO_PULSE*WINDOW1ST &&
		ABS(states[posIni+1], ZERO_PULSE) <= ZERO_PULSE*WINDOW) return pulses;

	if (ABS(total, XXX_PULSE*pulses) <= WINDOW1ST*XXX_PULSE*pulses) return pulses;

	return 0;
}

DWORD MSX4B_Ripper::checkBit1(DWORD posIni)
{
	return checkBitN(posIni, 1);
}

DWORD MSX4B_Ripper::checkBit0(DWORD posIni)
{
	return checkBitN(posIni, 0);
}

/**
 * @return		Predictive solution, true for success and false for failure.
 */
DWORD MSX4B_Ripper::predictiveBitsForward(DWORD posIni, int8_t currentBit, bool bitChoice, bool useNextStartBit)
{
	cout << std::dec;

	if (currentBit >= 0) {
		posIni += bitChoice ? 4 : 2;
		if (verboseMode) cout << WAVTIME(posIni) << "  Prediction using value " << (bitChoice ? "1" : "0") << endl;
	}
	if (eof(posIni)) return 0;

	DWORD bitLen0, bitLen1;
	//End the byte data
	for (int i = currentBit + 1; i < 8; i++) {
		bitLen0 = checkBit0(posIni);
		bitLen1 = checkBit1(posIni);
		if ((bitLen0 && bitLen1) || (!bitLen0 && !bitLen1)) {
			return 0;
		} else
		if (bitLen0) {
			if (verboseMode) cout << WAVTIME(posIni) << "    Bit#" << (i+1) << ": 0 OK" << NEXTPULSES2(posIni) << endl;
			posIni += 2;
		} else
		if (bitLen1) {
			if (verboseMode) cout << WAVTIME(posIni) << "    Bit#" << (i+1) << ": 1 OK" << NEXTPULSES(posIni) << endl;
			posIni += 4;
		} else
			return 0;
		if (eof(posIni)) return 0;
	}
	//Check 1st stop bit
	if (eof(posIni+4)) return 0;
	if (!checkBit1(posIni)) return 0;
	if (verboseMode) cout << WAVTIME(posIni) << "    Stop bit#1 OK " << NEXTPULSES(posIni) << endl;
	posIni += 4;
	//Check 2nd stop bit
	if (eof(posIni+4)) return 0;
	if (!checkBit1(posIni)) return 0;
	if (verboseMode) cout << WAVTIME(posIni) << "    Stop bit#2 OK " << NEXTPULSES(posIni) << endl;
	posIni += 4;
	//Check next byte Start bit
	if (useNextStartBit) {
		if (eof(posIni+2)) return 0;
		if (!checkBit0(posIni)) return 0;
		if (verboseMode) cout << WAVTIME(posIni) << "    Start bit OK " << NEXTPULSES2(posIni) << endl;
		posIni += 2;
	}

	return posIni;
}

/**
 * @return		User choice -> 0:Bit 0 | 1:Bit 1 | 0xFF:Abort
 */
BYTE MSX4B_Ripper::askUserForBitValue(DWORD posIni) {
	cout << endl <<
		TXT_GREEN << "Bit 0     _______" << endl <<
		             "         |       " << endl <<
		             "  ''''''''         Next pulse lengths (samples/pulse):" << TXT_RESET << endl;
	printf("   %3u    %3u      ", (uint16_t)states[posIni], (uint16_t)states[posIni+1]);
	for (int i=2; i<16; i++) printf("%3u ", (uint16_t)states[posIni+i]);
	cout << endl << endl <<
		TXT_GREEN << "Bit 1 ___     ___" << endl <<
		             "     |   |   |   " << endl <<
		             "  ''''   '''''     Next pulse lengths (samples/pulse):" << TXT_RESET << endl;
	printf("  %3u %3u %3u %3u  ", (uint16_t)states[posIni], (uint16_t)states[posIni+1], (uint16_t)states[posIni+2], (uint16_t)states[posIni+3]);
	for (int i=4; i<18; i++) printf("%3u ", (uint16_t)states[posIni+i]);

	char ask;
	while (1) {
		cout << endl << endl <<
			TXT_B_WHITE << "Some Pulse lengths are out of tolerance limits (single pulse: "<< (WORD)ONE_PULSE << " samples, double: "<< (WORD)ZERO_PULSE << " samples)." << endl << 
			TXT_B_YELLOW << "Select correct bit value from above ([0]/[1]/[A]bort)? " << TXT_RESET;
		cin >> ask;
		if (ask == '0') return 0;
		if (ask == '1') return 1;
		if (ask == 'a' || ask == 'A') return 0xFF;
	}
}

/**
 * @return		Byte value or >= 0xFFF0 if error or end on block/file
 */
WORD MSX4B_Ripper::getByte()
{
	DWORD posIni = pos;
	DWORD bitLen0, bitLen1, bitLen;
	bool  predictive0, predictive1;
	WORD  value = 0;
	BYTE  mask = 0;

	if (verboseMode) cout << WAVTIME(posIni) << "-------------------------------" << endl;

	//Check for a PILOT without previous silence
	int len;
	while ((len=checkBit1(posIni)) && !checkBitN(posIni, this->leadingValue)) {
		posIni += len;
	}
	if (posIni-pos>100) {
		cout << WAVTIME(pos) << "Pilot detected. Ending block read..." << endl;
		return RET_PILOT;
	}
	posIni = pos;

	//Check START BITS
	for (int i=1; i<=this->leadingBits; i++) {
		if (!(bitLen=checkBitN(posIni, this->leadingValue)) || eof(posIni+bitLen)) {
			if (detectSilence(posIni) || detectSilence(posIni+1)) {
				cout << WAVTIME(posIni) << "Silence detected. Ending block read..." << endl;
				return RET_SILENCE;
			}
			
			predictive0 = false;
			if (predictiveMode) {
				if (!predictive0) {
					predictive0 = predictiveBitsForward(posIni+2, -1, 0, true);
				}
			}

			if (!predictive0) {
				cout << WAVTIME(posIni) << MSG_WARNING << ": Bad start bit#" << i << ": Assuming " << (int)(this->leadingValue) << " and continue..." << endl;
			}

			bitLen = (this->leadingValue ? this->pulses1 : this->pulses0);
		}
		if (verboseMode) cout << WAVTIME(posIni) << "  START BIT#" << i << ": " << (int)(this->leadingValue) << NEXTPULSES2(posIni) << endl;
		posIni += bitLen;
		if (eof(posIni)) { value=RET_END; goto END; }	//EOF
	}

	//Bit #0-7
	for (int i=0; i<8 && !eof(posIni+2); i++) {
		bitLen0 = checkBit0(posIni);
		bitLen1 = checkBit1(posIni);

		if ((bitLen0 && bitLen1) || (!bitLen0 && !bitLen1)) {
			//Detect silences in the 4 next pulses
			BYTE idx = 0;
			if (detectSilence(posIni) || detectSilence(posIni+(++idx)) || detectSilence(posIni+(++idx)) || detectSilence(posIni+(++idx))) {
				cout << WAVTIME(posIni) << "Silence detected. Ending block read..." << NEXTPULSES(posIni) << endl;
				pos = posIni + idx;
				return RET_SILENCE;
			}

			cout << WAVTIME(posIni) << MSG_WARNING << ": Bad/Ambiguous pulse length in BIT #" << (i+1) << NEXTPULSES(posIni) << endl;
			BYTE ask = 0xFF;
			//Predictive Bits Forward
			predictive0 = predictive1 = false;
			if (predictiveMode) {
				predictive0 = predictiveBitsForward(posIni, i, 0, false);
				predictive1 = predictiveBitsForward(posIni, i, 1, false);
				if (predictive0 && predictive1) {
					//If ambiguity persists try to check the StartBit too...
					if (verboseMode) cout << WAVTIME(posIni) << "  Inconclusive prediction. Using next Start Bit too..." << endl;
					predictive0 = predictiveBitsForward(posIni, i, 0, true);
					predictive1 = predictiveBitsForward(posIni, i, 1, true);
				}
				if (predictive0 == predictive1) {
					predictive0 = predictiveBitsForward(posIni-1, i, 0, true);
					predictive1 = predictiveBitsForward(posIni-1, i, 1, true);
					if (predictive0 != predictive1) posIni--;
				}
				if (predictive0 == predictive1) {
					predictive0 = predictiveBitsForward(posIni+1, i, 0, true);
					predictive1 = predictiveBitsForward(posIni+1, i, 1, true);
					if (predictive0 != predictive1) posIni++;
				}
			}

			if (predictive0 == predictive1) {
				//Ask user for action to do (choice bit 1, bit 0, or Abort)
				if (interactiveMode) ask = askUserForBitValue(posIni);
			} else {
				if (predictive0) ask = 0;
				if (predictive1) ask = 1;
				cout << WAVTIME(posIni) << MSG_SUCCESS << ": Predictive Bits Forward: Solved using BIT #" << (i+1) << " like a " << (ask ? "1" : "0") << endl;
			}

			//Set predictive/interactive result to the Bit
			if (ask == 0) {
				if (verboseMode) cout << WAVTIME(posIni) << "  BIT #" << (i+1) << ": 0" << NEXTPULSES2(posIni) << endl;
				posIni += this->pulses0;
			} else
			if (ask == 1) {
				if (verboseMode) cout << WAVTIME(posIni) << "  BIT #" << (i+1) << ": 1" << NEXTPULSES(posIni) << endl;
				posIni += this->pulses1;
				value |= bit2set(mask);
			} else {
				cout << WAVTIME(posIni) << "Ending block read..." << endl;
				return RET_ABORT;
			}
		} else
		if (bitLen0) {
			if (verboseMode) cout << WAVTIME(posIni) << "  BIT #" << (i+1) << ": 0" << NEXTPULSES2(posIni) << endl;
			posIni += bitLen0;
		} else
		if (bitLen1) {
			if (verboseMode) cout << WAVTIME(posIni) << "  BIT #" << (i+1) << ": 1" << NEXTPULSES(posIni) << endl;
			posIni += bitLen1;
			value |= bit2set(mask);
		}
		mask++;
	}
	if (eof(posIni)) { value=RET_END; goto END; }	//EOF

	//Check STOP BITS
	for (int i=1; i<=this->trailingBits; i++) {
		if (!(bitLen=checkBitN(posIni, this->trailingValue))) {
			for (int b=0; b<this->trailingBits; b++) {
				if (detectSilence(posIni+b)) {
					cout << WAVTIME(posIni) << MSG_WARNING << ": Bad stop bit#" << i << ": Silence detected. Ending block read..." << NEXTPULSES(posIni) << endl;
					pos = posIni + b;
					return RET_SILENCE;
				}
			}
			cout << WAVTIME(posIni) << MSG_WARNING << ": Bad stop bit #" << i << " Assuming value " << (int)(this->trailingValue) << " and continue..." << NEXTPULSES(posIni) << endl;
			bitLen = (this->trailingValue ? this->pulses1 : this->pulses0);
		}
		if (verboseMode) cout << WAVTIME(posIni) << "  STOP BIT #1: " << (int)(this->trailingValue) << NEXTPULSES(posIni) << endl;
		posIni += bitLen;
		if (eof(posIni)) { value=RET_END; goto END; }	//EOF
	}

END:
	pos = posIni;
	return value;
}
