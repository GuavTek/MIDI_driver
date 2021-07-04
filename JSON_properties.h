/*
 * JSON_properties.h
 *
 * Created: 30/06/2021 20:47:43
 *  Author: GuavTek
 */ 


#ifndef JSON_PROPERTIES_H_
#define JSON_PROPERTIES_H_

const char header_resource[] = '{"resource":"%s"}';
const char header_status[] = '{"status":%d}';
const char resource_resList[] = 'ResourceList';
const char resource_devInfo[] = 'DeviceInfo';
const char resource_chanList[] = 'ChannelList';
const char resource_CMList[] = 'CMList';
const char prop_manufactID[] = '"manufacturerId":[%d,%d,%d]';
const char prop_manufacturer[] = '"manufacturer":"%s"';
const char prop_famID[] = '"familyId":[%d,%d]';
const char prop_family[] = '"family":"%s"';
const char prop_modelID[] = '"modelId":[%d,%d]';
const char prop_model[] = '"model":"%s"';
const char prop_verID[] = '"versionId":[%d,%d,%d,%d]';
const char prop_version[] = '"version":"%s"';
const char prop_title[] = '"title":"%s"';
const char prop_channel[] = '"channel":%d';
const char prop_links[] = '"links":[%s]';
const char prop_name[] = '"name":"%s"';
const char prop_priority[] = '"priority":%d';
const char prop_ctrlType[] = '"controlType":"%s"';
const char prop_ctrlID[] = '"controlIdx":[%d]';
const char prop_default[] = '"default":%d';
const char prop_bankPC[] = '"bankPC":[%d,%d,%d]';
const char prop_desc[] = '"description":"%s"';

enum class MIDI_CI_STATUS_E {
	Success = 200,
	AcceptedBusy = 202,
	ErrorUnavailable = 341,
	BadData = 342,
	TooManyRequest = 343,
	BadRequest = 400,
	NotAuthorized = 403,
	ResourceNotFound = 404,
	ResourceNotAllowed = 405,
	PayloadTooLarge = 413,
	UnsupportedType = 415,
	InvalidDataVersion = 445,
	InternalDeviceError = 500
};

enum class MIDI_CI_MSG_TYPE_E {
	InqPECapability = 0x30,
	RepPECapability = 0x31,
	InqHasPropData = 0x32,
	RepHasPropData = 0x33,
	InqGetPropData = 0x34,
	RepGetPropData = 0x35,
	InqSetPropData = 0x36,
	RepSetPropData = 0x37,
	InqSubscription = 0x38,
	RepSubscription = 0x39,
	Notify = 0x3f,
	InqDiscovery = 0x70,
	RepDiscovery = 0x71,
	InvalidateMUID = 0x72,
	NAK = 0x7f
};

#endif /* JSON_PROPERTIES_H_ */