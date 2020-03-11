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
#include <algorithm>
#include <iostream>
#include <iomanip>

bool cmdOptionExists(const char** begin, const char** end, const std::string& option) {
	return std::find(begin, end, option) != end;
}

void printPair(AppleSMCReader& rdr, std::pair<std::string, double>& p) {
	SMCKeyMetaData meta;
	rdr.getKeyMetaInfo(p.first.c_str(), meta);
	std::cout << p.first << " (len=" << meta.dataSize << ",attr=" << std::showbase << std::hex << (uint32_t) meta.dataAttributes << ",type=" << std::showbase << std::hex << meta.dataType << ") = " << std::dec << std::setprecision(5) << std::fixed << p.second << std::endl;
}

int main(int argc, const char* argv[]) {
	bool help = false;
	if (argc < 2)
		help = true;
	else if (cmdOptionExists((const char**) argv + 1, (const char**) argv + argc, "-h"))
		help = true;
	else if (cmdOptionExists((const char**) argv + 1, (const char**) argv + argc, "-help"))
		help = true;
	else if (cmdOptionExists((const char**) argv + 1, (const char**) argv + argc, "--h"))
		help = true;
	else if (cmdOptionExists((const char**) argv + 1, (const char**) argv + argc, "--help"))
		help = true;
	bool dump = cmdOptionExists((const char**) argv + 1, (const char**) argv + argc, "--dump");
	if (help) {
		std::string s(argv[0]);
		std::cerr << s.substr(s.rfind('/') + 1) << ": Reads values from the Apple System Management Control (SMC) chip of this machine." << std::endl;
		std::cerr << "Usage:  [--help] | [--dump] | *" << std::endl;
		std::cerr << "--help  This usage message." << std::endl;
		std::cerr << "--dump  Print all discoverable keys and their values." << std::endl;
		std::cerr << "     *  One or more space separated keys (PC0C B0RM TC1C, etc.)" << std::endl;
	} else if (dump) {
		AppleSMCReader rdr;
		for (auto p : rdr.allKeyValues())
			printPair(rdr, p);
	} else {
		AppleSMCReader rdr;
		for (int i = 1; i < argc; i++) {
			if (strlen(argv[i]) <= 4) {
				try {
					auto p = std::make_pair(std::string(argv[i]), rdr.readNumber(argv[i]));
					printPair(rdr, p);
				}
				catch (const std::exception& ex) {
					std::cerr << "Error processing key '" << argv[i] << "' : " << ex.what() << std::endl;
				}
			}
		}
	}
	return 0;
}
