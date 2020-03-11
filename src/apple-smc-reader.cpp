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

#include "apple-smc-reader.h"
#include <system_error>
#include <cmath>

#pragma ide diagnostic push
// I checked all the warnings about signed bitwise usage in this file, they are all clean so best to keep CLion happy :-)
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

/**
 * Std cpp error_category for the IOKit domain.
 */
class IOKitErrorCategory : public std::error_category {
	const char* name() const noexcept override;
	std::string message(int ev) const override;
};
const char* IOKitErrorCategory::name() const noexcept {
	return "iokit";
}
std::string IOKitErrorCategory::message(int ev) const {
	return AppleSMCErrorToString(ev);
}
const IOKitErrorCategory ioKitErrCategory;

std::error_code make_error_code(IOReturn e) {
	return {static_cast<int>(e), ioKitErrCategory};
}

// See header for documentation
AppleSMCReader::AppleSMCReader() : conn(0) {
	IOReturn result = AppleSMCOpen(&this->conn);
	if (result != kIOReturnSuccess)
		throw std::system_error(make_error_code(result));
}

// See header for documentation
AppleSMCReader::~AppleSMCReader() {
	AppleSMCClose(this->conn);  // We encapsulate the connection.  Failures are ignored since this is a destructor *and* if we were destructed, this should'nt ever fail.
}

// See header for documentation
double AppleSMCReader::readNumber(const char* key) {
	double retVal;
	IOReturn result = AppleSMCReadNumber(this->conn, key, &retVal);
	if (result != kIOReturnSuccess)
		throw std::system_error(make_error_code(result));
	return retVal;
}

// See header for documentation
uint8_t AppleSMCReader::readUInt8(const char* key) {
	SMCBytes_t buf;
	uint8_t bufLen;
	uint32_t dataType;
	IOReturn result = AppleSMCReadBuffer(this->conn, key, &dataType, buf, &bufLen);
	if (result != kIOReturnSuccess)
		throw std::system_error(make_error_code(result));
	if (dataType != DATATYPE_UINT8_KEY && (!(dataType == DATATYPE_HEX_KEY && bufLen == 1)))
		throw std::system_error(make_error_code(kIOReturnBadArgument));
	return *reinterpret_cast<uint8_t*>(buf);
}

// See header for documentation
int8_t AppleSMCReader::readInt8(const char* key) {
	SMCBytes_t buf;
	uint8_t bufLen;
	uint32_t dataType;
	IOReturn result = AppleSMCReadBuffer(this->conn, key, &dataType, buf, &bufLen);
	if (result != kIOReturnSuccess)
		throw std::system_error(make_error_code(result));
	if (dataType != DATATYPE_SI8_KEY && (!(dataType == DATATYPE_HEX_KEY && bufLen == 1)))
		throw std::system_error(make_error_code(kIOReturnBadArgument));
	return *reinterpret_cast<int8_t*>(buf);
}

// See header for documentation
uint16_t AppleSMCReader::readUInt16(const char* key) {
	SMCBytes_t buf;
	uint8_t bufLen;
	uint32_t dataType;
	IOReturn result = AppleSMCReadBuffer(this->conn, key, &dataType, buf, &bufLen);
	if (result != kIOReturnSuccess)
		throw std::system_error(make_error_code(result));
	if (dataType != DATATYPE_UINT16_KEY && (!(dataType == DATATYPE_HEX_KEY && bufLen == 2)))
		throw std::system_error(make_error_code(kIOReturnBadArgument));
	return ntohs(*reinterpret_cast<uint16_t*>(buf));
}

// See header for documentation
int16_t AppleSMCReader::readInt16(const char* key) {
	SMCBytes_t buf;
	uint8_t bufLen;
	uint32_t dataType;
	IOReturn result = AppleSMCReadBuffer(this->conn, key, &dataType, buf, &bufLen);
	if (result != kIOReturnSuccess)
		throw std::system_error(make_error_code(result));
	if (dataType != DATATYPE_SI16_KEY && (!(dataType == DATATYPE_HEX_KEY && bufLen == 2)))
		throw std::system_error(make_error_code(kIOReturnBadArgument));
	return ntohs(*reinterpret_cast<int16_t*>(buf));
}

// See header for documentation
uint32_t AppleSMCReader::readUInt32(const char* key) {
	SMCBytes_t buf;
	uint8_t bufLen;
	uint32_t dataType;
	IOReturn result = AppleSMCReadBuffer(this->conn, key, &dataType, buf, &bufLen);
	if (result != kIOReturnSuccess)
		throw std::system_error(make_error_code(result));
	if (dataType != DATATYPE_UINT32_KEY && (!(dataType == DATATYPE_HEX_KEY && bufLen == 4)))
		throw std::system_error(make_error_code(kIOReturnBadArgument));
	return ntohl(*reinterpret_cast<uint32_t*>(buf));
}

// See header for documentation
float AppleSMCReader::readFloat(const char* key) {
	SMCBytes_t buf;
	uint8_t bufLen;
	uint32_t dataType;
	IOReturn result = AppleSMCReadBuffer(this->conn, key, &dataType, buf, &bufLen);
	if (result != kIOReturnSuccess)
		throw std::system_error(make_error_code(result));
	if (bufLen != 2)
		throw std::system_error(make_error_code(kIOReturnBadArgument));
	return ToSMCFloat(dataType, ntohs(*reinterpret_cast<uint16_t*>(buf)));
}

// See header for documentation
void AppleSMCReader::getKeyMetaInfo(const char* key, SMCKeyMetaData& meta) {
	IOReturn result = AppleSMCGetKeyMetaInfo(this->conn, key, &meta);
	if (result != kIOReturnSuccess)
		throw std::system_error(make_error_code(result));
}

// See header for documentation
std::vector<std::pair<std::string, double>> AppleSMCReader::allKeyValues() {
	std::vector<std::pair<std::string, double>> retVal;
	SMCKeyData inputStructure;
	SMCKeyData outputStructure;
	char keyBuf[5];
	memset(&inputStructure, 0, sizeof(SMCKeyData));
	memset(&outputStructure, 0, sizeof(SMCKeyData));

	// Ask the SMC how many keys it knows about.
	int totalKeys = this->readUInt32("#KEY");
	for (int i = 0; i < totalKeys; i++) {
		IOReturn result;
		// read the name of the key we're looking for, by its ID (aka index).
		inputStructure.data8 = SMC_CMD_READ_INDEX;
		inputStructure.data32 = i;
		size_t structureOutputSize = sizeof(SMCKeyData);
		result = IOConnectCallStructMethod(this->conn, KERNEL_INDEX_SMC, &inputStructure, sizeof(SMCKeyData), &outputStructure, &structureOutputSize);
		if (result != kIOReturnSuccess)
			continue;
		// Convert the integer to human readable key.
		keyToString(outputStructure.key, keyBuf);
		// Retrieve the value of the key.
		double value;
		result = AppleSMCReadNumber(this->conn, keyBuf, &value);
		if (result != kIOReturnSuccess)
			value = NAN;
		// Keep track of the key/value pair.
		retVal.emplace_back(std::make_pair(keyBuf, value));
	}
	return retVal;
}

#pragma ide diagnostic pop
