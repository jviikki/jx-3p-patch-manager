ABOUT:

Enclosed is the source code and a Windows-compatible command line utility to convert an audio file recorded from a Roland JX-3P patch tape dump into a human-readable format. The JX-3P has no ability to dump patch parameters over MIDI, so this tool was written to get around that limitation. The resulting spreadsheet file will allow the end user to see the actual values of each parameter in the user patch banks.

The main utility, decode_patches.exe, takes the name of a tape dump audio file and outputs a file named 'patchdump.csv'. The utility is invoked as follows:

	decode_patches.exe patch_dump_file.wav


INSTALLATION:

The Windows-compatible 32-bit binary executable was built with the MinGW version of the GNU C Compiler and GNU Make on a Windows 7 system. If you want to edit the sources and recompile on Windows, it is recommended to use these tools in your toolchain. MinGW GCC is available at (http://www.mingw.org/). GNU Make for Windows is available at (http://gnuwin32.sourceforge.net/packages/make.htm). I provide no support for toolchain installation or code compilation. These tools (or compatible) are probably installed by default on your Linux system.

To compile, configure the makefile with the location of your gcc executable (if it's not in the path), then run `make`. This will build the required modules and binaries and will copy the libsndfile library DLL to the appropriate place. To delete the binaries, run `make clean`.


LICENSE:

Written by Shawn Thomas (C)2013 for publication on (http://www.phaysis.com). This code is provided as-is with no warranty expressed or implied. Use at your own risk. Author is not responsible for data loss.

Due to the difficulty of copyright enforcement once the source code is released to the public, the segments of this work written by me are released to the public domain. Do with it as you wish. If I can figure it out, you can figure it out. No magical skills required.

This work includes a dynamically-linked binary copy of the libsndfile library written by Erik de Castro Lopo (available at http://www.mega-nerd.com/libsndfile/) for audio file I/O. It is covered by the LGPL license and is copyrighted by its author. A copy of the LGPL license is located at (http://www.gnu.org/copyleft/lesser.html). If compiling on Linux, it is best to use your package management system to install the library (and any development files if you choose to recompile) into your system folders. Otherwise, download the library package from the home site and install a copy of the .dll/.so into the project's root folder.
