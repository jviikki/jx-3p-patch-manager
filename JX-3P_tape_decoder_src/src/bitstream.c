/*
 * bitstream.c
 * Read audio, output bitstream for visual analysis.
 * 
 * See README.txt for license and installation notes.
*/

#include <stdio.h>
#include "sndfile.h"
#include "detector.h"
#include "demodulator.h"


#define BUFFER_LEN 1024		// audio buffer
#define BUCKET_LEN 1000		// should be > 0.5 * BUFFER_LEN


/* File IO */
SNDFILE		*infile;
SF_INFO		sfinfo;
const char	*infilename;
FILE		*bitsfile;
const char	*bitsfilename = "bits.txt";


int main( int argc, char *argv[] )
{
	// check argc for filename
	if ( argc < 2 ) /* look for wave filename on command line */
    {
        fprintf( stderr, "usage: %s (wavfilename)", argv[0] );
        return 1;
    }
    else
    {
		infilename = argv[1];
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


	if (! (infile = sf_open (infilename, SFM_READ, &sfinfo)))
	{
		fprintf(stderr, "Not able to open input file %s.\n%s\n", infilename, sf_strerror(NULL));
		return 1;
	};

    while ((readcount = sf_read_double(infile, sample_data, BUFFER_LEN)))
	{
		xing_bucket_count = detect_crossings(sample_data, readcount, &cursor, xing_bucket);

		bit_bucket_count = demodulate_bitstream(xing_bucket, xing_bucket_count, &demod_state, bit_bucket);
		for (i=0; i < bit_bucket_count; i++)
		{
			bits_count++;
			fprintf(bitsfile, "%d",bit_bucket[i]);
		}
		if (bit_bucket_count > 0)
			fprintf(bitsfile, "\n");

	}

	printf("sample rate: %d\n", sfinfo.samplerate);
	printf("crossings: %d\n", cursor.crossing_count);
	printf("bits_count: %d\n", bits_count);

	sf_close(infile);
	fclose(bitsfile);

	return 0;
}
