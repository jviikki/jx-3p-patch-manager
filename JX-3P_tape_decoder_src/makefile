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
bindir=bin
CC=gcc
CFLAGS=-I$(incdir) -Wall -g


# targets
dist : $(bindir)/decode_patches
all : $(bindir)/analyzer $(bindir)/bitstream $(bindir)/c1diff $(bindir)/decode_patches
.PHONY : dist all


# components
detector.o : $(srcdir)/detector.c
	$(CC) -c -o $(objdir)/$@ $< $(CFLAGS)

demodulator.o : $(srcdir)/demodulator.c
	$(CC) -c -o $(objdir)/$@ $< $(CFLAGS)

decoder_patch.o : $(srcdir)/decoder_patch.c
	$(CC) -c -o $(objdir)/$@ $< $(CFLAGS)


# executables
analyzer.o : $(srcdir)/analyzer.c
	$(CC) -c -o $(objdir)/$@ $< $(CFLAGS)
$(bindir)/analyzer: detector.o analyzer.o
	$(CC) -o $@ $(addprefix $(objdir)/, $^) $(CFLAGS) -l$(LibSndFile)

bitstream.o : $(srcdir)/bitstream.c
	$(CC) -c -o $(objdir)/$@ $< $(CFLAGS)
$(bindir)/bitstream: detector.o demodulator.o bitstream.o
	$(CC) -o $@ $(addprefix $(objdir)/, $^) $(CFLAGS) -l$(LibSndFile)

c1diff.o : $(srcdir)/c1diff.c
	$(CC) -c -o $(objdir)/$@ $< $(CFLAGS)
$(bindir)/c1diff: detector.o demodulator.o c1diff.o
	$(CC) -o $@ $(addprefix $(objdir)/, $^) $(CFLAGS) -l$(LibSndFile)

decode_patches.o : $(srcdir)/decode_patches.c
	$(CC) -c -o $(objdir)/$@ $< $(CFLAGS)
$(bindir)/decode_patches: detector.o demodulator.o decoder_patch.o decode_patches.o
	$(CC) -o $@ $(addprefix $(objdir)/, $^) $(CFLAGS) -l$(LibSndFile)


clean:
	$(RM) $(call FixPath, $(objdir)/*.o)
	$(RM) $(bindir)/analyzer* $(bindir)/bitstream* $(bindir)/c1diff* $(bindir)/decode_patches*
