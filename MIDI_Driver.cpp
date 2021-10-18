/*
 * MIDI_Driver.cpp
 *
 * Created: 26/06/2021 14:39:29
 *  Author: GuavTek
 */ 

#include "MIDI_Driver.h"

void MIDI_C::Convert (struct MIDI1_msg_t *msgOut, struct MIDI2_voice_t *msgIn){
	switch(msgIn->status){
		case MIDI2_VOICE_E::RegControl:
			
			break;
		case MIDI2_VOICE_E::AssControl:
		
			break;
		case MIDI2_VOICE_E::NoteOff:
		
			break;
		case MIDI2_VOICE_E::NoteOn:
		
			break;
		case MIDI2_VOICE_E::Aftertouch:
			
			break;
		case MIDI2_VOICE_E::CControl:
			
			break;
		case MIDI2_VOICE_E::ProgChange:
			
			break;
		case MIDI2_VOICE_E::ChanPressure:
			
			break;
		case MIDI2_VOICE_E::Pitchbend:
			
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
			msgOut->velocity = msgIn->velocity;
			break;
		case MIDI1_STATUS_E::Aftertouch:
			msgOut->note = msgIn->key;
			msgOut->data = msgIn->velocity; // Needs massaging
			break;
		case MIDI1_STATUS_E::CControl:
			msgOut->controller = msgIn->controller;
			msgOut->data = msgIn->val;
			break;
		case MIDI1_STATUS_E::ProgChange:
			msgOut->program = msgIn->instrument;
			break;
		case MIDI1_STATUS_E::ChanPressure:
			msgOut->data = msgIn->pressure; // Needs conversion
			break;
		case MIDI1_STATUS_E::Pitchbend:
			msgOut->data = msgIn->bend; // Note: needs to be converted
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
		dataOut[i++] = ((uint8_t) msgIn->status << 4) | (msgIn->numBytes 0x0f);
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
	dataOut[0] = (((uint8_t) MIDI_MT_E::Utility) << 4) | (msgIn->group & 0x0f);
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

void MIDI_C::Decode (char* data, uint8_t length){	
	if (MIDIVersion == 2) {
		// MIDI 2.0 decoder
		enum MIDI_MT_E msgType;
		msgType = (MIDI_MT_E) (data[0] >> 4);
		
		if(msgType == MIDI_MT_E::Utility) {
			if (MIDI2_util_p != 0) {
				MIDI2_util_t msgCurrent;
				msgCurrent.group = data[0] & 0x0f;
				msgCurrent.status = (MIDI2_UTIL_E) (data[1] >> 4);
				if (msgCurrent.status == MIDI2_UTIL_E::JRClk) {
					msgCurrent.clk = (data[2] << 8) | data[3];
				} else if (msgCurrent.status == MIDI2_UTIL_E::JRTimestamp) {
					msgCurrent.timestamp = (data[2] << 8) | data[3];
				}
				MIDI2_util_p(&msgCurrent);
			}
		} else if (msgType == MIDI_MT_E::RealTime) {
			MIDI2_com_t msgCurrent;
			msgCurrent.group = data[0] & 0x0f;
			msgCurrent.status = (MIDI2_COM_E) (data[1] && 0x0f);
			if (msgCurrent.status == MIDI2_COM_E::TimeCode) {
				msgCurrent.timecode = data[2];
			} else if (msgCurrent.status == MIDI2_COM_E::SongPosPoint) {
				msgCurrent.songPos = data[2] | (data[3] << 7);
			} else if (msgCurrent.status == MIDI2_COM_E::SongSel) {
				msgCurrent.songNum = data[2];
			}
			
			if (MIDI2_com_p != 0) {
				MIDI2_com_p(&msgCurrent);
			} else if (MIDI1_p != 0) {
				// Convert?
			}
		} else if (msgType == MIDI_MT_E::Voice1) {
			MIDI1_msg_t msgCurrent;
			msgCurrent.group = data[0] & 0x0f;
			msgCurrent.status = (MIDI1_STATUS_E) (data[1] >> 4);
			msgCurrent.channel = data[1] & 0x0f;
			switch (msgCurrent.status){
				case MIDI1_STATUS_E::Aftertouch:
				case MIDI1_STATUS_E::NoteOn:
				case MIDI1_STATUS_E::NoteOff:
					msgCurrent.key = data[2];
					msgCurrent.velocity = data[3];
					break;
				case MIDI1_STATUS_E::CControl:
					msgCurrent.controller = data[2];
					msgCurrent.val = data[3];
					break;
				case MIDI1_STATUS_E::ProgChange:
					msgCurrent.instrument = data[2];
					break;
				case MIDI1_STATUS_E::ChanPressure:
					msgCurrent.pressure = data[2];
					break;
				case MIDI1_STATUS_E::Pitchbend:
					msgCurrent.bend = data[2] | (data[3] << 7);
					break;
				default:
					msgCurrent.status = MIDI1_STATUS_E::Invalid;
					break;
			}
			if (msgCurrent.status == MIDI1_STATUS_E::Invalid) {
				return;
			} else if (MIDI1_p != 0) {
				MIDI1_p(&msgCurrent);
			} else if (MIDI2_voice_p != 0) {
				// Convert?
			}
		} else if (msgType == MIDI_MT_E::Data64) {
			MIDI2_data64_t msgCurrent;
			msgCurrent.group = data[0] & 0x0f;
			msgCurrent.status = (MIDI2_DATA64_E) (data[1] >> 4);
			msgCurrent.numBytes = data[1] & 0x0f;
			for (uint8_t i = 0; i < (length - 2); i++) {
				if (i > 5) break;
				msgCurrent.data[i] = data[i+2];
			}
			if (MIDI2_data64_p != 0) {
				MIDI2_data64_p(&msgCurrent);
			} 
			/*else if (MIDI1_p != 0) {
				MIDI1_msg_t tempMsg;
				//Convert(&tempMsg, &msgCurrent);
				MIDI1_p(&tempMsg);
			} 
			/**/
		} else if (msgType == MIDI_MT_E::Voice2) {
			MIDI2_voice_t msgCurrent;
			msgCurrent.group = data[0] & 0x0f;
			msgCurrent.status = (MIDI2_VOICE_E) (data[1] >> 4);
			msgCurrent.channel = data[1] & 0x0f;
			switch (msgCurrent.status){
				case MIDI2_VOICE_E::RegNoteControl:
				case MIDI2_VOICE_E::AssNoteControl:
					msgCurrent.note = data[2];
					msgCurrent.controller = data[3];
					msgCurrent.data = (uint32_t) data[4];
					break;
				case MIDI2_VOICE_E::RegControl:
				case MIDI2_VOICE_E::AssControl:
				case MIDI2_VOICE_E::RelRegControl:
				case MIDI2_VOICE_E::RelAssControl:
					msgCurrent.bankCtrl = data[2];
					msgCurrent.index = data[3];
					msgCurrent.data = (uint32_t) data[4];
					break;
				case MIDI2_VOICE_E::NotePitchbend:
				case MIDI2_VOICE_E::Aftertouch:
					msgCurrent.note = data[2];
					msgCurrent.data = (uint32_t) data[4];
					break;
				case MIDI2_VOICE_E::NoteOff:
				case MIDI2_VOICE_E::NoteOn:
					msgCurrent.note = data[2];
					msgCurrent.attrType = (MIDI_ATTR_E) data[3];
					msgCurrent.velocity = (uint16_t) data[4];
					msgCurrent.attrVal = (uint16_t) data[6]; 
					break;
				case MIDI2_VOICE_E::CControl:
					msgCurrent.controller = data[2];
					msgCurrent.data = (uint16_t) data[4];
					break;
				case MIDI2_VOICE_E::ProgChange:
					msgCurrent.options = data[3];
					msgCurrent.program = data[4];
					msgCurrent.bankPC = (data[6] << 7) | data[7];
					break;
				case MIDI2_VOICE_E::ChanPressure:
				case MIDI2_VOICE_E::Pitchbend:
					msgCurrent.data = (uint32_t) data[4];
					break;
				case MIDI2_VOICE_E::NoteManage:
					msgCurrent.note = data[2];
					msgCurrent.options = data[3];
					break;
			}
			if (MIDI2_voice_p != 0) {
				MIDI2_voice_p(&msgCurrent);
			} else if (MIDI1_p != 0) {
				// Convert?
			}
		} else if (msgType == MIDI_MT_E::Data128) {
			if (MIDI2_data128_p != 0) {
				MIDI2_data128_t msgCurrent;
				msgCurrent.group = data[0] & 0x0f;
				msgCurrent.status = (MIDI2_DATA128_E) (data[1] >> 4);
				uint8_t off = 2;
				if ((msgCurrent.status == MIDI2_DATA128_E::MixHead) || (msgCurrent.status == MIDI2_DATA128_E::MixPay)) {
					msgCurrent.mdsID = data[1] & 0x0f;
					} else {
					msgCurrent.numBytes = data[1] & 0x0f;
					msgCurrent.streamID = data[2];
					off = 3;
				}
				for (uint8_t i = 0; i < (length - off); i++) {
					if (i > (15 - off)) break;
					msgCurrent.data[i] = data[i+off];
				}
				
				// Can not be converted to MIDI 1.0
				MIDI2_data128_p(&msgCurrent);
			}
		}
	} else {
		// MIDI 1.0 decoder
		for (uint8_t i = 0; i < length; i++) {
			msgBuffer[msgIndex] = data[i];
			if (data[i] > 127) {
				// New command
				msgIndex = 1;
				MIDI1_STATUS_E command = (MIDI1_STATUS_E) (*data >> 4);
				switch (command) {
					case MIDI1_STATUS_E::SysEx:
						if (*data == 0xf0) {
							// Sysex start
							msgLength = 8;
							decodeState = SysexMIDI1;
						} else if (*data == 0xf7) {
							// Sysex end
							msgLength = msgIndex;
							//decodeState = NormalMIDI1;
						} else if ((*data & 0x0f) == 2) {
							msgLength = 3;
							decodeState = NormalMIDI1;
						} else if ((*data & 0x0f) == 3) {
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
			if (msgIndex == msgLength) {
				// Decode buffered message
				MIDI1_msg_t tempMsg;
				tempMsg.channel = msgBuffer[0] & 0x0f;
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
						}
					} else if (tempMsg.channel == 7) {
						MIDI2_data64_t msgData;
						tempMsg.status = MIDI1_STATUS_E::Invalid;
						msgData.status = MIDI2_DATA64_E::End;
						msgData.numBytes = 0;
						decodeState = NormalMIDI1;
						if (MIDI2_data64_p != 0) {
							MIDI2_data64_p(&msgData);
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
				}
			} else if (msgIndex < msgLength) {
				msgIndex++;
			}
		}
	}
}
