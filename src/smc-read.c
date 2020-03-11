/*
MIT License

Copyright (c) 2020 Frank Stock

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "smc-read.h"
#include <string.h>
#include <math.h>
#include <arpa/inet.h>

#pragma ide diagnostic push
// I checked all the warnings about signed bitwise usage in this file, they are all clean so best to keep CLion happy :-)
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

/**
 * Most documentation on SMC keys are described in string format, even though the SMC itself considers keys to be 32 bit unsigned integers.
 * This function translates from a null terminated "C" string to a 32 bit unsigned integer that the SMC will recognize.
 */
uint32_t stringToKey(const char* str) {
	return ntohl(*(uint32_t*) str);
}

/**
 * Convert a SMC key to a human readable string.
 * @see stringToKey
 */
void keyToString(uint32_t key, char* str) {
	*((uint32_t*) str) = htonl(key);
	str[4] = 0;
}

// See header for documentation
IOReturn AppleSMCOpen(io_connect_t* conn) {
	if (conn == NULL)
		return kIOReturnInvalid;
	io_iterator_t existing;
	*conn = 0;
	// Create a matching dictionary that specifies an IOService class match.
	CFMutableDictionaryRef matching = IOServiceMatching("AppleSMC");
	// Look up registered IOService objects that match a matching dictionary.
	IOReturn result = IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &existing);
	if (result != kIOReturnSuccess)
		return result;
	// Returns the next object in an iteration.
	io_object_t service = IOIteratorNext(existing);
	// Releases an object handle previously returned by IOKitLib.
	IOObjectRelease(existing);
	if (service == 0)
		return kIOReturnNotFound;
	// A request to create a connection to an IOService.
	result = IOServiceOpen(service, mach_task_self(), 0, conn);
	// Releases an object handle previously returned by IOKitLib.
	IOObjectRelease(service);
	return result;
}

// See header for documentation
IOReturn AppleSMCClose(io_connect_t conn) {
	// Close a connection to an IOService and destroy the connect handle.
	return IOServiceClose(conn);
}

// See header for documentation
IOReturn AppleSMCReadBuffer(io_connect_t conn, const char* key, uint32_t* dataType, SMCBytes_t buff, uint8_t* buffLen) {
	SMCKeyData inputStructure;
	SMCKeyData outputStructure;

	// We *probably* don't care that there is stale data on our stack, but I've found that the SMC_CMD_READ_KEYINFO command is looking at more than just 'key' and 'data8'.
	memset(&inputStructure, 0, sizeof(SMCKeyData));
	memset(&outputStructure, 0, sizeof(SMCKeyData));

	// Send a command to retrieve information about the key (specifically it's dataType and size).
	inputStructure.key = stringToKey(key);
	inputStructure.data8 = SMC_CMD_READ_KEYINFO;
	size_t structureOutputSize = sizeof(SMCKeyData);
	IOReturn result = IOConnectCallStructMethod(conn, KERNEL_INDEX_SMC, &inputStructure, sizeof(SMCKeyData), &outputStructure, &structureOutputSize);
	if (result != kIOReturnSuccess)
		return result;
	if (dataType != NULL)
		*dataType = outputStructure.keyInfo.dataType;
	if (buffLen != NULL)
		*buffLen = outputStructure.keyInfo.dataSize;

	// Send another command to retrieve the actual key value.
	inputStructure.keyInfo.dataSize = outputStructure.keyInfo.dataSize;
	inputStructure.data8 = SMC_CMD_READ_BYTES;
	result = IOConnectCallStructMethod(conn, KERNEL_INDEX_SMC, &inputStructure, sizeof(SMCKeyData), &outputStructure, &structureOutputSize);
	if (result != kIOReturnSuccess)
		return result;

	// Copy the key value into the buffer supplied by our caller.
	memcpy(buff, outputStructure.bytes, sizeof(outputStructure.bytes));

	return kIOReturnSuccess;
}

// See header for documentation
IOReturn AppleSMCGetKeyMetaInfo(io_connect_t conn, const char* key, SMCKeyMetaData* meta) {
	SMCKeyData inputStructure;
	SMCKeyData outputStructure;

	// We *probably* don't care that there is stale data on our stack, but I've found that the SMC_CMD_READ_KEYINFO command is looking at more than just 'key' and 'data8'.
	memset(&inputStructure, 0, sizeof(SMCKeyData));
	memset(&outputStructure, 0, sizeof(SMCKeyData));

	// Send a command to retrieve information about the key (specifically it's dataType and size).
	inputStructure.key = stringToKey(key);
	inputStructure.data8 = SMC_CMD_READ_KEYINFO;
	size_t structureOutputSize = sizeof(SMCKeyData);
	IOReturn result = IOConnectCallStructMethod(conn, KERNEL_INDEX_SMC, &inputStructure, sizeof(SMCKeyData), &outputStructure, &structureOutputSize);
	if (result != kIOReturnSuccess)
		return result;
	if (meta != NULL)
		memcpy(meta, &outputStructure.keyInfo, sizeof(SMCKeyMetaData));

	return kIOReturnSuccess;
}

// See header for documentation
float ToSMCFloat(uint32_t dataType, uint16_t value) {
	float result = value;
	switch (dataType) {
		case DATATYPE_FP1F_KEY:
			result /= 32768.0f;
			break;
		case DATATYPE_FP4C_KEY:
			result /= 4096.0f;
			break;
		case DATATYPE_FP5B_KEY:
			result /= 2048.0f;
			break;
		case DATATYPE_FP6A_KEY:
			result /= 1024.0f;
			break;
		case DATATYPE_FP79_KEY:
			result /= 512.0f;
			break;
		case DATATYPE_FP88_KEY:
			result /= 256.0f;
			break;
		case DATATYPE_FPA6_KEY:
			result /= 64.0f;
			break;
		case DATATYPE_FPC4_KEY:
			result /= 16.0f;
			break;
		case DATATYPE_FPE2_KEY:
			result /= 4.0f;
			break;

		case DATATYPE_SP1E_KEY:
			result /= 16384.0f;
			break;
		case DATATYPE_SP3C_KEY:
			result /= 4096.0f;
			break;
		case DATATYPE_SP4B_KEY:
			result /= 2048.0f;
			break;
		case DATATYPE_SP5A_KEY:
			result /= 1024.0f;
			break;
		case DATATYPE_SP69_KEY:
			result /= 512.0f;
			break;
		case DATATYPE_SP78_KEY:
			result /= 256.0f;
			break;
		case DATATYPE_SP87_KEY:
			result /= 128.0f;
			break;
		case DATATYPE_SP96_KEY:
			result /= 64.0f;
			break;
		case DATATYPE_SPB4_KEY:
			result /= 16.0f;
			break;
		case DATATYPE_SPF0_KEY:
			// is / 1.0;
			break;

		case DATATYPE_PWM_KEY:
			result /= 655.36f;
			break;

		default:
			// Oops!
			result = NAN;
	}
	return result;
}

// See header for documentation
double ToSMCNumber(uint32_t dataType, const SMCBytes_t buf, uint8_t bufLen) {
	switch (dataType) {
		case DATATYPE_HEX_KEY:
			// Hex keys vary in length, but 1,2,4,8 are just numbers.
			switch (bufLen) {
				case 1:
					return *((uint8_t*) buf);
				case 2:
					return ntohs(*((uint16_t*) buf));
				case 4:
					return ntohl(*((uint32_t*) buf));
				default:
					return NAN;
			}
		case DATATYPE_UINT8_KEY:
		case DATATYPE_FLAG_KEY:
			if (bufLen != 1)
				return NAN;
			return *((uint8_t*) buf);
		case DATATYPE_SI8_KEY:
			if (bufLen != 1)
				return NAN;
			return *((int8_t*) buf);
		case DATATYPE_UINT16_KEY:
			if (bufLen != 2)
				return NAN;
			return ntohs(*((uint16_t*) buf));
		case DATATYPE_SI16_KEY:
			if (bufLen != 2)
				return NAN;
			return ntohs(*((int16_t*) buf));
		case DATATYPE_UINT32_KEY:
			if (bufLen != 4)
				return NAN;
			return ntohl(*((uint32_t*) buf));
		default:
			return ToSMCFloat(dataType, ntohs(*((uint16_t*) buf)));
	}
}

// See header for documentation
IOReturn AppleSMCReadNumber(io_connect_t conn, const char* key, double* value) {
	SMCBytes_t buf;
	uint8_t bufLen;
	uint32_t dataType;
	// Read the key into a buffer
	IOReturn result = AppleSMCReadBuffer(conn, key, &dataType, buf, &bufLen);
	if (result != kIOReturnSuccess)
		return result;
	if (value != NULL)
		*value = ToSMCNumber(dataType, buf, bufLen);
	return kIOReturnSuccess;
}

// See header for documentation
const char* const AppleSMCErrorToString(IOReturn error) { // NOLINT(readability-const-return-type)
	switch (err_get_code(error)) {
		case err_get_code(kIOReturnSuccess):
			return "success";
		case err_get_code(kIOReturnError):
			return "general error";
		case err_get_code(kIOReturnNoMemory):
			return "memory allocation error";
		case err_get_code(kIOReturnNoResources):
			return "resource shortage";
		case err_get_code(kIOReturnIPCError):
			return "Mach IPC failure";
		case err_get_code(kIOReturnNoDevice):
			return "no such device";
		case err_get_code(kIOReturnNotPrivileged):
			return "privilege violation";
		case err_get_code(kIOReturnBadArgument):
			return "invalid argument";
		case err_get_code(kIOReturnLockedRead):
			return "device is read locked";
		case err_get_code(kIOReturnLockedWrite):
			return "device is write locked";
		case err_get_code(kIOReturnExclusiveAccess):
			return "device is exclusive access";
		case err_get_code(kIOReturnBadMessageID):
			return "bad IPC message ID";
		case err_get_code(kIOReturnUnsupported):
			return "unsupported function";
		case err_get_code(kIOReturnVMError):
			return "virtual memory error";
		case err_get_code(kIOReturnInternalError):
			return "internal driver error";
		case err_get_code(kIOReturnIOError):
			return "I/O error";
		case err_get_code(kIOReturnCannotLock):
			return "cannot acquire lock";
		case err_get_code(kIOReturnNotOpen):
			return "device is not open";
		case err_get_code(kIOReturnNotReadable):
			return "device is not readable";
		case err_get_code(kIOReturnNotWritable):
			return "device is not writeable";
		case err_get_code(kIOReturnNotAligned):
			return "alignment error";
		case err_get_code(kIOReturnBadMedia):
			return "media error";
		case err_get_code(kIOReturnStillOpen):
			return "device is still open";
		case err_get_code(kIOReturnRLDError):
			return "rld failure";
		case err_get_code(kIOReturnDMAError):
			return "DMA failure";
		case err_get_code(kIOReturnBusy):
			return "device is busy";
		case err_get_code(kIOReturnTimeout):
			return "I/O timeout";
		case err_get_code(kIOReturnOffline):
			return "device is offline";
		case err_get_code(kIOReturnNotReady):
			return "device is not ready";
		case err_get_code(kIOReturnNotAttached):
			return "device/channel is not attached";
		case err_get_code(kIOReturnNoChannels):
			return "no DMA channels available";
		case err_get_code(kIOReturnNoSpace):
			return "no space for data";
		case err_get_code(kIOReturnPortExists):
			return "device port already exists";
		case err_get_code(kIOReturnCannotWire):
			return "cannot wire physical memory";
		case err_get_code(kIOReturnNoInterrupt):
			return "no interrupt attached";
		case err_get_code(kIOReturnNoFrames):
			return "no DMA frames enqueued";
		case err_get_code(kIOReturnMessageTooLarge):
			return "message is too large";
		case err_get_code(kIOReturnNotPermitted):
			return "operation is not permitted";
		case err_get_code(kIOReturnNoPower):
			return "device is without power";
		case err_get_code(kIOReturnNoMedia):
			return "media is not present";
		case err_get_code(kIOReturnUnformattedMedia):
			return "media is not formatted";
		case err_get_code(kIOReturnUnsupportedMode):
			return "unsupported mode";
		case err_get_code(kIOReturnUnderrun):
			return "data underrun";
		case err_get_code(kIOReturnOverrun):
			return "data overrun";
		case err_get_code(kIOReturnDeviceError):
			return "device error";
		case err_get_code(kIOReturnNoCompletion):
			return "no completion routine";
		case err_get_code(kIOReturnAborted):
			return "operation was aborted";
		case err_get_code(kIOReturnNoBandwidth):
			return "bus bandwidth would be exceeded";
		case err_get_code(kIOReturnNotResponding):
			return "device is not responding";
		case err_get_code(kIOReturnInvalid):
			return "unanticipated driver error";
	}
	return "unknown error";
}

#pragma ide diagnostic pop
