//
//  main.cpp
//  smc-reader
//
//  Created by Frank Stock on 3/11/20.
//  Copyright © 2020 Byte Lightning. All rights reserved.
//
#include "apple-smc-reader.h"
#include <algorithm>
#include <iostream>
#include <iomanip>

bool cmdOptionExists(const char** begin, const char** end, const std::string& option) {
	return std::find(begin, end, option) != end;
}

void printPair(AppleSMCReader& rdr, std::pair<std::string,double>& p) {
	SMCKeyMetaData meta;
	rdr.getKeyMetaInfo(p.first.c_str(), meta);
	std::cout << p.first << " (len=" << meta.dataSize << ",attr=" << std::showbase << std::hex << (uint32_t)meta.dataAttributes << ",type=" << std::showbase << std::hex << meta.dataType << ") = " << std::dec << std::setprecision(5) << std::fixed << p.second << std::endl;
}

int main(int argc, const char * argv[]) {
	bool help = false;
	if (argc < 2)
		help = true;
	else if (cmdOptionExists((const char**)argv+1, (const char**)argv+argc, "-h"))
		help = true;
	else if (cmdOptionExists((const char**)argv+1, (const char**)argv+argc, "-help"))
		help = true;
	else if (cmdOptionExists((const char**)argv+1, (const char**)argv+argc, "--h"))
		help = true;
	else if (cmdOptionExists((const char**)argv+1, (const char**)argv+argc, "--help"))
		help = true;
	bool dump = cmdOptionExists((const char**)argv+1, (const char**)argv+argc, "--dump");
	if (help) {
		std::string s(argv[0]);
		std::cerr << s.substr(s.rfind("/") + 1) << ": Reads values from the Apple System Management Control (SMC) chip of this machine." << std::endl;
		std::cerr << "Usage:  [--help] | [--dump] | *" << std::endl;
		std::cerr << "--help  This usage message." << std::endl;
		std::cerr << "--dump  Print all discoverable keys and their values." << std::endl;
		std::cerr << "     *  One or more space separated keys (PC0C B0RM TC1C, etc.)" << std::endl;
	}
	else if (dump) {
		AppleSMCReader rdr;
		for (auto p : rdr.allKeyValues())
			printPair(rdr, p);
	}
	else {
		AppleSMCReader rdr;
		for (int i=1; i<argc; i++) {
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
