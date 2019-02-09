# makeTSX
A program to create TSX files for MSX computers from WAV tapes.

## Howto compile
**Linux x64:**
Dependencies: `g++`
$ make

**Linux x32:**
Dependencies: `g++ g++-multilib linux-libc-dev:i386`
$ make -f Makefile.linux32

**Windows x64:**
Dependencies: `mingw-w64`
$ make -f Makefile.win64

**Windows x32:**
Dependencies: `mingw-w64`
$ make -f Makefile.win32
