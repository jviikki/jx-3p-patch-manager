/*
 * c1diff
 * pulls the first patch (C01) of two wav dumps and compares them
 * 
 * See README.txt for license and installation notes.
*/

#include <stdio.h>
#include "sndfile.h"
#include "detector.h"
#include "demodulator.h"


#define BUFFER_LEN 4096		// audio buffer
#define BUCKET_LEN 2048		// should be > 0.5 * BUFFER_LEN
#define PATCH_DUMP_SIZE 25424
#define PATCH_C1_SIZE	286

/* File IO */
SNDFILE		*infile1, *infile2;
SF_INFO		sfinfo1, sfinfo2;
const char	*infilename1;
const char	*infilename2;
FILE		*bitsfile;
const char	*bitsfilename = "c1bits.txt";


int main( int argc, char *argv[] )
{
	// check argc for filename
	if ( argc < 3 ) /* look for wave filename on command line */
    {
        fprintf( stderr, "usage: %s (wavfilename1) (wavfilename2)", argv[0] );
        return 1;
    }
    else
    {
		infilename1 = argv[1];
		infilename2 = argv[2];
	}

	int i;
   	static double sample_data[BUFFER_LEN];	// audio data buffer
	int readcount;					// buffer size filled


	// zero-crossing state and readers
	int xing_bucket[BUCKET_LEN];
	int xing_bucket_count;
	Detector_State cursor;
	cursor = init_detector_state(BUFFER_LEN);


	// init demodulator
	int bit_bucket[BUCKET_LEN];
	int bit_bucket_count;
	Demodulator_State demod_state;
	demod_state = init_demodulator_state();
	int bits_count;
	bits_count = 0;

	if (! (bitsfile = fopen (bitsfilename, "w")))
	{
		fprintf (stderr, "Not able to open output file %s.\n", bitsfilename);
		return 1;
	};

	int patch_buffer_1[PATCH_DUMP_SIZE];
	int patch_buffer_2[PATCH_DUMP_SIZE];

	// read first dump file
	if (! (infile1 = sf_open (infilename1, SFM_READ, &sfinfo1)))
	{
		fprintf(stderr, "Not able to open input file %s.\n%s\n", infilename1, sf_strerror(NULL));
		return 1;
	};
    while ((readcount = sf_read_double(infile1, sample_data, BUFFER_LEN)))
	{
		xing_bucket_count = detect_crossings(sample_data, readcount, &cursor, xing_bucket);

		bit_bucket_count = demodulate_bitstream(xing_bucket, xing_bucket_count, &demod_state, bit_bucket);
		for (i=0; i < bit_bucket_count; i++)
		{
			patch_buffer_1[bits_count++] = bit_bucket[i];
		}
	}
	sf_close(infile1);


	// reset for reading 2nd patch
	cursor = init_detector_state(BUFFER_LEN);
	demod_state = init_demodulator_state();
	bits_count = 0;

	// read second dump file
	if (! (infile2 = sf_open (infilename2, SFM_READ, &sfinfo2)))
	{
		fprintf(stderr, "Not able to open input file %s.\n%s\n", infilename2, sf_strerror(NULL));
		return 1;
	};
    while ((readcount = sf_read_double(infile2, sample_data, BUFFER_LEN)))
	{
		xing_bucket_count = detect_crossings(sample_data, readcount, &cursor, xing_bucket);

		bit_bucket_count = demodulate_bitstream(xing_bucket, xing_bucket_count, &demod_state, bit_bucket);
		for (i=0; i < bit_bucket_count; i++)
		{
			patch_buffer_2[bits_count++] = bit_bucket[i];
		}
	}
	sf_close(infile2);

	// print first patch raw
	fprintf(bitsfile, "%s:\n",infilename1);
	for (i=0; i < PATCH_C1_SIZE; i++)
	{
		fprintf(bitsfile, "%d",patch_buffer_1[i]);
	}
	fprintf(bitsfile, "\n");

	// print second patch raw
	fprintf(bitsfile, "%s:\n",infilename2);
	for (i=0; i < PATCH_C1_SIZE; i++)
	{
		fprintf(bitsfile, "%d",patch_buffer_2[i]);
	}
	fprintf(bitsfile, "\n");

	// print diff bits
	fprintf(bitsfile, "diff:\n");
	for (i=0; i < PATCH_C1_SIZE; i++)
	{
		int diff_flag;
		if (patch_buffer_1[i] != patch_buffer_2[i])
		{
			diff_flag = '#';
		}
		else
		{
			diff_flag = '-';
		}
		fprintf(bitsfile, "%c", diff_flag);
	}
	fprintf(bitsfile, "\n");

	fclose(bitsfile);

	return 0;
}
