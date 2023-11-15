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
			msgOut->options = 0;
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
	// Probably not needed, but TODO?
}

void MIDI_C::Convert(struct MIDI2_com_t *msgOut, struct MIDI1_msg_t *msgIn){
	msgOut->status = (MIDI2_COM_E) msgIn->channel;
		
	if (msgIn->channel == 2) {
		msgOut->songPos = msgIn->songPos;
	} else if (msgIn->channel == 1) {
		msgOut->songNum = msgIn->songNum;
	}
}

void MIDI_C::Convert(struct MIDI2_data64_t *msgOut, struct MIDI1_msg_t *msgIn){
	// Probably not needed, is always decoded to data64 directly
	// Data field is not in struct, but maybe TODO??
}

void MIDI_C::Convert(struct MIDI_UMP_t *msgOut, struct MIDI1_msg_t *msgIn){
	switch(msgIn->status){
		case MIDI1_STATUS_E::NoteOff:
		case MIDI1_STATUS_E::NoteOn:
		case MIDI1_STATUS_E::Aftertouch:
		case MIDI1_STATUS_E::CControl:
		case MIDI1_STATUS_E::ProgChange:
		case MIDI1_STATUS_E::ChanPressure:
		case MIDI1_STATUS_E::Pitchbend:
			Convert(&msgOut->voice2, msgIn);
			break;
		case MIDI1_STATUS_E::SysEx:
			if ((msgIn->channel == 0)||(msgIn->channel == 7)){
				// Data64
				Convert(&msgOut->data64, msgIn);
			} else {
				// realtime
				Convert(&msgOut->com, msgIn);
			}
			break;
		default:
			break;
	}
}

uint8_t MIDI_C::Encode(char* dataOut, struct MIDI2_voice_t* msgIn, uint8_t ver){
	if (ver == 1){
		uint8_t i = 0;
		MIDI1_msg_t tempMsg;
		Convert(&tempMsg, msgIn);
		if ((msgIn->status == MIDI2_VOICE_E::ProgChange) && (msgIn->options & 0b1)){
			// Insert bank change
			dataOut[i++] = ((uint8_t) MIDI1_STATUS_E::CControl << 4) | (msgIn->channel & 0x0f);
			dataOut[i++] = 32;	// LSB
			dataOut[i++] = msgIn->bankPC & 0x7f;
			dataOut[i++] = ((uint8_t) MIDI1_STATUS_E::CControl << 4) | (msgIn->channel & 0x0f);
			dataOut[i++] = 0;	// MSB
			dataOut[i++] = (msgIn->bankPC >> 7) & 0x7f;
		}
		uint8_t j = Encode(&dataOut[i], &tempMsg, 1);
		return i + j;
	}
	dataOut[0] = (((uint8_t) MIDI_MT_E::Voice2) << 4) | (msgIn->group & 0x0f);
	dataOut[1] = (((uint8_t) msgIn->status) << 4) | (msgIn->channel & 0x0f);
	switch (msgIn->status){
		case MIDI2_VOICE_E::RegNoteControl:
		case MIDI2_VOICE_E::AssNoteControl:
			dataOut[2] = msgIn->note;
			dataOut[3] = msgIn->controller;
			dataOut[4] = msgIn->data >> 24;
			dataOut[5] = msgIn->data >> 16;
			dataOut[6] = msgIn->data >> 8;
			dataOut[7] = msgIn->data;
			break;
		case MIDI2_VOICE_E::RegControl:
		case MIDI2_VOICE_E::AssControl:
		case MIDI2_VOICE_E::RelRegControl:
		case MIDI2_VOICE_E::RelAssControl:
			dataOut[2] = msgIn->bankCtrl;
			dataOut[3] = msgIn->index;
			dataOut[4] = msgIn->data >> 24;
			dataOut[5] = msgIn->data >> 16;
			dataOut[6] = msgIn->data >> 8;
			dataOut[7] = msgIn->data;
			break;
		case MIDI2_VOICE_E::NotePitchbend:
		case MIDI2_VOICE_E::Aftertouch:
			dataOut[2] = msgIn->note;
			dataOut[4] = msgIn->data >> 24;
			dataOut[5] = msgIn->data >> 16;
			dataOut[6] = msgIn->data >> 8;
			dataOut[7] = msgIn->data;
			break;
		case MIDI2_VOICE_E::NoteOff:
		case MIDI2_VOICE_E::NoteOn:
			dataOut[2] = msgIn->note;
			dataOut[3] = (uint8_t) msgIn->attrType;
			dataOut[4] = msgIn->velocity >> 8;
			dataOut[5] = msgIn->velocity;
			dataOut[6] = msgIn->attrVal >> 8;
			dataOut[7] = msgIn->attrVal;
			break;
		case MIDI2_VOICE_E::CControl:
			dataOut[2] = msgIn->controller;
			dataOut[4] = msgIn->data >> 24;
			dataOut[5] = msgIn->data >> 16;
			dataOut[6] = msgIn->data >> 8;
			dataOut[7] = msgIn->data;
			break;
		case MIDI2_VOICE_E::ProgChange:
			dataOut[3] = msgIn->options;
			dataOut[4] = msgIn->program;
			dataOut[6] = msgIn->bankPC >> 7;
			dataOut[7] = msgIn->bankPC & 0x7f;
			break;
		case MIDI2_VOICE_E::ChanPressure:
		case MIDI2_VOICE_E::Pitchbend:
			dataOut[4] = msgIn->data >> 24;
			dataOut[5] = msgIn->data >> 16;
			dataOut[6] = msgIn->data >> 8;
			dataOut[7] = msgIn->data;
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
	if (ver == 2){
		return 4;
	} else {
		return i;
	}
}

uint8_t MIDI_C::Encode(char* dataOut, struct MIDI2_util_t* msgIn){
	dataOut[0] = (((uint8_t) MIDI_MT_E::Utility) << 4);
	dataOut[1] = (uint8_t) msgIn->status << 4;
	switch (msgIn->status){
		case MIDI2_UTIL_E::JRClk:
		case MIDI2_UTIL_E::JRTimestamp:
		case MIDI2_UTIL_E::DeltaTPQ:	// Union should mean these are in the same memory
			dataOut[2] = msgIn->timestamp >> 8;
			dataOut[3] = msgIn->timestamp;
			break;
		case MIDI2_UTIL_E::DeltaPrevTick:
			dataOut[1] |= (msgIn->tickLast >> 16) & 0x0f;
			dataOut[2] = msgIn->tickLast >> 8;
			dataOut[3] = msgIn->tickLast;
			break;
		default:
			break;
	}
	return 4;
}

uint8_t MIDI_C::Encode(char* dataOut, struct MIDI2_flexdata_t* msgIn){
	dataOut[0] = ((uint8_t) MIDI_MT_E::FlexData) << 4;
	dataOut[0] |= msgIn->group & 0xf;
	dataOut[1] = msgIn->channel & 0xf;
	dataOut[1] |= (uint8_t) msgIn->destination << 4;
	dataOut[1] |= (uint8_t) msgIn->format << 6;
	dataOut[2] = (uint16_t) msgIn->status >> 8;
	dataOut[3] = (uint8_t) msgIn->status;
	switch (msgIn->status){
		case MIDI2_FLEXDATA_E::SetTempo:
			dataOut[4] = msgIn->tempo >> 24;
			dataOut[5] = msgIn->tempo >> 16;
			dataOut[6] = msgIn->tempo >> 8;
			dataOut[7] = msgIn->tempo;
			break;
		case MIDI2_FLEXDATA_E::SetTimeSig:
			dataOut[4] = msgIn->timeSig.numerator;
			dataOut[5] = msgIn->timeSig.denominator;
			dataOut[6] = msgIn->timeSig.numNotes;
			break;
		case MIDI2_FLEXDATA_E::SetMetronome:
			dataOut[4] = msgIn->metronome.primaryClick;
			dataOut[5] = msgIn->metronome.accent[0];
			dataOut[6] = msgIn->metronome.accent[1];
			dataOut[7] = msgIn->metronome.accent[2];
			dataOut[8] = msgIn->metronome.subClick[0];
			dataOut[9] = msgIn->metronome.subClick[1];
			break;
		case MIDI2_FLEXDATA_E::SetKeySig:
			dataOut[4] = msgIn->keySig.tonic & 0xf;
			dataOut[4] |= msgIn->keySig.sharps << 4;
			break;
		case MIDI2_FLEXDATA_E::SetChord:
			dataOut[4] = msgIn->chord.mainChord.tonic & 0xf;
			dataOut[4] |= msgIn->chord.mainChord.sharps << 4;
			dataOut[5] = (uint8_t) msgIn->chord.mainChord.type;
			for (uint8_t i = 0; i < 4; i++){
				dataOut[i+6] = msgIn->chord.mainChord.alts[i].degree & 0xf;
				dataOut[i+6] = (uint8_t) msgIn->chord.mainChord.alts[i].type >> 4;
			}
			dataOut[12] = msgIn->chord.bassChord.tonic & 0xf;
			dataOut[12] |= msgIn->chord.bassChord.sharps << 4;
			dataOut[13] = (uint8_t) msgIn->chord.bassChord.type;
			for (uint8_t i = 0; i < 2; i++){
				dataOut[i+14] = msgIn->chord.bassChord.alts[i].degree & 0xf;
				dataOut[i+14] = (uint8_t) msgIn->chord.bassChord.alts[i].type >> 4;
			}
			break;
		default:
			for (uint8_t i = 0; i < 12; i++){
				dataOut[i+4] = msgIn->data[i];
			}
			break;
	}
	return 16;
}

uint8_t MIDI_C::Encode(char* dataOut, struct MIDI2_stream_t* msgIn){
	dataOut[0] = ((uint8_t) MIDI_MT_E::FlexData) << 4;
	dataOut[0] |= ((uint8_t) msgIn->format << 2);
	dataOut[0] |= ((uint16_t) msgIn->status >> 8) & 0x3;
	dataOut[1] = (uint8_t) msgIn->status;
	switch (msgIn->status){
		case MIDI2_STREAM_E::EndpointDiscovery:
			dataOut[2] = msgIn->epDiscovery.verMaj;
			dataOut[3] = msgIn->epDiscovery.verMin;
			dataOut[7] = msgIn->epDiscovery.reqInfo & 0b1;
			dataOut[7] |= (msgIn->epDiscovery.reqDevID & 0b1) << 1;
			dataOut[7] |= (msgIn->epDiscovery.reqName & 0b1) << 2;
			dataOut[7] |= (msgIn->epDiscovery.reqInstID & 0b1) << 3;
			dataOut[7] |= (msgIn->epDiscovery.reqStream & 0b1) << 4;
			break;
		case MIDI2_STREAM_E::EndpointInfo:
			dataOut[2] = msgIn->epInfo.verMaj;
			dataOut[3] = msgIn->epInfo.verMin;
			dataOut[4] = msgIn->epInfo.funcNum | (msgIn->epInfo.isStatic << 7);
			dataOut[6] = msgIn->epInfo.midi1 & 0b1;
			dataOut[6] |= (msgIn->epInfo.midi2 & 0b1) << 1;
			dataOut[7] = msgIn->epInfo.txJR & 0b1;
			dataOut[7] |= (msgIn->epInfo.rxJR & 0b1) << 1;
			break;
		case MIDI2_STREAM_E::DeviceID:
			dataOut[5] = msgIn->devID.sysexID & 0x7f;
			dataOut[6] = (msgIn->devID.sysexID >> 7) & 0x7f;
			dataOut[7] = (msgIn->devID.sysexID >> 14) & 0x7f;
			dataOut[8] = msgIn->devID.devFamily & 0x7f;
			dataOut[9] = (msgIn->devID.devFamily >> 7) & 0x7f;
			dataOut[10] = msgIn->devID.devModel & 0x7f;
			dataOut[11] = (msgIn->devID.devModel >> 7) & 0x7f;
			dataOut[12] = msgIn->devID.devVersion & 0x7f;
			dataOut[13] = (msgIn->devID.devVersion >> 7) & 0x7f;
			dataOut[14] = (msgIn->devID.devVersion >> 14) & 0x7f;
			dataOut[15] = (msgIn->devID.devVersion >> 21) & 0x7f;
			break;
		case MIDI2_STREAM_E::EndpointName:
		case MIDI2_STREAM_E::ProductInstance:
			for (uint8_t i = 0; i < 14; i++){
				dataOut[i+2] = msgIn->data[i];
			}
			break;
		case MIDI2_STREAM_E::ConfigReq:
		case MIDI2_STREAM_E::ConfigNotice:
			dataOut[2] = msgIn->streamCon.protocol;
			dataOut[3] = msgIn->streamCon.txJR & 0b1;
			dataOut[3] |= (msgIn->streamCon.rxJR & 0b1) << 1;
			break;
		case MIDI2_STREAM_E::FunctionDiscovery:
			dataOut[2] = msgIn->funcDiscovery.funcNum;
			dataOut[3] = msgIn->funcDiscovery.reqInfo & 0b1;
			dataOut[3] |= (msgIn->funcDiscovery.reqName & 0b1) << 1;
			break;
		case MIDI2_STREAM_E::FunctionInfo:
			dataOut[2] = msgIn->funcInfo.funcNum | (msgIn->funcInfo.isActive << 7);
			dataOut[3] = (uint8_t) msgIn->funcInfo.direction;
			dataOut[3] |= (uint8_t) msgIn->funcInfo.midiSpeed << 2;
			dataOut[3] |= (uint8_t) msgIn->funcInfo.hint << 4;
			dataOut[4] = msgIn->funcInfo.groupFirst;
			dataOut[5] = msgIn->funcInfo.groupSpan;
			dataOut[6] = msgIn->funcInfo.ciVersion;
			dataOut[7] = msgIn->funcInfo.sysexNum;
			break;
		case MIDI2_STREAM_E::FunctionName:
			dataOut[2] = msgIn->funcName.funcNum;
			for (uint8_t i = 0; i < 13; i++){
				dataOut[i+3] = msgIn->funcName.name[i];
			}
			break;
		default:
			break;
	}
	return 16;
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
			return Encode(dataOut, &msgIn->voice2, ver);
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
		for (uint8_t l = 0; l < length; l++){
			if (msgIndex == 0){
				enum MIDI_MT_E msgType;
				msgType = (MIDI_MT_E) (data[l] >> 4);
				switch (msgType){
					case MIDI_MT_E::Utility:
					case MIDI_MT_E::RealTime:
					case MIDI_MT_E::Voice1:
					case MIDI_MT_E::Reserved6:
					case MIDI_MT_E::Reserved7: msgLength = 4; break;
					case MIDI_MT_E::Data64:
					case MIDI_MT_E::Voice2:
					case MIDI_MT_E::Reserved8:
					case MIDI_MT_E::Reserved9:
					case MIDI_MT_E::Reserved10: msgLength = 8; break;
					case MIDI_MT_E::Reserved11:
					case MIDI_MT_E::Reserved12: msgLength = 12; break;
					case MIDI_MT_E::Data128:
					case MIDI_MT_E::FlexData:
					case MIDI_MT_E::Reserved14:
					case MIDI_MT_E::Stream: msgLength = 16; break;
				}
			}
			msgBuffer[msgIndex++] = data[l];
			if (msgIndex < msgLength) continue;
			msgIndex = 0;
			MIDI_UMP_t msgCurrent;
			enum MIDI_MT_E msgType;
			msgType = (MIDI_MT_E) (msgBuffer[0] >> 4);
			msgCurrent.type = msgType;
			if(msgType == MIDI_MT_E::Utility) {
				msgCurrent.util.status = (MIDI2_UTIL_E) (msgBuffer[1] >> 4);
				if (msgCurrent.util.status == MIDI2_UTIL_E::DeltaPrevTick) {
					msgCurrent.util.tickLast = ((msgBuffer[1] & 0xf) << 16) | (msgBuffer[2] << 8) | msgBuffer[3];
				} else if (msgCurrent.util.status == MIDI2_UTIL_E::JRTimestamp
						|| msgCurrent.util.status == MIDI2_UTIL_E::DeltaTPQ
						|| msgCurrent.util.status == MIDI2_UTIL_E::JRClk) {
					// Should be the same memory due to union
					msgCurrent.util.timestamp = (msgBuffer[2] << 8) | msgBuffer[3];
				}
				if (MIDI2_util_p != 0) {
					MIDI2_util_p(&msgCurrent.util);
				} else if (MIDI_UMP_p != 0)	{
					MIDI_UMP_p(&msgCurrent);
				}
			} else if (msgType == MIDI_MT_E::RealTime) {
				msgCurrent.com.group = msgBuffer[0] & 0x0f;
				if (!(groupMask & (1 << msgCurrent.com.group))) continue;	// Masked out
				msgCurrent.com.status = (MIDI2_COM_E) (msgBuffer[1] && 0x0f);
				if (msgCurrent.com.status == MIDI2_COM_E::TimeCode) {
					msgCurrent.com.timecode = msgBuffer[2];
				} else if (msgCurrent.com.status == MIDI2_COM_E::SongPosPoint) {
					msgCurrent.com.songPos = msgBuffer[2] | (msgBuffer[3] << 7);
				} else if (msgCurrent.com.status == MIDI2_COM_E::SongSel) {
					msgCurrent.com.songNum = msgBuffer[2];
				}
			
				if (MIDI2_com_p != 0) {
					MIDI2_com_p(&msgCurrent.com);
				} else if (MIDI_UMP_p != 0) {
					MIDI_UMP_p(&msgCurrent);
				}
			} else if (msgType == MIDI_MT_E::Voice1) {
				msgCurrent.voice1.group = msgBuffer[0] & 0x0f;
				msgCurrent.voice1.channel = msgBuffer[1] & 0x0f;
				if (!(groupMask & (1 << msgCurrent.voice1.group)) || !(channelMask & (1 << msgCurrent.voice1.channel))) continue;	// Masked out
				msgCurrent.voice1.status = (MIDI1_STATUS_E) (msgBuffer[1] >> 4);
				switch (msgCurrent.voice1.status){
					case MIDI1_STATUS_E::Aftertouch:
					case MIDI1_STATUS_E::NoteOn:
					case MIDI1_STATUS_E::NoteOff:
						msgCurrent.voice1.key = msgBuffer[2];
						msgCurrent.voice1.velocity = msgBuffer[3];
						break;
					case MIDI1_STATUS_E::CControl:
						msgCurrent.voice1.controller = msgBuffer[2];
						msgCurrent.voice1.val = msgBuffer[3];
						break;
					case MIDI1_STATUS_E::ProgChange:
						msgCurrent.voice1.instrument = msgBuffer[2];
						break;
					case MIDI1_STATUS_E::ChanPressure:
						msgCurrent.voice1.pressure = msgBuffer[2];
						break;
					case MIDI1_STATUS_E::Pitchbend:
						msgCurrent.voice1.bend = msgBuffer[2] | (msgBuffer[3] << 7);
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
				msgCurrent.data64.group = msgBuffer[0] & 0x0f;
				if (!(groupMask & (1 << msgCurrent.data64.group))) continue;	// Masked out
				msgCurrent.data64.status = (MIDI2_DATA64_E) (msgBuffer[1] >> 4);
				msgCurrent.data64.numBytes = msgBuffer[1] & 0x0f;
				for (uint8_t i = 0; i < (length - 2); i++) {
					if (i > 5) break;
					msgCurrent.data64.data[i] = msgBuffer[i+2];
				}
				if (MIDI2_data64_p != 0) {
					MIDI2_data64_p(&msgCurrent.data64);
				} else if (MIDI_UMP_p != 0) {
					MIDI_UMP_p(&msgCurrent);
				}
			} else if (msgType == MIDI_MT_E::Voice2) {
				msgCurrent.voice2.group = msgBuffer[0] & 0x0f;
				msgCurrent.voice2.channel = msgBuffer[1] & 0x0f;
				if (!(groupMask & (1 << msgCurrent.voice2.group)) || !(channelMask & (1 << msgCurrent.voice2.channel))) continue;	// Masked out
				msgCurrent.voice2.status = (MIDI2_VOICE_E) (msgBuffer[1] >> 4);
				switch (msgCurrent.voice2.status){
					case MIDI2_VOICE_E::RegNoteControl:
					case MIDI2_VOICE_E::AssNoteControl:
						msgCurrent.voice2.note = msgBuffer[2];
						msgCurrent.voice2.controller = msgBuffer[3];
						msgCurrent.voice2.data = (msgBuffer[4] << 24) | (msgBuffer[5] << 16) | (msgBuffer[6] << 8) | msgBuffer[7];
						break;
					case MIDI2_VOICE_E::RegControl:
					case MIDI2_VOICE_E::AssControl:
					case MIDI2_VOICE_E::RelRegControl:
					case MIDI2_VOICE_E::RelAssControl:
						msgCurrent.voice2.bankCtrl = msgBuffer[2];
						msgCurrent.voice2.index = msgBuffer[3];
						msgCurrent.voice2.data = (msgBuffer[4] << 24) | (msgBuffer[5] << 16) | (msgBuffer[6] << 8) | msgBuffer[7];
						break;
					case MIDI2_VOICE_E::NotePitchbend:
					case MIDI2_VOICE_E::Aftertouch:
						msgCurrent.voice2.note = msgBuffer[2];
						msgCurrent.voice2.data = (msgBuffer[4] << 24) | (msgBuffer[5] << 16) | (msgBuffer[6] << 8) | msgBuffer[7];
						break;
					case MIDI2_VOICE_E::NoteOff:
					case MIDI2_VOICE_E::NoteOn:
						msgCurrent.voice2.note = msgBuffer[2];
						msgCurrent.voice2.attrType = (MIDI_ATTR_E) msgBuffer[3];
						msgCurrent.voice2.velocity = (msgBuffer[4] << 8) | msgBuffer[5];
						msgCurrent.voice2.attrVal = (msgBuffer[6] << 8) | msgBuffer[7]; 
						break;
					case MIDI2_VOICE_E::CControl:
						msgCurrent.voice2.controller = msgBuffer[2];
						msgCurrent.voice2.data = (msgBuffer[4] << 24) | (msgBuffer[5] << 16) | (msgBuffer[6] << 8) | msgBuffer[7];
						break;
					case MIDI2_VOICE_E::ProgChange:
						msgCurrent.voice2.options = msgBuffer[3];
						msgCurrent.voice2.program = msgBuffer[4];
						msgCurrent.voice2.bankPC = (msgBuffer[6] << 7) | msgBuffer[7];
						if (msgCurrent.voice2.options & 0b1){
							prevBank = 0xc000 | msgCurrent.voice2.bankPC;
						}
						break;
					case MIDI2_VOICE_E::ChanPressure:
					case MIDI2_VOICE_E::Pitchbend:
					msgCurrent.voice2.data = (msgBuffer[4] << 24) | (msgBuffer[5] << 16) | (msgBuffer[6] << 8) | msgBuffer[7];
						break;
					case MIDI2_VOICE_E::NoteManage:
						msgCurrent.voice2.note = msgBuffer[2];
						msgCurrent.voice2.options = msgBuffer[3];
						break;
				}
				if (MIDI2_voice_p != 0) {
					MIDI2_voice_p(&msgCurrent.voice2);
				} else if (MIDI_UMP_p != 0) {
					MIDI_UMP_p(&msgCurrent);
				}
			} else if (msgType == MIDI_MT_E::Data128) {
				msgCurrent.data128.group = msgBuffer[0] & 0x0f;
				if (!(groupMask & (1 << msgCurrent.data128.group))) continue;	// Masked out
				msgCurrent.data128.status = (MIDI2_DATA128_E) (msgBuffer[1] >> 4);
				uint8_t off = 2;
				if ((msgCurrent.data128.status == MIDI2_DATA128_E::MixHead) || (msgCurrent.data128.status == MIDI2_DATA128_E::MixPay)) {
					msgCurrent.data128.mdsID = msgBuffer[1] & 0x0f;
					} else {
					msgCurrent.data128.numBytes = msgBuffer[1] & 0x0f;
					msgCurrent.data128.streamID = msgBuffer[2];
					off = 3;
				}
				for (uint8_t i = 0; i < (length - off); i++) {
					if (i > (15 - off)) break;
					msgCurrent.data128.data[i] = msgBuffer[i+off];
				}
				
				// Can not be converted to MIDI 1.0
				if (MIDI2_data128_p != 0){
					MIDI2_data128_p(&msgCurrent.data128);
				} else if (MIDI_UMP_p != 0) {
					MIDI_UMP_p(&msgCurrent);
				}
			} else if (msgType == MIDI_MT_E::Reserved6)	{
			} else if (msgType == MIDI_MT_E::Reserved7)	{
			} else if (msgType == MIDI_MT_E::Reserved8)	{
			} else if (msgType == MIDI_MT_E::Reserved9)	{
			} else if (msgType == MIDI_MT_E::Reserved10)	{
			} else if (msgType == MIDI_MT_E::Reserved11)	{
			} else if (msgType == MIDI_MT_E::Reserved12)	{
			} else if (msgType == MIDI_MT_E::FlexData)	{
				bool maskedOut;
				msgCurrent.flex.group = msgBuffer[0] & 0xf;
				maskedOut = !(groupMask & (1 << msgCurrent.flex.group));
				msgCurrent.flex.channel = msgBuffer[1] & 0xf;
				msgCurrent.flex.destination = (MIDI2_FLEX_ADDR_E) ((msgBuffer[1] >> 4) & 0x3);
				msgCurrent.flex.format = (MIDI2_FORMAT_E) ((msgBuffer[1] >> 6) & 0x3);
				maskedOut |= (msgCurrent.flex.destination == MIDI2_FLEX_ADDR_E::Channel) && !(channelMask & (1 << msgCurrent.flex.channel));
				if (maskedOut) continue;
				msgCurrent.flex.status = (MIDI2_FLEXDATA_E) ((msgBuffer[2] << 8) | msgBuffer[3]);
				struct {int8_t val : 4;} signedNibble;	// Make sure 4-bit value gets sign-extended		
				switch (msgCurrent.flex.status){
					case MIDI2_FLEXDATA_E::SetTempo:
						msgCurrent.flex.tempo = (msgBuffer[4] << 24) | (msgBuffer[5] << 16) | (msgBuffer[6] << 8) | msgBuffer[7];
						break;
					case MIDI2_FLEXDATA_E::SetTimeSig:
						msgCurrent.flex.timeSig.numerator = msgBuffer[4];
						msgCurrent.flex.timeSig.denominator = msgBuffer[5];
						msgCurrent.flex.timeSig.numNotes = msgBuffer[6];
						break;
					case MIDI2_FLEXDATA_E::SetMetronome:
						msgCurrent.flex.metronome.primaryClick = msgBuffer[4];
						for (uint8_t i = 0; i < 3; i++){
							msgCurrent.flex.metronome.accent[i] = msgBuffer[i+5];
						}
						for (uint8_t i = 0; i < 2; i++){
							msgCurrent.flex.metronome.subClick[i] = msgBuffer[i+8];
						}
						break;
					case MIDI2_FLEXDATA_E::SetKeySig:
						msgCurrent.flex.keySig.tonic = msgBuffer[4] & 0xf;
						signedNibble.val = msgBuffer[4] >> 4;
						msgCurrent.flex.keySig.sharps = signedNibble.val;
						break;
					case MIDI2_FLEXDATA_E::SetChord:
						msgCurrent.flex.chord.mainChord.tonic = msgBuffer[4] & 0xf;
						signedNibble.val = msgBuffer[4] >> 4;
						msgCurrent.flex.chord.mainChord.sharps = signedNibble.val;
						msgCurrent.flex.chord.mainChord.type = (CHORD_TYPE_E) msgBuffer[5];
						for (uint8_t i = 0; i < 4; i++){
							signedNibble.val = msgBuffer[i+6] & 0xf;
							msgCurrent.flex.chord.mainChord.alts[i].degree = signedNibble.val;
							msgCurrent.flex.chord.mainChord.alts[i].type = (CHORD_ALT_E) (msgBuffer[i+6] >> 4);
						}
						msgCurrent.flex.chord.bassChord.tonic = msgBuffer[12] & 0xf;
						signedNibble.val = msgBuffer[12] >> 4;
						msgCurrent.flex.chord.bassChord.sharps = signedNibble.val;
						msgCurrent.flex.chord.bassChord.type = (CHORD_TYPE_E) msgBuffer[13];
						for (uint8_t i = 0; i < 2; i++){
							signedNibble.val = msgBuffer[i+14] & 0xf;
							msgCurrent.flex.chord.bassChord.alts[i].degree = signedNibble.val;
							msgCurrent.flex.chord.bassChord.alts[i].type = (CHORD_ALT_E) (msgBuffer[i+14] >> 4);
						}
						break;
					default:
						for (uint8_t i = 0; i < 12; i++){
							msgCurrent.flex.data[i] = msgBuffer[i+4];
						}
						break;
				}
				if (MIDI2_flex_p != 0){
					MIDI2_flex_p(&msgCurrent.flex);
				} else if (MIDI_UMP_p != 0){
					MIDI_UMP_p(&msgCurrent);
				}
			} else if (msgType == MIDI_MT_E::Reserved14)	{
			} else if (msgType == MIDI_MT_E::Stream)	{
				msgCurrent.stream.status = (MIDI2_STREAM_E) ((msgBuffer[1] | msgBuffer[0] << 8) & 0x03ff);
				switch (msgCurrent.stream.status){
					case MIDI2_STREAM_E::EndpointDiscovery:
						msgCurrent.stream.epDiscovery.verMaj = msgBuffer[2];
						msgCurrent.stream.epDiscovery.verMin = msgBuffer[3];
						msgCurrent.stream.epDiscovery.reqInfo = msgBuffer[7] & 0b1;
						msgCurrent.stream.epDiscovery.reqDevID = (msgBuffer[7] >> 1) & 0b1;
						msgCurrent.stream.epDiscovery.reqName = (msgBuffer[7] >> 2) & 0b1;
						msgCurrent.stream.epDiscovery.reqInstID = (msgBuffer[7] >> 3) & 0b1;
						msgCurrent.stream.epDiscovery.reqStream = (msgBuffer[7] >> 4) & 0b1;
						break;
					case MIDI2_STREAM_E::EndpointInfo:
						msgCurrent.stream.epInfo.verMaj = msgBuffer[2];
						msgCurrent.stream.epInfo.verMin = msgBuffer[3];
						msgCurrent.stream.epInfo.isStatic = msgBuffer[4] >> 7;
						msgCurrent.stream.epInfo.funcNum = msgBuffer[4] & 0x7f;
						msgCurrent.stream.epInfo.midi1 = msgBuffer[6] & 0b1;
						msgCurrent.stream.epInfo.midi2 = (msgBuffer[6] >> 1) & 0b1;
						msgCurrent.stream.epInfo.txJR = msgBuffer[7] & 0b1;
						msgCurrent.stream.epInfo.rxJR = (msgBuffer[7] >> 1) & 0b1;
						break;
					case MIDI2_STREAM_E::DeviceID:
						msgCurrent.stream.devID.sysexID = (msgBuffer[5] | (msgBuffer[6] << 7) | (msgBuffer[7] << 14)) & 0x1fffff;
						msgCurrent.stream.devID.devFamily = (msgBuffer[8] | (msgBuffer[9] << 7)) & 0x3fff;
						msgCurrent.stream.devID.devModel = (msgBuffer[10] | (msgBuffer[11] << 7)) & 0x3fff;
						msgCurrent.stream.devID.devVersion = (msgBuffer[12] | (msgBuffer[13] << 7) | (msgBuffer[14] << 14) | (msgBuffer[15] << 21)) & 0x0fffffff;
						break;
					case MIDI2_STREAM_E::EndpointName:
					case MIDI2_STREAM_E::ProductInstance:
						for (uint8_t i = 0; i < 14; i++){
							msgCurrent.stream.data[i] = msgBuffer[i+2];
						}
						break;
					case MIDI2_STREAM_E::ConfigReq:
					case MIDI2_STREAM_E::ConfigNotice:
						msgCurrent.stream.streamCon.protocol = msgBuffer[2];
						msgCurrent.stream.streamCon.rxJR = (msgBuffer[3] >> 1) & 0b1;
						msgCurrent.stream.streamCon.txJR = msgBuffer[3] & 0b1;
						break;
					case MIDI2_STREAM_E::FunctionDiscovery:
						msgCurrent.stream.funcDiscovery.funcNum = msgBuffer[2];
						msgCurrent.stream.funcDiscovery.reqInfo = msgBuffer[3] & 0b1;
						msgCurrent.stream.funcDiscovery.reqName = (msgBuffer[3] >> 1) & 0b1;
						break;
					case MIDI2_STREAM_E::FunctionInfo:
						msgCurrent.stream.funcInfo.isActive = msgBuffer[2] >> 7;
						msgCurrent.stream.funcInfo.funcNum = msgBuffer[2] & 0x7f;
						msgCurrent.stream.funcInfo.hint = (MIDI_DIR_E) ((msgBuffer[3] >> 4) & 0b11);
						msgCurrent.stream.funcInfo.midiSpeed = (MIDI1_BLOCK_E) ((msgBuffer[3] >> 2) & 0b11);
						msgCurrent.stream.funcInfo.direction = (MIDI_DIR_E) (msgBuffer[3] & 0b11);
						msgCurrent.stream.funcInfo.groupFirst = msgBuffer[4];
						msgCurrent.stream.funcInfo.groupSpan = msgBuffer[5];
						msgCurrent.stream.funcInfo.ciVersion = msgBuffer[6];
						msgCurrent.stream.funcInfo.sysexNum = msgBuffer[7];
						break;
					case MIDI2_STREAM_E::FunctionName:
						msgCurrent.stream.funcName.funcNum = msgBuffer[2];
						for (uint8_t i = 0; i < 13; i++){
							msgCurrent.stream.funcName.name[i] = msgBuffer[i+3];
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
						if (tempMsg.controller == 0){
							prevBank &= 0xc07f;	// Delete MSB
							prevBank |= 0xc000;
							prevBank |= tempMsg.val << 7;
						} else if (tempMsg.controller == 32){
							prevBank &= 0xff80;	// Delete LSB
							prevBank |= 0xc000;
							prevBank |= tempMsg.val;
						}
					}
					break;
				case MIDI1_STATUS_E::ProgChange:
					tempMsg.instrument = msgBuffer[1];
					if (MIDI2_voice_p != 0) {
						MIDI2_voice_t tempMsg2;
						Convert(&tempMsg2, &tempMsg);
						bool bankValid = prevBank >> 15;
						tempMsg2.options |= bankValid;
						tempMsg2.bankPC = (prevBank & 0x3fff) * bankValid;
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
