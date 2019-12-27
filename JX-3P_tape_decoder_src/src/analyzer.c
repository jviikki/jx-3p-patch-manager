/*
 * analyzer - determine freqs of 0 and 1
 * emits text file of all zero crossing lengths
 * 
 * See README.txt for license and installation notes.
*/

#include <stdio.h>
#include "sndfile.h"
#include "detector.h"

#define BUFFER_LEN 4096
#define BUCKET_LEN 2048
#define HISTOGRAM_LEN 50


/* File IO */
SNDFILE		*infile;
SF_INFO		sfinfo;
const char	*infilename;
FILE		*crossfile;
const char	*crossfilename = "crossings.csv";


int main( int argc, char *argv[] )
{
	// check argc for filename
	if ( argc < 2 ) /* look for wave filename on command line */
    {
        printf( "usage: %s (wavfilename)", argv[0] );
        return 1;
    }
    else
    {
		infilename = argv[1];
	}

	int i;
   	static double data[BUFFER_LEN];	// audio data buffer
	int readcount;					// buffer size filled

	// statistics
	int min_sample, max_sample;
	min_sample = max_sample = 0;
	int sample_count;
	sample_count = 0;
	double max_val, min_val;
	max_val = min_val = 0;
	double dc_bias, dc_sum;
	dc_bias = dc_sum = 0;

	// Histogram of samples between zero-crossings
	int	xing_histogram[HISTOGRAM_LEN];
	for ( i = 0; i < HISTOGRAM_LEN ; i++ )
	{
		xing_histogram[i] = 0;
	}
	
	// zero-crossing state and readers
	int cross_bucket[BUCKET_LEN];
	int crossing_count;
	Detector_State cursor;
	cursor = init_detector_state(BUFFER_LEN);

	if (! (crossfile = fopen (crossfilename, "w")))
	{
		printf ("Not able to open output file %s.\n", crossfilename);
		return 1;
	};


	if (! (infile = sf_open (infilename, SFM_READ, &sfinfo)))
	{
		printf ("Not able to open input file %s.\n", infilename);
		puts (sf_strerror (NULL));
		return 1;
	};

	fprintf(crossfile, "sample,length\n");
    while ((readcount = sf_read_double (infile, data, BUFFER_LEN)))
	{
//		fprintf(stderr, "readcount: %d\n", readcount);
		for (i = 0 ; i < readcount ; i++)
		{
			sample_count++;
			if (data[i] > max_val)
			{
				max_sample = sample_count;
				max_val = data[i];
			}
			if (data[i] < min_val)
			{
				min_sample = sample_count;
				min_val = data[i];
			}
			dc_sum += data[i];
		}

		int xsmp;
		xsmp = cursor.sample_count;
		crossing_count = detect_crossings(data, readcount, &cursor, cross_bucket);
		for (i=0; i<crossing_count; i++)
		{
			if ( (cross_bucket[i] > 0) && (cross_bucket[i] < HISTOGRAM_LEN) )
			{
				xing_histogram[cross_bucket[i]]++;
			}
			xsmp += cross_bucket[i];
			fprintf(crossfile, "%d,%d\n", xsmp, cross_bucket[i]);
		}
	}
	dc_bias = dc_sum / sample_count;


	printf("samples: %d\n", sample_count);
	printf("sample rate: %d\n", sfinfo.samplerate);
	printf("min: %f\tat: %d\nmax: %f\tat: %d\n",
		min_val, min_sample, max_val, max_sample);
	printf("dc bias: %f\n", dc_bias);
	printf("crossings: %d\n", cursor.crossing_count);

	printf("Zero-crossing histogram:\n");
	for (i=0; i<HISTOGRAM_LEN; i++)
	{
		if (xing_histogram[i] > 0)
		{	float pcnt;
			pcnt = (xing_histogram[i] * 1.0) / (cursor.crossing_count * 1.0) * 100.0;
			printf("%02d: %7d \t(%7.3f%%)\n", i, xing_histogram[i], pcnt);
		}
	}

	sf_close(infile);
	fclose(crossfile);

	return 0;
}

