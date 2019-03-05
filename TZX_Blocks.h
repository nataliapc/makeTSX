#ifndef __TZX_BLOCKS_H__
#define __TZX_BLOCKS_H__

#include "types.h"
#include "ByteBuffer.h"


using namespace std;
using namespace Utility;

namespace TZX_Blocks
{
	// ============================================================================================
	// Class Block

	class Block
	{
	public:
		Block();
		Block(const Block& other);
		~Block();

		size_t	getSize();
		BYTE	getId();
		BYTE	getHeadSize();
		const char* getBytes();
		void	hexDump();
		void	hexCharDump();
		virtual string toString();
		
		Block& operator=(const Block &);

	protected:
		void	init(BYTE, BYTE, char*, size_t);
		void	putByte(BYTE);
		void	putWord(WORD);
		void	putWord24(WORD24);
		void	putDWord(DWORD);
		void	putString(string);
		void	put(char*, size_t);
		void	put(istream &, size_t);
		void	put(ByteBuffer*);

	protected:
		ByteBuffer	*bytes;
	private:
		BYTE	id;
		BYTE	headSize;	//Header size before block data (if any)
	};

	// ============================================================================================
	// Block #10 - Standard speed data block
	//	0x00	-	WORD 		Pause after this block (ms.) {1000}
	//	0x02	N	WORD 		Length of data that follow
	//	0x04	-	BYTE[N]		Data as in .TAP files
	class Block10 : public Block
	{
	public:
		Block10(WORD pause, char* data, size_t size);
		Block10(istream &);
		void   setPause(WORD pause);
		void   addPause(WORD);
		string toString() override;
	};

	// ============================================================================================
	// Block #11 - Turbo speed data block
	//	0x00	-	WORD		Length of PILOT pulse {2168}
	//	0x02	-	WORD		Length of SYNC first pulse {667}
	//	0x04	-	WORD		Length of SYNC second pulse {735}
	//	0x06	-	WORD		Length of ZERO bit pulse {855}
	//	0x08	-	WORD		Length of ONE bit pulse {1710}
	//	0x0A	-	WORD		Length of PILOT tone (number of pulses) {8063 header (flag<128), 3223 data (flag>=128)}
	//	0x0C	-	BYTE		Used bits in the last byte (other bits should be 0) {8}
	//							(e.g. if this is 6, then the bits used (x) in the last byte are: xxxxxx00, where MSb is the leftmost bit, LSb is the rightmost bit)
	//	0x0D	-	WORD		Pause after this block (ms.) {1000}
	//	0x0F	N	BYTE[3]		Length of data that follow
	//	0x12	-	BYTE[N]		Data as in .TAP files
	class Block11 : public Block
	{
	public:
		Block11(WORD pilotlen, WORD synclen1, WORD synclen2, WORD bit0len, WORD bit1len, WORD pilotnum, BYTE rbits, WORD pause, char *data, size_t size);
		Block11(istream &);
		void   setPause(WORD pause);
		void   addPause(WORD);
		string toString() override;
	};

	// ============================================================================================
	// Block #12 - Pure tone
	//	0x00	-	WORD		Length of one pulse in T-states
	//	0x02	-	WORD		Number of pulses
	class Block12 : public Block
	{
	public:
		Block12(WORD pulselen, WORD pulsenum);
		Block12(istream &);
		string toString() override;
	};


	// ============================================================================================
	// Block #13 - Sequence of pulses of various lengths
	//	0x00	N	BYTE		Number of pulses
	//	0x01	-	WORD[N]		Pulses' lengths
	class Block13 : public Block
	{
	public:
		Block13(BYTE pulsenum, WORD *lengths);
		Block13(istream &);
		string toString() override;
	};


	// ============================================================================================
	// Block #14 - Pure data block
	//	0x00	-	WORD		Length of ZERO bit pulse
	//	0x02	-	WORD		Length of ONE bit pulse
	//	0x04	-	BYTE		Used bits in last byte (other bits should be 0)
	//							(e.g. if this is 6, then the bits used (x) in the last byte are: xxxxxx00, where MSb is the leftmost bit, LSb is the rightmost bit)
	//	0x05	-	WORD		Pause after this block (ms.)
	//	0x07	N	BYTE[3] 	Length of data that follow
	//	0x0A	-	BYTE[N]		Data as in .TAP files
	class Block14 : public Block
	{
	public:
		Block14(WORD bit0len, WORD bit1len, BYTE rbits, WORD pause, char *data, size_t size);
		Block14(istream &);
		void   setPause(WORD pause);
		void   addPause(WORD);
		string toString() override;
	};

	// ============================================================================================
	// Block #15 - Direct recording block
	//	0x00	-	WORD 		Number of T-states per sample (bit of data)
	//	0x02	-	WORD 		Pause after this block in milliseconds (ms.)
	//	0x04	-	BYTE 		Used bits (samples) in last byte of data (1-8)
	//							(e.g. if this is 2, only first two samples of the last byte will be played)
	//	0x05	N	BYTE[3]		Length of samples' data
	//	0x08	-	BYTE[N]		Samples data. Each bit represents a state on the EAR port (i.e. one sample).
	//							MSb is played first.
	class Block15 : public Block
	{
	public:
		Block15(WORD numstates, WORD pause, BYTE rbits, char *data, size_t size);
		Block15(istream &);
		string toString() override;
	};

	// ============================================================================================
	// Block #20 - Pause (silence) or 'Stop the tape' command
	//	0x00	-	WORD		Pause duration (ms.)
	class Block20 : public Block
	{
	public:
		Block20(WORD pause);
		Block20(istream &);
		WORD getPause();
		void setPause(WORD pause);
		void addPause(WORD pause);
		string toString() override;
	};



	// ============================================================================================
	// Block #21 - Group start
	//	0x00	L	BYTE		Length of the group name string
	//	0x01	-	CHAR[L]		Group name in ASCII format (please keep it under 30 characters long)
	class Block21 : public Block
	{
	public:
		Block21(string nameGroup);
		Block21(istream &);
		string toString() override;
	};

	// ============================================================================================
	// Block #22 - Group end
	class Block22 : public Block
	{
	public:
		Block22();
		Block22(istream &);
		string toString() override;
	};

	// ============================================================================================
	// Block #23 - Jump to block
	//	0x00	-	WORD		Relative jump value
	class Block23 : public Block
	{
	public:
		Block23(WORD jump);
		Block23(istream &);
		string toString() override;
	};

	// ============================================================================================
	// Block #24 - Loop start
	//	0x00	-	WORD		Number of repetitions (greater than 1)
	class Block24 : public Block
	{
	public:
		Block24(WORD numRepeats);
		Block24(istream &);
		string toString() override;
	};

	// ============================================================================================
	// Block #25 - Loop end
	class Block25 : public Block
	{
	public:
		Block25();
		Block25(istream &);
		string toString() override;
	};

	// ============================================================================================
	// Block #26 - Call sequence
	//	0x00	N	WORD		Number of calls to be made
	//	0x02	-	WORD[N]		Array of call block numbers (relative-signed offsets)
	class Block26 : public Block
	{
	public:
		Block26(WORD num, WORD *calls);
		Block26(istream &);
		string toString() override;
	};

	// ============================================================================================
    //Block #27 - Return from sequence
	class Block27 : public Block
	{
	public:
		Block27();
		Block27(istream &);
		string toString() override;
	};

	// ============================================================================================
	// Block #28 - Select block
	//	0x00	-	WORD		Length of the whole block (without these two bytes)
	//	0x02	N	BYTE		Number of selections
	//	0x03	-	SELECT[N]	List of selections
	//
	//	SELECT structure format
	//	0x00	-	WORD		Relative Offset
	//	0x02	L	BYTE		Length of description text
	//	0x03	-	CHAR[L]		Description text (please use single line and max. 30 chars)
	class Block28 : public Block
	{
	public:
		Block28(BYTE num, WORD *offsets, string *texts);
		Block28(istream &);
		string toString() override;
	};

	// ============================================================================================
	// Block #2A - Stop the tape if in 48K mode
	//	0x00	0	DWORD		Length of the block without these four bytes (0)
	class Block2A : public Block
	{
	public:
		Block2A();
		Block2A(istream &);
		string toString() override;
	};

	// ============================================================================================
	// Block #30 - Text description
	//	0x00	N	BYTE		Length of the text description
	//	0x01	-	CHAR[N]		Text description in ASCII format
	class Block30 : public Block
	{
	public:
		Block30(string text);
		Block30(istream &);
		string toString() override;
	};

	// ============================================================================================
	// Block #31 - Message block
	//	0x00	-	BYTE		Time (in seconds) for which the message should be displayed
	//	0x01	N	BYTE		Length of the text message
	//	0x02	-	CHAR[N]		Message that should be displayed in ASCII format
	class Block31 : public Block
	{
	public:
		Block31(BYTE seconds, string text);
		Block31(istream &);
		string toString() override;
	};

	// ============================================================================================
	// Block #32 - Archive info
	//	0x00	-	WORD		Length of the whole block (without these two bytes)
	//	0x02	N	BYTE		Number of text strings
	//	0x03	-	TEXT[N]		List of text strings
	//
	//	TEXT structure format
	//	0x00	-	BYTE		Text identification byte:
	//				00 - Full title
	//				01 - Software house/publisher
	//				02 - Author(s)
	//				03 - Year of publication
	//				04 - Language
	//				05 - Game/utility type
	//				06 - Price
	//				07 - Protection scheme/loader
	//				08 - Origin
	//				FF - Comment(s)
	//	0x01	L	BYTE	Length of text string
	//	0x02	-	CHAR[L]	Text string in ASCII format
	class Block32 : public Block
	{
	public:
		Block32(BYTE num, BYTE *ids, string *texts);
		Block32(istream &);
		string toString() override;
	};

	// ============================================================================================
	// Block #33 - Hardware type
	//	0x00	N	BYTE		Number of machines and hardware types for which info is supplied
	//	0x01	-	HWINFO[N]	List of machines and hardware
	//
	//	HWINFO structure format
	//	0x00	-	BYTE		Hardware type
	//	0x01	-	BYTE		Hardware ID
	//	0x02	-	BYTE		Hardware information:
	//				00 - The tape RUNS on this machine or with this hardware, but may or may not use the hardware or special features of the machine.
	//				01 - The tape USES the hardware or special features of the machine, such as extra memory or a sound chip.
	//				02 - The tape RUNS but it DOESN'T use the hardware or special features of the machine.
	//				03 - The tape DOESN'T RUN on this machine or with this hardware.
	class Block33 : public Block
	{
	public:
		Block33(BYTE num, BYTE *hwtype, BYTE *hwid, BYTE *hwinfo);
		Block33(istream &);
		string toString() override;
	};

	// ============================================================================================
	// ID 35 - Custom info block
	//	0x00	-	CHAR[10]	Identification string (in ASCII)
	//	0x10	L	DWORD		Length of the custom info
	//	0x14	-	BYTE[L]		Custom info
	class Block35 : public Block
	{
	public:
		Block35(string label, string info);
		Block35(istream &);
		string toString() override;
	};

	// ============================================================================================
	// Block #4B - Kansas City Standard
	//	0x00   N+12	DWORD		Block length without these four bytes (extension rule)
	//	0x04	-	WORD		Pause after this block in milliseconds
	//	0x06	-	WORD		Duration of a PILOT pulse in T-states {same as ONE pulse}
	//	0x08	-	WORD		Number of pulses in the PILOT tone
	//	0x0A	-	WORD		Duration of a ZERO pulse in T-states
	//	0x0C	-	WORD		Duration of a ONE pulse in T-states
	//	0x0E	-	BYTE		Bits 7-4: Number of ZERO pulses in a ZERO bit (0-16) {2}
	//			 (bitmapped)	Bits 3-0: Number of ONE pulses in a ONE bit (0-16) {4}
	//	0x0F	-	BYTE		Bits 7-6: Numbers of leading bits {1}
	//			 (bitmapped)	Bit 5: Value of leading bits {0}
	//							Bits 4-3: Number of trailing bits {2}
	//							Bit 2: Value of trailing bits {1}
	//							Bit 1: Reserved
	//							Bit 0: Endianless (0 for LSb first, 1 for MSb first) {0}
	//	0x10	-	BYTE[N]		Data stream
	class Block4B : public Block
	{
	public:
		Block4B(WORD pause, WORD pilot, WORD pulses, WORD bit0len, WORD bit1len, BYTE bitcfg, BYTE bytecfg, char *data, size_t size);
		Block4B(istream &);
		BYTE   getFileType();
		string getFileTypeLoad();
		string getFileTypeDescription();
		void   setPause(WORD pause);
		void   addPause(WORD);
		string toString() override;
		string toString(bool);
	};

	// ============================================================================================
	// Block #5A - "Glue" block (90 dec, ASCII Letter 'Z')
	//	0x00	-	BYTE[9]		Value: { "XTape!",0x1A,MajR,MinR }
	//							Just skip these 9 bytes and you will end up on the next ID.
	class Block5A : public Block
	{
	public:
		Block5A();
		Block5A(istream &);
		string toString() override;
	};

	//=================================
	//NOT IMPLEMENTED BLOCKS
	//=================================

	// ============================================================================================
	// Block #19 - Generalized data block
	//	0x00 	 -		DWORD		Block length (without these four bytes)
	//	0x04 	 -		WORD 		Pause after this block (ms)
	//	0x06 	 TOTP	DWORD		Total number of symbols in pilot/sync block (can be 0)
	//	0x0A 	 NPP	BYTE		Maximum number of pulses per pilot/sync symbol
	//	0x0B 	 ASP	BYTE		Number of pilot/sync symbols in the alphabet table (0=256)
	//	0x0C 	 TOTD	DWORD		Total number of symbols in data stream (can be 0)
	//	0x10 	 NPD	BYTE		Maximum number of pulses per data symbol
	//	0x11 	 ASD	BYTE		Number of data symbols in the alphabet table (0=256)
	//	0x12 	 -	 SYMDEF[ASP]	Pilot and sync symbols definition table
	//								This field is present only if TOTP>0
	//	0x12+  -	 PRLE[TOTP]		Pilot and sync data stream
	//	 (2*NPP+1)*ASP				This field is present only if TOTP>0
	//	0x12+  - 	 SYMDEF[ASD] 	Data symbols definition table
	//	 (TOTP>0)*((2*NPP+1)*ASP)+	This field is present only if TOTD>0
	//	 TOTP*3
	//	0x12+  - 	  BYTE[DS]		Data stream
	//	 (TOTP>0)*((2*NPP+1)*ASP)+	This field is present only if TOTD>0
	//	 TOTP*3+
	//	 (2*NPD+1)*ASD
	class Block19 : public Block
	{
	public:
		Block19();
		Block19(istream &);
		string toString() override;
	};

	// ============================================================================================
	// Block #18 - CSW recording block
	class Block18 : public Block
	{
	public:
		string toString() override;
	};

	// ============================================================================================
	// ID 2B - Set signal level
	class Block2B : public Block
	{
	public:
		string toString() override;
	};


	//=================================
	// DEPRECATED BLOCKS
	//=================================
	// Block #16 - C64 ROM type data block
	class Block16 : public Block {
	public:
		Block16();
		Block16(istream &);
		string toString() override;
	};
	// Block #17 - C64 turbo tape data block
	class Block17 : public Block {
	public:
		Block17();
		Block17(istream &);
		string toString() override;
	};
	// Block #34 - Emulation info
	class Block34 : public Block {
	public:
		Block34();
		Block34(istream &);
		string toString() override;
	};
	// Block #40 - Snapshot block
	class Block40 : public Block {
	public:
		Block40();
		Block40(istream &);
		string toString() override;
	};

}

#endif //__TZX_BLOCKS_H__
