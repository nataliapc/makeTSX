
#include <cmath>
#include <vector>
#include <cstring>

#include "B10_Standard_Ripper.h"


using namespace std;
using namespace WAV_Class;
using namespace Rippers;

#define TOLERANCE				0.15f
#define bytesPerPulse(tstates)	((float)tstates*WAVSampleRate/Z80HZ)	// Bytes per pulse for T-states parameter on a WAV frequency
#define bytesPerBit(tstates)	(bytesPerPulse(bauds)*2)				// Wav Bytes for every Data bit for T-states parameter

/*
 * PILOT Pulse:		2168 Tstates (27.316 bytes @8bits:44100Hz) 8063/3223 pulses
 * SYNC#1 Pulse:	 667 Tstates ( 8.404 bytes @8bits:44100Hz)
 * SYNC#2 Pulse:	 735 Tstates ( 9.261 bytes @8bits:44100Hz)
 * ZERO bit Pulse:	 855 Tstates (10.773 bytes @8bits:44100Hz)
 * ONE bit Pulse:	1710 Tstates (21.546 bytes @8bits:44100Hz)
 */
#define PILOT_PULSE				(bytesPerPulse(2168*modif))
#define SYNC1_PULSE				(bytesPerPulse(667*modif))
#define SYNC2_PULSE				(bytesPerPulse(735*modif))
#define ZERO_PULSE				(bytesPerPulse(855*modif))
#define ONE_PULSE				(bytesPerPulse(1710*modif))

#define SILENCE_PULSE			(WAVSampleRate/1000)

#define NEXTPULSES(pos)			" [" << states[pos] << "]"
#define NEXTPULSES2(pos)		" [" << states[pos] << " " << states[pos+1] << "]"
#define NEXTPULSES4(pos)		" [" << states[pos] << " " << states[pos+1] << " " << states[pos+2] << " " << states[pos+3] << "]"



B10_Standard_Ripper::B10_Standard_Ripper(WAV *wav) : BlockRipper(wav)
{
}

B10_Standard_Ripper::B10_Standard_Ripper(const B10_Standard_Ripper& other) : BlockRipper(other)
{
}

B10_Standard_Ripper::~B10_Standard_Ripper()
{
}

bool B10_Standard_Ripper::detectSilence(DWORD pos)
{
	return (!eof(pos) && states[pos] >= SILENCE_PULSE);
}

bool B10_Standard_Ripper::detectBlock()
{
	block = NULL;
	DWORD posIni = pos;
	WORD  pausems = 0;

	//Detect block#10

	//Search Pilot
	DWORD pilots = checkPilot(posIni);
	if (pilots && !eof(pos+pilots)) {

		cout << WAVTIME(posIni) << TXT_B_GREEN << "Detected #10 Standard Speed Pilot tone (" << std::dec << pilots << " pulses|speed:x" << modif << ")" << NEXTPULSES4(posIni) << TXT_RESET << endl;
		posIni += pilots;

		//Check Sync#1
		if (!eof(posIni) && ABS(states[posIni], SYNC1_PULSE) > SYNC1_PULSE*0.20f) {

			//If correct pilot pulses we use a big tolerance for sync pulses
			float customTolerance = (pilots==3223 || pilots==8063) ? 0.30f : 0.15f;

			//If SYNC1 fails try to check if SYNC1+SYNC2 is timed correctly
			if (ABS(states[posIni]+states[posIni+1], SYNC1_PULSE+SYNC2_PULSE) > (SYNC1_PULSE+SYNC2_PULSE)*customTolerance) {
				bool ask = false;
				if (interactiveMode) {
					cout << WAVTIME(posIni) << MSG_WARNING << ": Bad SYNC#1 Pulse " << NEXTPULSES2(posIni) << endl;
					ask = askUserForSyncBits(posIni);
				}
				if (!ask) {
					//If fails return that is not a block #10
					cout << WAVTIME(posIni) << MSG_ERROR << ": Bad SYNC#1 Pulse!" << NEXTPULSES2(posIni) << endl;
					return false;
				}
			}
			//SYNC1+SYNC2 is correct, skip SYNC bits
			cout << WAVTIME(posIni) << MSG_WARNING << ": Corrected SYNC PULSES (sum are OK)" << NEXTPULSES2(posIni) << endl;
			posIni += 2;

		} else {
			if (verboseMode) cout << WAVTIME(posIni) << "  SYNC#1 PULSE OK" << NEXTPULSES(posIni) << endl;
			posIni++;

			//Check Sync#2
			if (!eof(posIni) && ABS(states[posIni], SYNC2_PULSE) > SYNC2_PULSE*0.20f) {
				cout << WAVTIME(posIni) << MSG_WARNING << ": Bad SYNC#2 Pulse: Assuming is OK and continue" << NEXTPULSES2(posIni) << endl;
			}

			if (verboseMode) cout << WAVTIME(posIni) << "  SYNC#2 PULSE OK" << NEXTPULSES(posIni) << endl;
			posIni++;
		}

		pos = posIni;

		//Get Data bytes
		WORD value = 0, lastByte = 0;
		WORD chksum = 0, lastChksum = 0;
		vector<BYTE> buff;
		while (!eof()) {
			value = getByte();
			if (value==0xFFFF) {	//Found silence or EOF
				break;
			}
			lastByte = value;
			buff.push_back(value);
			if (lastByte==chksum && checkPilot(pos)) { //Blocks without silence pause?
				break;
			}
			if (buff.size()>0) {
				lastChksum = chksum;
				chksum ^= (lastByte & 0xFF);
			}
			if (verboseMode) cout << WAVTIME(pos) << std::hex << TXT_B_WHITE << "Pos:[0x" << buff.size() << "] Detected BYTE #" << std::hex << value << " (" << std::dec << value << ")" << TXT_RESET << endl;
		}
		chksum = lastChksum;
		cout << WAVTIME(pos) << "Extracted data: " << std::dec << (buff.size()) << " bytes" << endl;
		if (chksum==lastByte) {
			cout << WAVTIME(pos) << MSG_SUCCESS << ": CHECKSUM OK [0x"<< std::hex << chksum << "]" << endl;
		} else {
			cout << WAVTIME(pos) << MSG_ERROR << ": CHECKSUM ERR [0x"<< std::hex << chksum << " but expected 0x" << lastByte << "]" << endl;
		}

/*		//Pulses after data block?
		posIni = pos;
		WORD pulsesAfterData = skipToNextSilence();
		if (pulsesAfterData > 0) {
			cout << WAVTIME(posIni) << WARNING ": Skipping " << (pos-posIni) << " pulses after data block for "<< std::dec << (pulsesAfterData/(float)WAVSampleRate) << "sec" << endl;
		}
*/		//Pause after data in ms
		posIni = pos;
		pausems = (WORD)(skipSilence() * 1000.0f / WAVSampleRate);
		if (pausems > 0) {
			cout << WAVTIME(posIni) << "Skip silence ("<< std::dec << pausems/1000.0f << "sec)" << endl;
		}

		//Constructing the Block
		if (pilots > 0 && buff.size() > 0) {
			block = new Block10(
							pausems,
							(char*)buff.data(),
							buff.size());
			cout << WAVTIME(pos) << TXT_B_GREEN << "Adding #10 Standard Speed Data Block" << TXT_RESET << endl;
			return true;
		}
	}

	return false;
}

DWORD B10_Standard_Ripper::checkPilot(DWORD posIni)
{
	float pulseSum = states[posIni] + states[posIni+1];
	float pulses = 2.f;
	float pulseLen = 0.f;
	modif = 1.0f;
	posIni += 2;
	while (!eof(posIni)) {
		pulseLen = pulseSum / pulses;
		if (pulses>4.f && ABS(states[posIni-2]+states[posIni-1]+states[posIni], pulseLen*3) > pulseLen*0.5) break;
		if (pulses>2.f && states[posIni] <= PILOT_PULSE*0.55f) break;
		pulseSum += states[posIni++];
		pulses++;
	}
	//At least 100 pilot pulses
	if (pulseLen==0 || pulses < 100) return 0;
	if (ABS(pulseLen, PILOT_PULSE) > PILOT_PULSE*0.1f) return 0;
	modif = pulseLen / PILOT_PULSE;
	return pulses;
}

bool B10_Standard_Ripper::checkBit1(DWORD posIni)
{
	if (ABS(states[posIni], ONE_PULSE) <= ONE_PULSE*TOLERANCE) return true;
	if (ABS(states[posIni+1], ONE_PULSE) <= ONE_PULSE*TOLERANCE) return true;

	return false;
}

bool B10_Standard_Ripper::checkBit0(DWORD posIni)
{
	if (ABS(states[posIni], ZERO_PULSE) <= ZERO_PULSE*TOLERANCE) return true;
	if (ABS(states[posIni+1], ZERO_PULSE) <= ZERO_PULSE*TOLERANCE) return true;

	return false;
}

/**
 * @return		User choice -> true:accept sync values | false:abort
 */
bool B10_Standard_Ripper::askUserForSyncBits(DWORD posIni) {
	cout << endl <<
		TXT_GREEN << "   Sync#1  Sync#2  " << endl <<
		             "          _________" << endl <<
		             "         |         " << endl <<
		             "  ''''''''         " << TXT_RESET << endl;
	printf("    %3u      %3u", (uint16_t)states[posIni], (uint16_t)states[posIni+1]);

	char ask;
	while (1) {
		cout << endl << endl <<
			TXT_B_WHITE << "Sync pulse lengths are out of tolerance limits (values must be Sync#1=8 & Sync#2=9 samples)" << endl <<
			TXT_B_YELLOW << "Select action (Sync bits are [O]K/[A]bort)? " << TXT_RESET;
		cin >> ask;
		if (ask == 'o' || ask == 'O') return true;
		if (ask == 'a' || ask == 'A') return false;
	}
}

/**
 * @return		User choice -> 0:Bit 0 | 1:Bit 1 | 0xFF:Abort
 */
BYTE B10_Standard_Ripper::askUserForBitValue(DWORD posIni) {
	cout << endl <<
		TXT_GREEN << "Bit 0  ___" << endl <<
		             "      |   " << endl <<
		             "  '''''          Next pulse lengths (samples/pulse):" << TXT_RESET << endl;
	printf("  %3u %3u        ", (uint16_t)states[posIni], (uint16_t)states[posIni+1]);
	for (int i=2; i<16; i++) printf("%3u ", (uint16_t)states[posIni+i]);
	cout << endl << endl <<
		TXT_GREEN << "Bit 1    ______" << endl <<
		             "        |      " << endl <<
		             "  '''''''        Next pulse lengths (samples/pulse):" << TXT_RESET << endl;
	printf("   %3u    %3u    ", (uint16_t)states[posIni], (uint16_t)states[posIni+1]);
	for (int i=2; i<16; i++) printf("%3u ", (uint16_t)states[posIni+i]);

	char ask;
	while (1) {
		cout << endl << endl <<
			TXT_B_WHITE << "Some Pulse lengths are out of tolerance limits (0 pulses: "<< (WORD)ZERO_PULSE << " samples, 1 pulses: "<< (WORD)ONE_PULSE << " samples)." << endl << 
			TXT_B_YELLOW << "Select correct bit value from above ([0]/[1]/[A]bort)? " << TXT_RESET;
		cin >> ask;
		if (ask == '0') return 0;
		if (ask == '1') return 1;
		if (ask == 'a' || ask == 'A') return 0xFF;
	}
}

/**
 * @return		Byte value or 0xFFFF if error or end on block/file
 */
WORD B10_Standard_Ripper::getByte()
{
	DWORD posIni = pos;
	WORD  value = 0;
	BYTE  mask = 128;
	bool bit0, bit1;

	if (verboseMode) cout << WAVTIME(posIni) << "-------------------------------" << endl;

	if (detectSilence(posIni) || detectSilence(posIni+1)) {
		cout << WAVTIME(posIni) << "Silence detected. Ending block read..." << endl;
		return 0xFFFF;
	}

	//Bit #0-7
	for (int i=0; i<8 && !eof(posIni); i++) {
		bit0 = checkBit0(posIni);
		bit1 = checkBit1(posIni);

		//Inconclusive or No bits found: Interactive mode
		if ((!bit0 && !bit1) || (bit0 && bit1)) {
			//Detect silences in the 4 next pulses
			BYTE idx = 0;
			if (detectSilence(posIni) || detectSilence(posIni+(++idx))) {
				cout << WAVTIME(posIni) << MSG_WARNING << ": Silence detected. Ending block read..." << NEXTPULSES(posIni) << endl;
				pos = posIni + idx;
				return 0xFFFF;
			}

			cout << WAVTIME(posIni) << MSG_WARNING << ": Bad/Ambiguous pulse length in BIT #" << (8-i) << NEXTPULSES2(posIni) << endl;

			BYTE ask = 0xFF;
			//Predicting using cycle sum
			if (ask == 0xFF && predictiveMode) {
				WORD total = states[posIni] + states[posIni+1];
				float diff0 = ABS(total, ZERO_PULSE*2.f);
				float diff1 = ABS(total, ONE_PULSE*2.f);
				if (diff0 < diff1) {
					//It's a Bit 0
					cout << WAVTIME(posIni) << MSG_SUCCESS << ": cycle sum closest to BIT #" << (8-i) << " like a 0" << endl;
					ask = 0;
				}
				if (diff0 > diff1) {
					//It's a Bit 1
					cout << WAVTIME(posIni) << MSG_SUCCESS << ": cycle sum closest to BIT #" << (8-i) << " like a 1" << endl;
					ask = 1;
				}
			}

			//Predicting is las bit in block before a silence
			if (ask == 0xFF && predictiveMode) {
				if (detectSilence(posIni+2) && bit0) {
					//It's a Bit 0
					cout << WAVTIME(posIni) << MSG_SUCCESS << ": last bit in this block and seems a 0" << NEXTPULSES4(posIni) << endl;
					ask = 0;
				}
			}

			//Ask user for action to do (choice bit 1, bit 0, or Abort)
			if (ask == 0xFF && interactiveMode) {
				ask = askUserForBitValue(posIni);
			}

			//Set predictive/interactive result to the Bit
			if (ask == 0) {
				bit0 = true;
				bit1 = false;
			} else
			if (ask == 1) {
				bit0 = false;
				bit1 = true;
			} else {
				cout << WAVTIME(posIni) << "Ending block read..." << endl;
				return 0xFFFF;
			}
		}
		//Bit 0
		if (bit0) {
			if (verboseMode) cout << WAVTIME(posIni) << "  BIT #" << (8-i) << ": 0" << NEXTPULSES2(posIni) << endl;
			posIni += 2;
		} else
		//Bit 1
		if (bit1) {
			if (verboseMode) cout << WAVTIME(posIni) << "  BIT #" << (8-i) << ": 1" << NEXTPULSES2(posIni) << endl;
			posIni += 2;
			value |= mask;
		} else
		//Unexpected error. Aborting
		{
			cout << WAVTIME(posIni) << MSG_ERROR << ": Unexpected error! Aborting read..." << endl;
			goto END;
		}
		mask >>= 1;
	}

END:
	pos = posIni;
	return value;
}
