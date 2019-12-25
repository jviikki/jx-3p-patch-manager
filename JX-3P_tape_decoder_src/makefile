ifdef SystemRoot
	OS = win32
	RM = del /Q
	CP = copy
	FixPath = $(subst /,\,$1)
	LibSndFile = libsndfile-1
else
	OS = linux
	RM = rm -f
	CP = cp
	FixPath = $1
	LibSndFile = sndfile
endif

incdir=include
objdir=obj
libdir=$(call FixPath, lib/$(OS))
srcdir=src
CC=gcc
CFLAGS=-I$(incdir) -L$(libdir) -Wall


# targets
dist : decode_patches
all : analyzer bitstream c1diff decode_patches
.PHONY : dist all


# components
detector.o : $(srcdir)/detector.c
	$(CC) -c -o $(objdir)/$@ $< $(CFLAGS)

demodulator.o : $(srcdir)/demodulator.c
	$(CC) -c -o $(objdir)/$@ $< $(CFLAGS)

decoder_patch.o : $(srcdir)/decoder_patch.c
	$(CC) -c -o $(objdir)/$@ $< $(CFLAGS)

sndfile :
ifeq ($(OS),win32)
	copy lib\win32\libsndfile-1.dll .
endif
	$(CP) $(call FixPath, lib/$(OS)/sndfile.h) $(incdir)




# executables
analyzer.o : $(srcdir)/analyzer.c sndfile
	$(CC) -c -o $(objdir)/$@ $< $(CFLAGS)
analyzer: detector.o analyzer.o
	$(CC) -o $@ $(addprefix $(objdir)/, $^) $(CFLAGS) -l$(LibSndFile)

bitstream.o : $(srcdir)/bitstream.c sndfile
	$(CC) -c -o $(objdir)/$@ $< $(CFLAGS)
bitstream: detector.o demodulator.o bitstream.o
	$(CC) -o $@ $(addprefix $(objdir)/, $^) $(CFLAGS) -l$(LibSndFile)

c1diff.o : $(srcdir)/c1diff.c sndfile
	$(CC) -c -o $(objdir)/$@ $< $(CFLAGS)
c1diff: detector.o demodulator.o c1diff.o
	$(CC) -o $@ $(addprefix $(objdir)/, $^) $(CFLAGS) -l$(LibSndFile)

decode_patches.o : $(srcdir)/decode_patches.c sndfile
	$(CC) -c -o $(objdir)/$@ $< $(CFLAGS)
decode_patches: detector.o demodulator.o decoder_patch.o decode_patches.o
	$(CC) -o $@ $(addprefix $(objdir)/, $^) $(CFLAGS) -l$(LibSndFile)


clean:
	$(RM) $(call FixPath, $(objdir)/*.o)
	$(RM) $(call FixPath, $(incdir)/sndfile.h)
	$(RM) libsndfile*
	$(RM) analyzer* bitstream* c1diff* decode_patches*
