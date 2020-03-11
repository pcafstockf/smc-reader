# smc-reader
MIT licensed code for reading from the Apple System Management Control (SMC)

## About
[Apple's SMC subsystem](https://en.wikipedia.org/wiki/System_Management_Controller) allows you to query the system for CPU temperature, fan speeds, power usage, etc.  
There are currently many good libraries to accomplish that, but everything that I found was based on smc.cc/h which is a GPL licensed implementation.  
I felt there was a need for similar functionality under a more permissive license.

This project does contain the source code for a command line tool which allows you to query specific keys in the SMC, or to dump all SMC keys for your machine.  
However, the real purpose of the project is to publish the ./src/smc-read.c/.h files.

## Other Resources
This project is all about the code, it makes no attempt to be an information source about SMC itself.  
I found this [discussion thread](https://www.insanelymac.com/forum/topic/328814-smc-keys-knowledge-database/) to be a helpful starting point, and there are tons of links in that thread.  
There are a great list of keys at [fakesmc](https://app.assembla.com/wiki/show/fakesmc), and [VirtualSMC](https://github.com/acidanthera/VirtualSMC/blob/master/Docs/SMCKeys.txt)  
The [JSystemInfoKit](https://github.com/xythobuz/JSystemInfoKit) project also had a nice descriptions of the keys, and is [archived](https://web.archive.org/web/20160525005847/https://jbot-42.github.io/systeminfokit.html) here.

That's it.  
Enjoy!

## MIT License

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
