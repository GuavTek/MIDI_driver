﻿/*
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
	NOOP			= 0x0, 
	JRClk			= 0x1, 
	JRTimestamp		= 0x2,
	DeltaTPQ		= 0x3,
	DeltaPrevTick	= 0x4
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

enum class MIDI2_FORMAT_E {
	Single		= 0x0,
	Start		= 0x1,
	Continue	= 0x2,
	End			= 0x3
};

enum class MIDI2_FLEX_ADDR_E {
	Channel		= 0x0,
	Group		= 0x1,
	Reserved2	= 0x2,
	Reserved3	= 0x3
};

enum class MIDI2_FLEXDATA_E {
	SetTempo		= 0x0000,
	SetTimeSig		= 0x0001,
	SetMetronome	= 0x0002,
	SetKeySig		= 0x0005,
	SetChord		= 0x0006,
	MetaUnknown		= 0x0100,
	MetaProName		= 0x0101,
	MetaSongName	= 0x0102,
	MetaClipName	= 0x0103,
	MetaCopyright	= 0x0104,
	MetaComposer	= 0x0105,
	MetaLyricist	= 0x0106,
	MetaArranger	= 0x0107,
	MetaPublisher	= 0x0108,
	MetaPerformer	= 0x0109,
	MetaPerformer2	= 0x010a,
	MetaDate		= 0x010b,
	MetaLocation	= 0x010c,
	TextUnknown		= 0x0200,
	TextLyric		= 0x0201,
	TextLanguage	= 0x0202,
	TextRuby		= 0x0203,
	TextRubyLang	= 0x0204,
	Reserved3	= 0x0300
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

enum class MIDI2_STREAM_E {
	EndpointDiscovery	= 0x000,
	EndpointInfo		= 0x001,
	DeviceID			= 0x002,
	EndpointName		= 0x003,
	ProductInstance		= 0x004,
	ConfigReq			= 0x005,
	ConfigNotice		= 0x006,
	FunctionDiscovery	= 0x010,
	FunctionInfo		= 0x011,
	FunctionName		= 0x012,
	ClipStart			= 0x020,
	ClipEnd				= 0x021
};

enum class CHORD_TYPE_E {
	Clear	= 0x00,
	Maj		= 0x01,
	Maj6	= 0x02,
	Maj7	= 0x03,
	Maj9	= 0x04,
	Maj11	= 0x05,
	Maj13	= 0x06,
	Min		= 0x07,
	Min6	= 0x08,
	Min7	= 0x09,
	Min9	= 0x0a,
	Min11	= 0x0b,
	Min13	= 0x0c,
	Dom		= 0x0d,
	Dom9	= 0x0e,
	Dom11	= 0x0f,
	Dom13	= 0x10,
	Aug		= 0x11,
	Aug7	= 0x12,
	Dim		= 0x13,
	Dim7	= 0x14,
	HalfDim	= 0x15,
	MajMin	= 0x16,
	Pedal	= 0x17,
	Power	= 0x18,
	Sus2	= 0x19,
	Sus4	= 0x1a,
	Sus4_7	= 0x1b,
};

enum class CHORD_ALT_E {
	Normal	= 0x0,
	Add		= 0x1,
	Sub		= 0x2,
	Raise	= 0x3,
	Lower	= 0x4
};

enum class MIDI_DIR_E {
	Reserved	= 0x0,
	Input		= 0x1,
	Output		= 0x2,
	Bidirect	= 0x3
};

enum class MIDI1_BLOCK_E {
	None			= 0x0,
	Unrestricted	= 0x1,
	Restricted		= 0x2,
	Reserved		= 0x3
};
	
struct MIDI_chord_t {
	int8_t sharps;
	uint8_t tonic;
	enum CHORD_TYPE_E type;
	struct {
		enum CHORD_ALT_E type;
		int8_t degree;
	} alts[4];
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
		uint16_t tickPerQuarter;
		uint32_t tickLast;
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

struct MIDI2_flexdata_t {
	uint8_t group;
	enum MIDI2_FLEXDATA_E status;
	enum MIDI2_FORMAT_E format;
	enum MIDI2_FLEX_ADDR_E destination;
	uint8_t channel;
	union {
		uint8_t data[12];
		uint32_t tempo;
		struct {
			uint8_t numerator;
			uint8_t denominator;
			uint8_t numNotes;
		} timeSig;
		struct {
			uint8_t primaryClick;
			uint8_t accent[3];
			uint8_t subClick[2];
		} metronome;
		struct {
			int8_t sharps;
			uint8_t tonic;
		} keySig;
		struct {
			struct MIDI_chord_t mainChord;
			struct MIDI_chord_t bassChord;
		} chord;
	};
};

struct MIDI2_stream_t {
	enum MIDI2_FORMAT_E format;
	enum MIDI2_STREAM_E status;
	union {
		uint8_t data[14];
		struct {
			uint8_t verMaj;
			uint8_t verMin;
			bool reqInfo;
			bool reqDevID;
			bool reqName;
			bool reqInstID;
			bool reqStream;
			bool reserved5;
			bool reserved6;
			bool reserved7;
		} epDiscovery;
		struct {
			uint8_t verMaj;
			uint8_t verMin;
			bool isStatic;
			uint8_t funcNum;
			bool midi1;
			bool midi2;
			bool rxJR;
			bool txJR;
		} epInfo;
		struct {
			uint32_t sysexID;
			uint16_t devFamily;
			uint16_t devModel;
			uint32_t devVersion;
		} devID;
		struct {
			uint8_t protocol;
			bool rxJR;
			bool txJR;
		} streamCon;
		struct {
			uint8_t funcNum;
			bool reqInfo;
			bool reqName;
			bool reserved2;
			bool reserved3;
			bool reserved4;
			bool reserved5;
			bool reserved6;
			bool reserved7;
		} funcDiscovery;
		struct {
			bool isActive;
			uint8_t funcNum;
			enum MIDI_DIR_E hint;
			enum MIDI1_BLOCK_E midiSpeed;
			enum MIDI_DIR_E direction;
			uint8_t groupFirst;
			uint8_t groupSpan;
			uint8_t ciVersion;
			uint8_t sysexNum;
		} funcInfo;
		struct {
			uint8_t funcNum;
			uint8_t name[13];
		} funcName;
	};
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
		MIDI2_flexdata_t flex;
		MIDI2_stream_t stream;
	};
};

class MIDI_C
{
public:
	static void Convert(struct MIDI1_msg_t *msgOut, struct MIDI2_voice_t *msgIn);
	static void Convert(struct MIDI2_voice_t *msgOut, struct MIDI1_msg_t *msgIn);
	static void Convert(struct MIDI1_msg_t *msgOut, struct MIDI2_com_t *msgIn);
	static void Convert(struct MIDI2_com_t *msgOut, struct MIDI1_msg_t *msgIn);
	static void Convert(struct MIDI2_data64_t *msgOut, struct MIDI1_msg_t *msgIn);
	static void Convert(struct MIDI_UMP_t *msgOut, struct MIDI1_msg_t *msgIn);
	static uint8_t Encode(char* dataOut, struct MIDI2_voice_t* msgIn, uint8_t ver);
	static uint8_t Encode(char* dataOut, struct MIDI2_data128_t* msgIn);
	static uint8_t Encode(char* dataOut, struct MIDI2_data64_t* msgIn, uint8_t ver);
	static uint8_t Encode(char* dataOut, struct MIDI2_com_t* msgIn, uint8_t ver);
	static uint8_t Encode(char* dataOut, struct MIDI2_util_t* msgIn);
	static uint8_t Encode(char* dataOut, struct MIDI2_flexdata_t* msgIn);
	static uint8_t Encode(char* dataOut, struct MIDI2_stream_t* msgIn);
	static uint8_t Encode(char* dataOut, struct MIDI1_msg_t* msgIn, uint8_t ver);
	static uint8_t Encode(char* dataOut, struct MIDI_UMP_t* msgIn, uint8_t ver);
	inline void Set_channel_mask(uint16_t mask) {channelMask = mask;};
	inline void Set_group_mask(uint16_t mask) {groupMask = mask;};
	void Decode (char* data, uint8_t length);
	inline void Set_handler(void (*cb) (MIDI2_voice_t*)) {MIDI2_voice_p = cb;};
	inline void Set_handler(void (*cb) (MIDI2_data128_t*)) {MIDI2_data128_p = cb;};
	inline void Set_handler(void (*cb) (MIDI2_data64_t*)) {MIDI2_data64_p = cb;};
	inline void Set_handler(void (*cb) (MIDI2_com_t*)) {MIDI2_com_p = cb;};
	inline void Set_handler(void (*cb) (MIDI2_util_t*)) {MIDI2_util_p = cb;};
	inline void Set_handler(void (*cb) (MIDI2_flexdata_t*)) {MIDI2_flex_p = cb;};
	inline void Set_handler(void (*cb) (MIDI2_stream_t*)) {MIDI2_stream_p = cb;};
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
	void (* MIDI2_flex_p) (struct MIDI2_flexdata_t* msg);
	void (* MIDI2_stream_p) (struct MIDI2_stream_t* msg);
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
	uint16_t prevBank;
	char msgBuffer[16];
};

#endif /* MIDI_2_DRIVER_H_ */