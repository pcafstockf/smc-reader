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

#ifndef SMC_READER_H
#define SMC_READER_H

#include <IOKit/IOKitLib.h>

// Allow regular C code to use everything except the AppleSMCReader class.
#ifdef __cplusplus
extern "C" {
#endif

	// SMC data types are described in human readable string format, even though the SMC itself considers types to be 32 bit unsigned integers.
	// Comparing two numbers is a *lot* faster than converting to a string and doing calling strcmp, so this library works with integers.
#define DATATYPE_FP1F_KEY 0x66703166    // "fp1f"
#define DATATYPE_FP4C_KEY 0x66703463    // "fp4c"
#define DATATYPE_FP5B_KEY 0x66703562    // "fp5b"
#define DATATYPE_FP6A_KEY 0x66703661    // "fp6a"
#define DATATYPE_FP79_KEY 0x66703739    // "fp79"
#define DATATYPE_FP88_KEY 0x66703838    // "fp88"
#define DATATYPE_FPA6_KEY 0x66706136    // "fpa6"
#define DATATYPE_FPC4_KEY 0x66706334    // "fpc4"
#define DATATYPE_FPE2_KEY 0x66706532    // "fpe2"

#define DATATYPE_SP1E_KEY 0x73703165    // "sp1e"
#define DATATYPE_SP3C_KEY 0x73703363    // "sp3c"
#define DATATYPE_SP4B_KEY 0x73703462    // "sp4b"
#define DATATYPE_SP5A_KEY 0x73703561    // "sp5a"
#define DATATYPE_SP69_KEY 0x73703639    // "sp69"
#define DATATYPE_SP78_KEY 0x73703738    // "sp78"
#define DATATYPE_SP87_KEY 0x73703837    // "sp87"
#define DATATYPE_SP96_KEY 0x73703936    // "sp96"
#define DATATYPE_SPB4_KEY 0x73706234    // "spb4"
#define DATATYPE_SPF0_KEY 0x73706630    // "spf0"

#define DATATYPE_UINT8_KEY 0x75693820   // "ui8 "
#define DATATYPE_UINT16_KEY 0x75693136  // "ui16"
#define DATATYPE_UINT32_KEY 0x75693332  // "ui32"

#define DATATYPE_SI8_KEY 0x73693820     // "si8 "
#define DATATYPE_SI16_KEY 0x73693136    // "si16"

#define DATATYPE_PWM_KEY 0x7B70776D     // "{pwm"
#define DATATYPE_FLAG_KEY 0x666C6167    // "flag"
#define DATATYPE_HEX_KEY 0x6865785F     // "hex_"


// For purposes of this library, we have a 32 byte buffer for exchanging command specific data with the SMC.
typedef uint8_t SMCBytes_t[32];

// This declares the meta info about a key.
typedef struct {
	uint32_t dataSize;
	uint32_t dataType;
	uint8_t dataAttributes;
} SMCKeyMetaData;

/**
 * Code using I/O Kit usually follows the same pattern:
 *  Find the service (usually via IOServiceGetMatchingServices)
 *  Open a connection to the service with IOServiceOpen
 *  Send some message to the service and get the result using one of the IOConnectCall*** functions.
 *  Close the service with IOServiceClose
 * This function performs the first two steps and if successful initializes the connection reference.
 *
 * @param conn  Reference to a connection handle used to communicate with the SMC.  This value is undefined if the connection was not opened successfully.
 * @return      kIOReturnSuccess if the connection was opened successfully, some other error if not.
 */
IOReturn AppleSMCOpen(io_connect_t& conn);

/**
 * Read the numeric value of a key from the SMC.
 *
 * @param conn      Reference to the connection handle obtained from @see AppleSMCOpen
 * @param key       A human readable string describing the key (such as "PC0C" for CPU Core power in watts).
 * @param value     The current value of 'key'.  This value is undefined if the function returns does not return kIOReturnSuccess.
 * @return          kIOReturnSuccess if the value was read successfully, some other error if not.
 */
IOReturn AppleSMCReadNumber(io_connect_t& conn, const char* key, double& value);

/**
 * Closes the connection handle obtained previously from @see AppleSMCOpen
 *
 * @param conn      Reference to the connection handle obtained from @see AppleSMCOpen
 * @return          kIOReturnSuccess if the connection was closed successfully, some other error if not.
 */
IOReturn AppleSMCClose(io_connect_t& conn);

/**
 * Convenience function to map SMC error codes to human readable strings.
 */
const char* const AppleSMCErrorToString(IOReturn error);

/**
 * Convenience function to collect the meta information about a key (because you likely already know it if you are asking for that key).
 */
IOReturn AppleSMCGetKeyMetaInfo(io_connect_t& conn, const char* key, SMCKeyMetaData* meta);

/**
 * This function is exposed for unusual scenarios.
 * You should normally use @see AppleSMCReadNumber
 * This will read the data from a specific key in the SMC into the supplied buffer and set the dataType and buffLen parameters to reflect the type of the data in the buffer.
 * Use this if you need to read any of the following non-numeric types:
 *      ch8*, {alc, {alv, {hdi, {lim, {fds, {rev
 *    FWIW, these are the keys that show up on my macbook pro that are *not* listed in the defines above.
 *    ch8* seems to be a bit stream of all varying lengths.  The others seem to be fixed length.
 *    One oddness was the key MSPT which is of type ui8, but is of length 5
 *    This is probably you best place to start if you want to explore:  https://www.insanelymac.com/forum/topic/328814-smc-keys-knowledge-database/
 */
IOReturn AppleSMCReadBuffer(io_connect_t& conn, const char* key, uint32_t& dataType, SMCBytes_t buff, uint8_t& buffLen);

/**
 * Decimal values are read from the SMC as 16 bit unsigned integers that are then converted to decimal based to the 'dataType'.
 * This function performs that conversion and is exposed for scenarios where you might need to use @AppleSMCReadBuffer
 */
float ToSMCFloat(uint32_t dataType, uint16_t value);

/**
 * This function is exposed for scenarios where you might need to use @AppleSMCReadBuffer
 */
double ToSMCNumber(uint32_t dataType, const SMCBytes_t buf, uint8_t bufLen);

#ifdef __cplusplus
}
#endif

/**
 * If you have development tools on the Mac, you have a CPP compiler.
 * A simple convenience class is provided to make reading from the SMC more intuitive and safer.
 */
#ifdef __cplusplus

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

#endif  //__cplusplus

#endif //SMC_READER_H
