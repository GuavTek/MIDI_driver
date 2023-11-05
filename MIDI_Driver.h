/*
 * MIDI_Driver.h
 *
 * Created: 26/06/2021 14:39:50
 *  Author: GuavTek
 */ 


#ifndef MIDI_DRIVER_H_
#define MIDI_DRIVER_H_

#include <stdint.h>

enum class MIDI_MT_E {
	Utility		= 0x0,
	RealTime	= 0x1,
	Voice1		= 0x2,
	Data64		= 0x3,
	Voice2		= 0x4,
	Data128		= 0x5,
	Reserved6	= 0x6,
	Reserved7	= 0x7,
	Reserved8	= 0x8,
	Reserved9	= 0x9,
	Reserved10	= 0xa,
	Reserved11	= 0xb,
	Reserved12	= 0xc,
	FlexData	= 0xd,
	Reserved14	= 0xe,
	Stream		= 0xf,
};

enum class MIDI_ATTR_E {
	NoData			= 0x0,
	ManuSpecific	= 0x1,		// Manufacturer specific
	ProfSpecific	= 0x2,		// Profile specific
	Pitch			= 0x3		// For alternative pitches
};

enum class MIDI1_STATUS_E {
	NoteOff			= 0b1000,
	NoteOn			= 0b1001,
	Aftertouch		= 0b1010,
	CControl		= 0b1011,
	ProgChange		= 0b1100,
	ChanPressure	= 0b1101,
	Pitchbend		= 0b1110,
	SysEx			= 0b1111,
	Invalid			= 0b0000
};

enum class MIDI2_UTIL_E { 
	NOOP = 0x0, 
	JRClk = 0x1, 
	JRTimestamp = 0x2 
};

enum class MIDI2_COM_E {
	TimeCode		= 0x1,
	SongPosPoint	= 0x2,
	SongSel			= 0x3,
	TuneRequest		= 0x6,
	TimingClock		= 0x8,
	Start			= 0xA,
	Continue		= 0xB,
	Stop			= 0xC,
	ActiveSensing	= 0xE,
	Reset			= 0xF
};

enum class MIDI2_DATA64_E {
	Single		= 0x0,
	Start		= 0x1,
	Continue	= 0x2,
	End			= 0x3
};

enum class MIDI2_DATA128_E {
	Single		= 0x0,
	Start		= 0x1,
	Continue	= 0x2,
	End			= 0x3,
	MixHead		= 0x8,
	MixPay		= 0x9
};

enum class MIDI2_VOICE_E {
	RegNoteControl	= 0b0000,
	AssNoteControl	= 0b0001,
	RegControl		= 0b0010,
	AssControl		= 0b0011,
	RelRegControl	= 0b0100,
	RelAssControl	= 0b0101,
	NotePitchbend	= 0b0110,
	NoteOff			= 0b1000,
	NoteOn			= 0b1001,
	Aftertouch		= 0b1010,
	CControl		= 0b1011,
	ProgChange		= 0b1100,
	ChanPressure	= 0b1101,
	Pitchbend		= 0b1110,
	NoteManage		= 0b1111
};

struct MIDI1_msg_t
{
	uint8_t group;
	enum MIDI1_STATUS_E status;
	uint8_t channel;
	union{
		uint16_t bend;
		struct{uint8_t key; uint8_t velocity; };
		struct{uint8_t controller; uint8_t val; };
		uint8_t songNum;
		uint16_t songPos;
		uint8_t pressure;
		uint8_t instrument;
	};
};

struct MIDI2_util_t {
	enum MIDI2_UTIL_E status;
	union {
		uint16_t clk;
		uint16_t timestamp;
	}; 
};

struct MIDI2_com_t {
	uint8_t group;
	enum MIDI2_COM_E status;
	union {
		uint8_t timecode;
		uint8_t songNum;
		uint16_t songPos;
	};
};

struct MIDI2_data64_t {
	uint8_t group;
	enum MIDI2_DATA64_E status;
	uint8_t numBytes;
	uint8_t data[6];
};

struct MIDI2_data128_t {
	uint8_t group;
	enum MIDI2_DATA128_E status;
	union {
		uint8_t numBytes;
		uint8_t mdsID;
	};
	uint8_t streamID;
	uint8_t data[13];
};

struct MIDI2_voice_t {
	uint8_t group;
	enum MIDI2_VOICE_E status;
	uint8_t channel;
	union {
		uint8_t note;
		uint8_t bankCtrl;
	};
	union {
		uint8_t controller;
		uint8_t index;
		enum MIDI_ATTR_E attrType;
		uint8_t options;
	};
	union {
		uint32_t data;
		struct { uint16_t velocity; uint16_t attrVal; };
		struct { uint8_t program; uint16_t bankPC; };
	};
};

struct MIDI_UMP_t {
	MIDI_MT_E type;
	union {
		MIDI2_util_t util;
		MIDI2_com_t com;
		MIDI1_msg_t voice1;
		MIDI2_data64_t data64;
		MIDI2_voice_t voice2;
		MIDI2_data128_t data128;
	};
};

class MIDI_C
{
public:
	static void Convert(struct MIDI1_msg_t *msgOut, struct MIDI2_voice_t *msgIn);
	static void Convert(struct MIDI2_voice_t *msgOut, struct MIDI1_msg_t *msgIn);
	static void Convert(struct MIDI1_msg_t *msgOut, struct MIDI2_com_t *msgIn);
	static void Convert(struct MIDI2_com_t *msgOut, struct MIDI1_msg_t *msgIn);
	static uint8_t Encode(char* dataOut, struct MIDI2_voice_t* msgIn);
	static uint8_t Encode(char* dataOut, struct MIDI2_data128_t* msgIn);
	static uint8_t Encode(char* dataOut, struct MIDI2_data64_t* msgIn, uint8_t ver);
	static uint8_t Encode(char* dataOut, struct MIDI2_com_t* msgIn, uint8_t ver);
	static uint8_t Encode(char* dataOut, struct MIDI2_util_t* msgIn);
	static uint8_t Encode(char* dataOut, struct MIDI1_msg_t* msgIn, uint8_t ver);
	static uint8_t Encode(char* dataOut, struct MIDI_UMP_t* msgIn, uint8_t ver);
	void Set_channel_mask(uint16_t mask) {channelMask = mask;};
	void Set_group_mask(uint16_t mask) {groupMask = mask;};
	void Decode (char* data, uint8_t length);
	inline void Set_handler(void (*cb) (MIDI2_voice_t*)) {MIDI2_voice_p = cb;};
	inline void Set_handler(void (*cb) (MIDI2_data128_t*)) {MIDI2_data128_p = cb;};
	inline void Set_handler(void (*cb) (MIDI2_data64_t*)) {MIDI2_data64_p = cb;};
	inline void Set_handler(void (*cb) (MIDI2_com_t*)) {MIDI2_com_p = cb;};
	inline void Set_handler(void (*cb) (MIDI2_util_t*)) {MIDI2_util_p = cb;};
	inline void Set_handler(void (*cb) (MIDI1_msg_t*)) {MIDI1_p = cb;};
	inline void Set_handler(void (*cb) (MIDI_UMP_t*)) {MIDI_UMP_p = cb;};
	inline uint8_t Get_Version(void) {return MIDIVersion;};
	MIDI_C(uint8_t InitialVersion){
		MIDIVersion = InitialVersion;
		decodeState = Init;
	};
protected:
	void (* MIDI2_voice_p) (struct MIDI2_voice_t* msg);
	void (* MIDI2_data128_p) (struct MIDI2_data128_t* msg);
	void (* MIDI2_data64_p) (struct MIDI2_data64_t* msg);
	void (* MIDI2_com_p) (struct MIDI2_com_t* msg);
	void (* MIDI2_util_p) (struct MIDI2_util_t* msg);
	void (* MIDI1_p) (struct MIDI1_msg_t* msg);
	void (* MIDI_UMP_p) (struct MIDI_UMP_t* msg);
	uint8_t MIDIVersion;
	enum {
		Init,
		NormalMIDI1,
		SysexMIDI1,
		NormalMIDI2
	} decodeState;
	uint8_t msgIndex;
	int8_t msgLength;
	uint16_t channelMask = 0xffff;
	uint16_t groupMask = 0xffff;
	char msgBuffer[8];
};

#endif /* MIDI_2_DRIVER_H_ */