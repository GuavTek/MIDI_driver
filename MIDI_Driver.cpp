/*
 * MIDI_Driver.cpp
 *
 * Created: 26/06/2021 14:39:29
 *  Author: GuavTek
 */ 

#include "MIDI_Driver.h"

void MIDI_C::Convert (struct MIDI1_msg_t *msgOut, struct MIDI2_voice_t *msgIn){
	msgOut->group = msgIn->group;
	msgOut->channel = msgIn->channel;
	msgOut->status = (MIDI1_STATUS_E) msgIn->status;
	switch(msgIn->status){
		case MIDI2_VOICE_E::NoteOff:
		case MIDI2_VOICE_E::NoteOn:
			msgOut->key = msgIn->note & 0x7f;
			msgOut->velocity = msgIn->velocity >> 9;
			break;
		case MIDI2_VOICE_E::CControl:
			msgOut->controller = msgIn->controller & 0x7f;
			msgOut->val = msgIn->data >> 25;
			break;
		case MIDI2_VOICE_E::Aftertouch:
			msgOut->key = msgIn->note;
			msgOut->velocity = msgIn->data >> 25;
			break;
		case MIDI2_VOICE_E::ProgChange:
			msgOut->instrument = msgIn->program & 0x7f;
			break;
		case MIDI2_VOICE_E::ChanPressure:
			msgOut->pressure = msgIn->data >> 25;
			break;
		case MIDI2_VOICE_E::Pitchbend:
			msgOut->bend = msgIn->data >> 18;
			break;
		default:
			msgOut->status = MIDI1_STATUS_E::Invalid;
			break;
	}
	return;
}

void MIDI_C::Convert(struct MIDI2_voice_t *msgOut, struct MIDI1_msg_t *msgIn){
	msgOut->group = msgIn->group;
	msgOut->channel = msgIn->channel;
	msgOut->status = (MIDI2_VOICE_E) msgIn->status;
	switch (msgIn->status)
	{
		case MIDI1_STATUS_E::NoteOn:
		case MIDI1_STATUS_E::NoteOff:
			msgOut->note = msgIn->key;
			// 7 bit to 16 bit stuffing
			msgOut->velocity = (((1 << 9) | (1 << 2)) * msgIn->velocity) | (msgIn->velocity >> 5);
			break;
		case MIDI1_STATUS_E::Aftertouch:
			msgOut->note = msgIn->key;
			// 7 bit to 32 bit stuffing
			msgOut->data = (((1 << 25) | (1 << 18) | (1 << 11) | (1 << 4)) * msgIn->velocity) | (msgIn->velocity >> 3);
			break;
		case MIDI1_STATUS_E::CControl:
			msgOut->controller = msgIn->controller;
			// 7 bit to 32 bit stuffing
			msgOut->data = (((1 << 25) | (1 << 18) | (1 << 11) | (1 << 4)) * msgIn->val) | (msgIn->val >> 3);
			break;
		case MIDI1_STATUS_E::ProgChange:
			msgOut->program = msgIn->instrument;
			break;
		case MIDI1_STATUS_E::ChanPressure:
			// 7 bit to 32 bit stuffing
			msgOut->data = (((1 << 25) | (1 << 18) | (1 << 11) | (1 << 4)) * msgIn->pressure) | (msgIn->pressure >> 3);
			break;
		case MIDI1_STATUS_E::Pitchbend:
			// 14 bit to 32 bit stuffing
			msgOut->data = (((1 << 18) | (1 << 4)) * msgIn->bend) | (msgIn->bend >> 10);
			break;
		default:
			break;
	}
}

void MIDI_C::Convert(struct MIDI1_msg_t *msgOut, struct MIDI2_com_t *msgIn){
	
}

void MIDI_C::Convert(struct MIDI2_com_t *msgOut, struct MIDI1_msg_t *msgIn){
	
	msgOut->status = (MIDI2_COM_E) msgIn->channel;
		
	if (msgIn->channel == 2) {
		msgOut->songPos = msgIn->songPos;
	} else if (msgIn->channel == 1) {
		msgOut->songNum = msgIn->songNum;
	}
}

uint8_t MIDI_C::Encode(char* dataOut, struct MIDI2_voice_t* msgIn){
	dataOut[0] = (((uint8_t) MIDI_MT_E::Voice2) << 4) | (msgIn->group & 0x0f);
	dataOut[1] = (((uint8_t) msgIn->status) << 4) | (msgIn->channel & 0x0f);
	switch (msgIn->status){
		case MIDI2_VOICE_E::RegNoteControl:
		case MIDI2_VOICE_E::AssNoteControl:
			dataOut[2] = msgIn->note;
			dataOut[3] = msgIn->controller;
			dataOut[4] = msgIn->data;
			break;
		case MIDI2_VOICE_E::RegControl:
		case MIDI2_VOICE_E::AssControl:
		case MIDI2_VOICE_E::RelRegControl:
		case MIDI2_VOICE_E::RelAssControl:
			dataOut[2] = msgIn->bankCtrl;
			dataOut[3] = msgIn->index;
			dataOut[4] = msgIn->data;
			break;
		case MIDI2_VOICE_E::NotePitchbend:
		case MIDI2_VOICE_E::Aftertouch:
			dataOut[2] = msgIn->note;
			dataOut[4] = msgIn->data;
			break;
		case MIDI2_VOICE_E::NoteOff:
		case MIDI2_VOICE_E::NoteOn:
			dataOut[2] = msgIn->note;
			dataOut[3] = (uint8_t) msgIn->attrType;
			dataOut[4] = msgIn->velocity;
			dataOut[6] = msgIn->attrVal;
			break;
		case MIDI2_VOICE_E::CControl:
			dataOut[2] = msgIn->controller;
			dataOut[4] = msgIn->data;
			break;
		case MIDI2_VOICE_E::ProgChange:
			dataOut[3] = msgIn->options;
			dataOut[4] = msgIn->program;
			dataOut[6] = msgIn->bankPC >> 7;
			dataOut[7] = msgIn->bankPC & 0x7f;
			break;
		case MIDI2_VOICE_E::ChanPressure:
		case MIDI2_VOICE_E::Pitchbend:
			dataOut[4] = msgIn->data;
			break;
		case MIDI2_VOICE_E::NoteManage:
			dataOut[2] = msgIn->note;
			dataOut[3] = msgIn->options;
			break;
	}
	return 8;
}

uint8_t MIDI_C::Encode(char* dataOut, struct MIDI2_data128_t* msgIn){
	dataOut[0] = (((uint8_t) MIDI_MT_E::Data128) << 4) | (msgIn->group & 0x0f);
	dataOut[1] = (uint8_t) msgIn->status << 4;
	uint8_t off = 2;
	if ((msgIn->status == MIDI2_DATA128_E::MixHead) || (msgIn->status == MIDI2_DATA128_E::MixPay)) {
		dataOut[1] |= msgIn->mdsID;
	} else {
		dataOut[1] |= msgIn->numBytes;
		dataOut[2] = msgIn->streamID;
		off = 3;
	}
	for (uint8_t i = 0; i < (15 - off); i++) {
		if (i > (15 - off)) break;
		dataOut[i+off] = msgIn->data[i];
	}
	return 16;
}

uint8_t MIDI_C::Encode(char* dataOut, struct MIDI2_data64_t* msgIn, uint8_t ver){
	uint8_t i = 0;
	if (ver == 2) {
		dataOut[i++] = (((uint8_t) MIDI_MT_E::Data64) << 4) | (msgIn->group & 0x0f);
		dataOut[i++] = ((uint8_t) msgIn->status << 4) | (msgIn->numBytes & 0x0f);
	} else if (msgIn->status == MIDI2_DATA64_E::Single || msgIn->status == MIDI2_DATA64_E::Start) {
		dataOut[i++] = 0xf0;		// Sysex start
	}
	// Data bytes
	for (uint8_t j = 0; j < msgIn->numBytes; j++) {
		dataOut[i++] = msgIn->data[j];
	}
	
	if (ver == 2) {
		return 8;
	} else {
		if (msgIn->status == MIDI2_DATA64_E::Single || msgIn->status == MIDI2_DATA64_E::End) {
			dataOut[i++] = 0xf7;	// Sysex end
		}
		return i;
	}
}

uint8_t MIDI_C::Encode(char* dataOut, struct MIDI2_com_t* msgIn, uint8_t ver){
	uint8_t i = 0;
	if (ver == 2) {
		// UMP format
		dataOut[i++] = (((uint8_t) MIDI_MT_E::RealTime) << 4) | (msgIn->group & 0x0f);
	}
	dataOut[i++] = (0xf << 4) | ((uint8_t) msgIn->status);
	switch (msgIn->status){
		case MIDI2_COM_E::TimeCode:
			dataOut[i++] = msgIn->timecode;
			break;
		case MIDI2_COM_E::SongPosPoint:
			dataOut[i++] = msgIn->songPos & 0x7f;
			dataOut[i++] = msgIn->songPos >> 7;
			break;
		case MIDI2_COM_E::SongSel:
			dataOut[i++] = msgIn->songNum;
			break;
		default:
			break;
	}
	if (ver == 2)
	{
		return 4;
	} 
	else
	{
		return i;
	}
}

uint8_t MIDI_C::Encode(char* dataOut, struct MIDI2_util_t* msgIn){
	dataOut[0] = (((uint8_t) MIDI_MT_E::Utility) << 4);
	dataOut[1] = (uint8_t) msgIn->status << 4;
	dataOut[2] = msgIn->timestamp;	// clk and timestamp should be the same bcs union
	return 4;
}

uint8_t MIDI_C::Encode(char* dataOut, struct MIDI1_msg_t* msgIn, uint8_t ver){
	uint8_t i = 0;
	if (ver == 2) {
		// UMP format
		dataOut[i++] = (((uint8_t) MIDI_MT_E::Voice1) << 4) | (msgIn->group & 0x0f);
	}
	
	dataOut[i++] = ((uint8_t) msgIn->status << 4) | (msgIn->channel & 0x0f);
	
	switch(msgIn->status){
		case MIDI1_STATUS_E::NoteOff:
		case MIDI1_STATUS_E::NoteOn:
		case MIDI1_STATUS_E::Aftertouch:
			dataOut[i++] = msgIn->key;
			dataOut[i++] = msgIn->velocity;
			break;
		case MIDI1_STATUS_E::CControl:
			dataOut[i++] = msgIn->controller;
			dataOut[i++] = msgIn->val;
			break;
		case MIDI1_STATUS_E::ProgChange:
			dataOut[i++] = msgIn->instrument;
			break;
		case MIDI1_STATUS_E::ChanPressure:
			dataOut[i++] = msgIn->pressure;
			break;
		case MIDI1_STATUS_E::Pitchbend:
			dataOut[i++] = msgIn->bend & 0x7f;
			dataOut[i++] = msgIn->bend >> 7;
			break;
		default:
			return 0;
	}
	
	if (ver == 2) {
		return 4;
	} else {
		return i;
	}
}

uint8_t MIDI_C::Encode(char* dataOut, struct MIDI_UMP_t* msgIn, uint8_t ver){
	switch(msgIn->type){
		case MIDI_MT_E::Utility:
			if (ver == 2){
				return Encode(dataOut, &msgIn->util);
			} else {
				// MIDI 2.0 only
				return 0;
			}
		case MIDI_MT_E::RealTime:
			return Encode(dataOut, &msgIn->com, ver);
		case MIDI_MT_E::Voice1:
			return Encode(dataOut, &msgIn->voice1, ver);
		case MIDI_MT_E::Data64:
			return Encode(dataOut, &msgIn->data64, ver);
		case MIDI_MT_E::Voice2:
			if (ver == 2){
				return Encode(dataOut, &msgIn->voice2);
			} else {
				MIDI1_msg_t msgConverted;
				Convert(&msgConverted, &msgIn->voice2);
				return Encode(dataOut, &msgConverted, 1);
			}
		case MIDI_MT_E::Data128:
			if (ver == 2){
				return Encode(dataOut, &msgIn->data128);
			} else {
				// MIDI 2.0 only
				return 0;
			}
		default:
			return 0;
	}
}

void MIDI_C::Decode (char* data, uint8_t length){	
	if (MIDIVersion == 2) {
		// MIDI 2.0 decoder
		for (uint8_t l = 0; l < length; ){
			char* inData = &data[l];
			MIDI_UMP_t msgCurrent;
			enum MIDI_MT_E msgType;
			msgType = (MIDI_MT_E) (inData[0] >> 4);
			msgCurrent.type = msgType;
			if(msgType == MIDI_MT_E::Utility) {
				l += 4;
				msgCurrent.util.status = (MIDI2_UTIL_E) (inData[1] >> 4);
				if (msgCurrent.util.status == MIDI2_UTIL_E::JRClk) {
					msgCurrent.util.clk = (inData[2] << 8) | inData[3];
				} else if (msgCurrent.util.status == MIDI2_UTIL_E::JRTimestamp) {
					msgCurrent.util.timestamp = (inData[2] << 8) | inData[3];
				}
			
				if (MIDI2_util_p != 0) {
					MIDI2_util_p(&msgCurrent.util);
				} else if (MIDI_UMP_p != 0)	{
					MIDI_UMP_p(&msgCurrent);
				}
			} else if (msgType == MIDI_MT_E::RealTime) {
				l += 4;
				msgCurrent.com.group = inData[0] & 0x0f;
				if (!(groupMask & (1 << msgCurrent.com.group))) continue;	// Masked out
				msgCurrent.com.status = (MIDI2_COM_E) (inData[1] && 0x0f);
				if (msgCurrent.com.status == MIDI2_COM_E::TimeCode) {
					msgCurrent.com.timecode = inData[2];
				} else if (msgCurrent.com.status == MIDI2_COM_E::SongPosPoint) {
					msgCurrent.com.songPos = inData[2] | (inData[3] << 7);
				} else if (msgCurrent.com.status == MIDI2_COM_E::SongSel) {
					msgCurrent.com.songNum = inData[2];
				}
			
				if (MIDI2_com_p != 0) {
					MIDI2_com_p(&msgCurrent.com);
				} else if (MIDI_UMP_p != 0) {
					MIDI_UMP_p(&msgCurrent);
				}
			} else if (msgType == MIDI_MT_E::Voice1) {
				l += 4;
				msgCurrent.voice1.group = inData[0] & 0x0f;
				msgCurrent.voice1.channel = inData[1] & 0x0f;
				if (!(groupMask & (1 << msgCurrent.voice1.group)) || !(channelMask & (1 << msgCurrent.voice1.channel))) continue;	// Masked out
				msgCurrent.voice1.status = (MIDI1_STATUS_E) (inData[1] >> 4);
				switch (msgCurrent.voice1.status){
					case MIDI1_STATUS_E::Aftertouch:
					case MIDI1_STATUS_E::NoteOn:
					case MIDI1_STATUS_E::NoteOff:
						msgCurrent.voice1.key = inData[2];
						msgCurrent.voice1.velocity = inData[3];
						break;
					case MIDI1_STATUS_E::CControl:
						msgCurrent.voice1.controller = inData[2];
						msgCurrent.voice1.val = inData[3];
						break;
					case MIDI1_STATUS_E::ProgChange:
						msgCurrent.voice1.instrument = inData[2];
						break;
					case MIDI1_STATUS_E::ChanPressure:
						msgCurrent.voice1.pressure = inData[2];
						break;
					case MIDI1_STATUS_E::Pitchbend:
						msgCurrent.voice1.bend = inData[2] | (inData[3] << 7);
						break;
					default:
						msgCurrent.voice1.status = MIDI1_STATUS_E::Invalid;
						break;
				}
				
				if (msgCurrent.voice1.status == MIDI1_STATUS_E::Invalid) {
					return;
				} else if (MIDI1_p != 0) {
					MIDI1_p(&msgCurrent.voice1);
				} else if (MIDI_UMP_p != 0) {
					MIDI_UMP_p(&msgCurrent);
				}
			} else if (msgType == MIDI_MT_E::Data64) {
				l += 8;
				msgCurrent.data64.group = inData[0] & 0x0f;
				if (!(groupMask & (1 << msgCurrent.data64.group))) continue;	// Masked out
				msgCurrent.data64.status = (MIDI2_DATA64_E) (inData[1] >> 4);
				msgCurrent.data64.numBytes = inData[1] & 0x0f;
				for (uint8_t i = 0; i < (length - 2); i++) {
					if (i > 5) break;
					msgCurrent.data64.data[i] = inData[i+2];
				}
				
				if (MIDI2_data64_p != 0) {
					MIDI2_data64_p(&msgCurrent.data64);
				} else if (MIDI_UMP_p != 0) {
					MIDI_UMP_p(&msgCurrent);
				}
			} else if (msgType == MIDI_MT_E::Voice2) {
				l += 8;
				msgCurrent.voice2.group = inData[0] & 0x0f;
				msgCurrent.voice2.channel = inData[1] & 0x0f;
				if (!(groupMask & (1 << msgCurrent.voice2.group)) || !(channelMask & (1 << msgCurrent.voice2.channel))) continue;	// Masked out
				msgCurrent.voice2.status = (MIDI2_VOICE_E) (inData[1] >> 4);
				switch (msgCurrent.voice2.status){
					case MIDI2_VOICE_E::RegNoteControl:
					case MIDI2_VOICE_E::AssNoteControl:
						msgCurrent.voice2.note = inData[2];
						msgCurrent.voice2.controller = inData[3];
						msgCurrent.voice2.data = (uint32_t) inData[4];
						break;
					case MIDI2_VOICE_E::RegControl:
					case MIDI2_VOICE_E::AssControl:
					case MIDI2_VOICE_E::RelRegControl:
					case MIDI2_VOICE_E::RelAssControl:
						msgCurrent.voice2.bankCtrl = inData[2];
						msgCurrent.voice2.index = inData[3];
						msgCurrent.voice2.data = (uint32_t) inData[4];
						break;
					case MIDI2_VOICE_E::NotePitchbend:
					case MIDI2_VOICE_E::Aftertouch:
						msgCurrent.voice2.note = inData[2];
						msgCurrent.voice2.data = (uint32_t) inData[4];
						break;
					case MIDI2_VOICE_E::NoteOff:
					case MIDI2_VOICE_E::NoteOn:
						msgCurrent.voice2.note = inData[2];
						msgCurrent.voice2.attrType = (MIDI_ATTR_E) inData[3];
						msgCurrent.voice2.velocity = (uint16_t) inData[4];
						msgCurrent.voice2.attrVal = (uint16_t) inData[6]; 
						break;
					case MIDI2_VOICE_E::CControl:
						msgCurrent.voice2.controller = inData[2];
						msgCurrent.voice2.data = (uint16_t) inData[4];
						break;
					case MIDI2_VOICE_E::ProgChange:
						msgCurrent.voice2.options = inData[3];
						msgCurrent.voice2.program = inData[4];
						msgCurrent.voice2.bankPC = (inData[6] << 7) | inData[7];
						break;
					case MIDI2_VOICE_E::ChanPressure:
					case MIDI2_VOICE_E::Pitchbend:
						msgCurrent.voice2.data = (uint32_t) inData[4];
						break;
					case MIDI2_VOICE_E::NoteManage:
						msgCurrent.voice2.note = inData[2];
						msgCurrent.voice2.options = inData[3];
						break;
				}
				if (MIDI2_voice_p != 0) {
					MIDI2_voice_p(&msgCurrent.voice2);
				} else if (MIDI_UMP_p != 0) {
					MIDI_UMP_p(&msgCurrent);
				}
			} else if (msgType == MIDI_MT_E::Data128) {
				l += 16;
				msgCurrent.data128.group = inData[0] & 0x0f;
				if (!(groupMask & (1 << msgCurrent.data128.group))) continue;	// Masked out
				msgCurrent.data128.status = (MIDI2_DATA128_E) (inData[1] >> 4);
				uint8_t off = 2;
				if ((msgCurrent.data128.status == MIDI2_DATA128_E::MixHead) || (msgCurrent.data128.status == MIDI2_DATA128_E::MixPay)) {
					msgCurrent.data128.mdsID = inData[1] & 0x0f;
					} else {
					msgCurrent.data128.numBytes = inData[1] & 0x0f;
					msgCurrent.data128.streamID = inData[2];
					off = 3;
				}
				for (uint8_t i = 0; i < (length - off); i++) {
					if (i > (15 - off)) break;
					msgCurrent.data128.data[i] = inData[i+off];
				}
				
				// Can not be converted to MIDI 1.0
				if (MIDI2_data128_p != 0){
					MIDI2_data128_p(&msgCurrent.data128);
				} else if (MIDI_UMP_p != 0) {
					MIDI_UMP_p(&msgCurrent);
				}
			} else if (msgType == MIDI_MT_E::Reserved6)	{
				l += 4;
			} else if (msgType == MIDI_MT_E::Reserved7)	{
				l += 4;
			} else if (msgType == MIDI_MT_E::Reserved8)	{
				l += 8;
			} else if (msgType == MIDI_MT_E::Reserved9)	{
				l += 8;
			} else if (msgType == MIDI_MT_E::Reserved10)	{
				l += 8;
			} else if (msgType == MIDI_MT_E::Reserved11)	{
				l += 12;
			} else if (msgType == MIDI_MT_E::Reserved12)	{
				l += 12;
			} else if (msgType == MIDI_MT_E::FlexData)	{
				l += 16;
				bool maskedOut;
				msgCurrent.flex.group = inData[0] & 0xf;
				maskedOut = !(groupMask & (1 << msgCurrent.flex.group));
				msgCurrent.flex.channel = inData[1] & 0xf;
				msgCurrent.flex.destination = (MIDI2_FLEX_ADDR_E) ((inData[1] >> 4) & 0x3);
				msgCurrent.flex.format = (MIDI2_FORMAT_E) ((inData[1] >> 6) & 0x3);
				maskedOut |= (msgCurrent.flex.destination == MIDI2_FLEX_ADDR_E::Channel) && !(channelMask & (1 << msgCurrent.flex.channel));
				if (maskedOut) continue;
				msgCurrent.flex.status = (MIDI2_FLEXDATA_E) ((inData[2] << 8) | inData[3]);
				switch (msgCurrent.flex.status){
					case MIDI2_FLEXDATA_E::SetTempo:
						msgCurrent.flex.tempo = (inData[4] << 24) | (inData[5] << 16) | (inData[6] << 8) | inData[7];
						break;
					case MIDI2_FLEXDATA_E::SetTimeSig:
						msgCurrent.flex.timeSig.numerator = inData[4];
						msgCurrent.flex.timeSig.denominator = inData[5];
						msgCurrent.flex.timeSig.numNotes = inData[6];
						break;
					case MIDI2_FLEXDATA_E::SetMetronome:
						msgCurrent.flex.metronome.primaryClick = inData[4];
						for (uint8_t i = 0; i < 3; i++){
							msgCurrent.flex.metronome.accent[i] = inData[i+5];
						}
						for (uint8_t i = 0; i < 2; i++){
							msgCurrent.flex.metronome.subClick[i] = inData[i+8];
						}
						break;
					case MIDI2_FLEXDATA_E::SetKeySig:
						struct {int8_t val : 4;} tempSigned;	// Make sure 4-bit value gets sign-extended
						msgCurrent.flex.keySig.tonic = inData[4] & 0xf;
						tempSigned.val = inData[4] >> 4;
						msgCurrent.flex.keySig.sharps = tempSigned.val;
						break;
					case MIDI2_FLEXDATA_E::SetChord:
						struct {int8_t val : 4;} tempSigned;	// Make sure 4-bit value gets sign-extended
						msgCurrent.flex.chord.mainChord.tonic = inData[4] & 0xf;
						tempSigned.val = inData[4] >> 4;
						msgCurrent.flex.chord.mainChord.sharps = tempSigned.val;
						msgCurrent.flex.chord.mainChord.type = (CHORD_TYPE_E) inData[5];
						for (uint8_t i = 0; i < 4; i++){
							tempSigned.val = inData[i+6] & 0xf;
							msgCurrent.flex.chord.mainChord.alts[i].degree = tempSigned.val;
							msgCurrent.flex.chord.mainChord.alts[i].type = (CHORD_ALT_E) (inData[i+6] >> 4);
						}
						msgCurrent.flex.chord.bassChord.tonic = inData[12] & 0xf;
						tempSigned.val = inData[12] >> 4;
						msgCurrent.flex.chord.bassChord.sharps = tempSigned.val;
						msgCurrent.flex.chord.bassChord.type = (CHORD_TYPE_E) inData[13];
						for (uint8_t i = 0; i < 2; i++){
							tempSigned.val = inData[i+14] & 0xf;
							msgCurrent.flex.chord.bassChord.alts[i].degree = tempSigned.val;
							msgCurrent.flex.chord.bassChord.alts[i].type = (CHORD_ALT_E) (inData[i+14] >> 4);
						}
						break;
					default:
						for (uint8_t i = 0; i < 12; i++){
							msgCurrent.flex.data[i] = inData[i+4];
						}
						break;
				}
				if (MIDI2_flex_p != 0){
					MIDI2_flex_p(&msgCurrent.flex);
				} else if (MIDI_UMP_p != 0){
					MIDI_UMP_p(&msgCurrent);
				}
			} else if (msgType == MIDI_MT_E::Reserved14)	{
				l += 16;
			} else if (msgType == MIDI_MT_E::Stream)	{
				l += 16;
				msgCurrent.stream.status = (MIDI2_STREAM_E) ((inData[1] | inData[0] << 8) & 0x03ff);
				switch (msgCurrent.stream.status){
					case MIDI2_STREAM_E::EndpointDiscovery:
						msgCurrent.stream.epDiscovery.verMaj = inData[2];
						msgCurrent.stream.epDiscovery.verMin = inData[3];
						msgCurrent.stream.epDiscovery.reqInfo = inData[7] & 0b1;
						msgCurrent.stream.epDiscovery.reqDevID = (inData[7] >> 1) & 0b1;
						msgCurrent.stream.epDiscovery.reqName = (inData[7] >> 2) & 0b1;
						msgCurrent.stream.epDiscovery.reqInstID = (inData[7] >> 3) & 0b1;
						msgCurrent.stream.epDiscovery.reqStream = (inData[7] >> 4) & 0b1;
						break;
					case MIDI2_STREAM_E::EndpointInfo:
						msgCurrent.stream.epInfo.verMaj = inData[2];
						msgCurrent.stream.epInfo.verMin = inData[3];
						msgCurrent.stream.epInfo.isStatic = inData[4] >> 7;
						msgCurrent.stream.epInfo.funcNum = inData[4] & 0x7f;
						msgCurrent.stream.epInfo.midi1 = inData[6] & 0b1;
						msgCurrent.stream.epInfo.midi2 = (inData[6] >> 1) & 0b1;
						msgCurrent.stream.epInfo.txJR = inData[7] & 0b1;
						msgCurrent.stream.epInfo.rxJR = (inData[7] >> 1) & 0b1;
						break;
					case MIDI2_STREAM_E::DeviceID:
						msgCurrent.stream.devID.sysexID = (inData[5] | (inData[6] << 7) | (inData[7] << 14)) & 0x1fffff;
						msgCurrent.stream.devID.devFamily = (inData[8] | (inData[9] << 7)) & 0x3fff;
						msgCurrent.stream.devID.devModel = (inData[10] | (inData[11] << 7)) & 0x3fff;
						msgCurrent.stream.devID.devVersion = (inData[12] | (inData[13] << 7) | (inData[14] << 14) | (inData[15] << 21)) & 0x0fffffff;
						break;
					case MIDI2_STREAM_E::EndpointName:
					case MIDI2_STREAM_E::ProductInstance:
						for (uint8_t i = 0; i < 14; i++){
							msgCurrent.stream.data[i] = inData[i+2];
						}
						break;
					case MIDI2_STREAM_E::ConfigReq:
					case MIDI2_STREAM_E::ConfigNotice:
						msgCurrent.stream.streamCon.protocol = inData[2];
						msgCurrent.stream.streamCon.rxJR = (inData[3] >> 1) & 0b1;
						msgCurrent.stream.streamCon.txJR = inData[3] & 0b1;
						break;
					case MIDI2_STREAM_E::FunctionDiscovery:
						msgCurrent.stream.funcDiscovery.funcNum = inData[2];
						msgCurrent.stream.funcDiscovery.reqInfo = inData[3] & 0b1;
						msgCurrent.stream.funcDiscovery.reqName = (inData[3] >> 1) & 0b1;
						break;
					case MIDI2_STREAM_E::FunctionInfo:
						msgCurrent.stream.funcInfo.isActive = inData[2] >> 7;
						msgCurrent.stream.funcInfo.funcNum = inData[2] & 0x7f;
						msgCurrent.stream.funcInfo.hint = (MIDI_DIR_E) ((inData[3] >> 4) & 0b11);
						msgCurrent.stream.funcInfo.midiSpeed = (MIDI1_BLOCK_E) ((inData[3] >> 2) & 0b11);
						msgCurrent.stream.funcInfo.direction = (MIDI_DIR_E) (inData[3] & 0b11);
						msgCurrent.stream.funcInfo.groupFirst = inData[4];
						msgCurrent.stream.funcInfo.groupSpan = inData[5];
						msgCurrent.stream.funcInfo.ciVersion = inData[6];
						msgCurrent.stream.funcInfo.sysexNum = inData[7];
						break;
					case MIDI2_STREAM_E::FunctionName:
						msgCurrent.stream.funcName.funcNum = inData[2];
						for (uint8_t i = 0; i < 13; i++){
							msgCurrent.stream.funcName.name[i] = inData[i+3];
						}
						break;
					default:
						break;
				}				
				// Can not be converted to MIDI 1.0
				if (MIDI2_stream_p != 0){
					MIDI2_stream_p(&msgCurrent.stream);
				} else if (MIDI_UMP_p != 0) {
					MIDI_UMP_p(&msgCurrent);
				}
			}
		}
	} else {
		// MIDI 1.0 decoder
		for (uint8_t i = 0; i < length; i++) {
			msgBuffer[msgIndex] = data[i];
			if (data[i] & 0x80) {
				// New command
				MIDI1_STATUS_E command = (MIDI1_STATUS_E) (data[i] >> 4);
				switch (command) {
					case MIDI1_STATUS_E::SysEx:
						if (data[i] == 0xf0) {
							// Sysex start
							msgLength = 8;
							decodeState = SysexMIDI1;
						} else if (data[i] == 0xf7) {
							// Sysex end
							msgLength = msgIndex;
							//decodeState = NormalMIDI1;
						} else if ((data[i] & 0x0f) == 2) {
							msgLength = 3;
							decodeState = NormalMIDI1;
						} else if ((data[i] & 0x0f) == 3) {
							msgLength = 2;
							decodeState = NormalMIDI1;
						} else {
							msgLength = 1;
							// Realtime does not interrupt sysex
						}
						break;
					case MIDI1_STATUS_E::ProgChange:
					case MIDI1_STATUS_E::ChanPressure:
						msgLength = 2;
						decodeState = NormalMIDI1;
						break;
					default:
						msgLength = 3;
						decodeState = NormalMIDI1;
						break;
				}
			}
			if (msgIndex < msgLength) {
				msgIndex++;
			}
			if (msgIndex == msgLength) {
				// Decode buffered message
				MIDI1_msg_t tempMsg;
				tempMsg.channel = msgBuffer[0] & 0x0f;
				if (!(channelMask & (1 << tempMsg.channel))) {
					// Channel is masked out, skip this
					msgIndex = 0;
					msgLength = -1;
					continue;
				}
				tempMsg.status = (MIDI1_STATUS_E) (msgBuffer[0] >> 4);
				switch (tempMsg.status) {
				case MIDI1_STATUS_E::NoteOn:
				case MIDI1_STATUS_E::NoteOff:
				case MIDI1_STATUS_E::Aftertouch:
					tempMsg.key = msgBuffer[1];
					tempMsg.velocity = msgBuffer[2];
					if (MIDI2_voice_p != 0) {
						MIDI2_voice_t tempMsg2;
						Convert(&tempMsg2, &tempMsg);
						MIDI2_voice_p(&tempMsg2);
					}
					break;
				case MIDI1_STATUS_E::CControl:
					tempMsg.controller = msgBuffer[1];
					tempMsg.val = msgBuffer[2];
					if (MIDI2_voice_p != 0) {
						MIDI2_voice_t tempMsg2;
						Convert(&tempMsg2, &tempMsg);
						MIDI2_voice_p(&tempMsg2);
					}
					break;
				case MIDI1_STATUS_E::ProgChange:
					tempMsg.instrument = msgBuffer[1];
					if (MIDI2_voice_p != 0) {
						MIDI2_voice_t tempMsg2;
						Convert(&tempMsg2, &tempMsg);
						MIDI2_voice_p(&tempMsg2);
					}
					break;
				case MIDI1_STATUS_E::ChanPressure:
					tempMsg.pressure = msgBuffer[1];
					if (MIDI2_voice_p != 0) {
						MIDI2_voice_t tempMsg2;
						Convert(&tempMsg2, &tempMsg);
						MIDI2_voice_p(&tempMsg2);
					}
					break;
				case MIDI1_STATUS_E::Pitchbend:
					tempMsg.bend = msgBuffer[1] | (msgBuffer[2] << 7);
					if (MIDI2_voice_p != 0) {
						MIDI2_voice_t tempMsg2;
						Convert(&tempMsg2, &tempMsg);
						MIDI2_voice_p(&tempMsg2);
					}
					break;
				case MIDI1_STATUS_E::SysEx:
					if (tempMsg.channel == 0) {
						MIDI2_data64_t msgData;
						tempMsg.status = MIDI1_STATUS_E::Invalid;
						msgData.numBytes = msgIndex - 1;
						msgData.status = MIDI2_DATA64_E::Start;
						
						for (uint8_t i = 0; i <= msgIndex; i++) {
							if (msgBuffer[i] == 0xf7) {
								// End of sysex
								decodeState = NormalMIDI1;
								msgData.numBytes--;
								msgData.status = MIDI2_DATA64_E::Single;
								break;
							}
							msgData.data[i] = msgBuffer[i];
						}
						if (MIDI2_data64_p != 0) {
							MIDI2_data64_p(&msgData);
							msgIndex = 0;
							if (decodeState == NormalMIDI1) {
								msgLength = -1;
							}
							return;
						}
					} else if (tempMsg.channel == 7) {
						MIDI2_data64_t msgData;
						tempMsg.status = MIDI1_STATUS_E::Invalid;
						msgData.status = MIDI2_DATA64_E::End;
						msgData.numBytes = 0;
						decodeState = NormalMIDI1;
						if (MIDI2_data64_p != 0) {
							MIDI2_data64_p(&msgData);
							msgIndex = 0;
							if (decodeState == NormalMIDI1) {
								msgLength = -1;
							}
							return;
						}
					} else {
						// Common and realtime messages
						if (tempMsg.channel == 2) {
							tempMsg.songPos = msgBuffer[1] | (msgBuffer[2] << 7);
						} else if (tempMsg.channel == 1) {
							tempMsg.songNum = msgBuffer[1];
						}
						if (MIDI2_com_p != 0) {
							MIDI2_com_t tempMsg2;
							Convert(&tempMsg2, &tempMsg);
							MIDI2_com_p(&tempMsg2);
							msgIndex = 0;
							if (decodeState == NormalMIDI1) {
								msgLength = -1;
							}
							return;
						}
					}
					break;
				default:
					if (decodeState == SysexMIDI1) {
						MIDI2_data64_t msgData;
						msgData.status = MIDI2_DATA64_E::Continue;
						msgData.numBytes = msgIndex;
						for (uint8_t i = 0; i <= msgIndex; i++) {
							if (msgBuffer[i] == 0xf7) {
								// End of sysex
								decodeState = NormalMIDI1;
								msgData.numBytes--;
								msgData.status = MIDI2_DATA64_E::End;
								break;
							}
							msgData.data[i] = msgBuffer[i];
						}
						if (MIDI2_data64_p != 0) {
							MIDI2_data64_p(&msgData);
							msgIndex = 0;
							if (decodeState == NormalMIDI1) {
								msgLength = -1;
							}
							return;
						}
					}
					tempMsg.status = MIDI1_STATUS_E::Invalid;
					break;
				}
				
				msgIndex = 0;
				if (decodeState == NormalMIDI1) {
					msgLength = -1;
				}
				if (MIDI1_p != 0) {
					MIDI1_p(&tempMsg);
				} else if (MIDI_UMP_p != 0) {
					MIDI_UMP_t tempMsg2;
					tempMsg2.voice1 = tempMsg;
					tempMsg2.type = MIDI_MT_E::Voice1;
					MIDI_UMP_p(&tempMsg2);
				}
			}
		}
	}
}
