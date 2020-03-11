#pragma once
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

/**
 * If you have development tools on the Mac, you have a CPP compiler.
 * A simple convenience class is provided to make reading from the SMC more intuitive and safer.
 */
#ifndef APPLESMC_READER_H
#define APPLESMC_READER_H

#include "smc-read.h"
#include <vector>
#include <string>

/**
 * To use this class, simply declare an instance on the stack with:
 *  	AppleSMCReader smc;
 * The constructor will open a connection to the SMC, and the destructor will close that connection.
 * You can read values with something like:
 *  	auto corePower = smc.readNumber("PC0C");
 * All of the class methods (except the destructor) throw exceptions of type std::system_error if there are errors communicating with the SMC.
 */
class AppleSMCReader {
public:
	AppleSMCReader();

	~AppleSMCReader();

	AppleSMCReader(const AppleSMCReader& src) = delete;

	AppleSMCReader& operator=(const AppleSMCReader& src) = delete;

	/**
	 * Reads all keys that are available on the SMC of this machine and returns their values.
	 */
	std::vector<std::pair<std::string, double>> allKeyValues();

	void getKeyMetaInfo(const char* key, SMCKeyMetaData& meta);

	/**
	 * Simple invokes @AppleSMCReadNumber
	 */
	double readNumber(const char* key);

	// If you attempt to read a specific data type from the SMC and the key is *not* of the expected dataType, an exception (std::system_error.code == kIOReturnBadArgument) will be thrown
	uint8_t readUInt8(const char* key);

	int8_t readInt8(const char* key);

	uint16_t readUInt16(const char* key);

	int16_t readInt16(const char* key);

	uint32_t readUInt32(const char* key);

	// If you attempt to read a key that is *not* one of the known decimal types (defined at the top of this file), this method will return a NAN value.
	float readFloat(const char* key);

protected:
	io_connect_t conn;
};

#endif // APPLESMC_READER_H
