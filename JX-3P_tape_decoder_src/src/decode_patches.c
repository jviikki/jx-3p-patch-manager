/*
 * decode_patches.c
 * Input wavfile of tape dump, decode, and output parameters from all patches and banks
 * 
 * See README.txt for license and installation notes.
*/

#include <stdio.h>
#include "sndfile.h"
#include "detector.h"
#include "demodulator.h"
#include "decoder_patch.h"


#define BUFFER_LEN 1024		// audio buffer
#define BUCKET_LEN 1000		// should be > 0.5 * BUFFER_LEN

/* File IO */
SNDFILE		*infile;
SF_INFO		sfinfo;
const char	*infilename;
FILE		*patchfile_csv;
const char	*patchfilename_csv = "patchdump.csv";
FILE		*patchfile_smf;
const char	*patchfilename_smf = "patchdump.mid";

Patch_Collection patch_collection;


/* command options (for when I institute command line options */
#define VERBOSITY_QUIET		0
#define VERBOSITY_FATAL		1
#define VERBOSITY_WARN		2
#define VERBOSITY_NORMAL	3
#define VERBOSITY_DEBUG		4
// I'm just not happy with verbosity tests - doesn't filter down to submodules
unsigned short int opt_verbosity = VERBOSITY_NORMAL;
const char *outbasename;
unsigned short int opt_csv		= 1;
unsigned short int opt_midi		= 0;



/* prototypes */

void wait_for_exit();


int main( int argc, char *argv[] )
{
	// master count of errors. Pause for user input at end if > 0.
	int error_count = 0;

// TODO: add arguments for flexibility
	// check argc for filename
	if ( argc < 2 ) /* look for wave filename on command line */
    {
        fprintf(stderr, "usage: %s (wavfilename)", argv[0] );
        return 1;
    }
    else
    {
		infilename = argv[1];
	}

	if (!(opt_csv || opt_midi))
	{
		if (opt_verbosity >= VERBOSITY_FATAL)
			fprintf(stderr, "No CSV or MIDI output options requested. Exiting.\n");
        wait_for_exit();
		return 1;
	}

	if (opt_csv && ! (patchfile_csv = fopen (patchfilename_csv, "w")))
	{
		if (opt_verbosity >= VERBOSITY_FATAL)
			fprintf(stderr, "Not able to open output CSV file %s.\n", patchfilename_csv);
        wait_for_exit();
		return 1;
	};

	if (opt_midi && ! (patchfile_smf = fopen (patchfilename_smf, "w")))
	{
		if (opt_verbosity >= VERBOSITY_FATAL)
			fprintf(stderr, "Not able to open output MIDI file %s.\n", patchfilename_smf);
		fclose(patchfile_csv);
        wait_for_exit();
		return 1;
	};


	if (! (infile = sf_open (infilename, SFM_READ, &sfinfo)))
	{
		if (opt_verbosity >= VERBOSITY_FATAL)
			fprintf(stderr, "Not able to open input file %s.\n%s\n", infilename, sf_strerror(NULL));
		if (opt_midi)
			fclose(patchfile_smf);
		if (opt_csv)
			fclose(patchfile_csv);
        wait_for_exit();
		return 1;
	};

	if (opt_verbosity >= VERBOSITY_DEBUG) fprintf(stdout, "file format: %d\n", sfinfo.format);
	if (opt_verbosity > VERBOSITY_QUIET) fprintf(stdout, "sample rate: %d\n", sfinfo.samplerate);
	if (opt_verbosity > VERBOSITY_QUIET) fprintf(stdout, "channels: %d%s\n", sfinfo.channels, 
		((opt_verbosity >= VERBOSITY_WARN) && (sfinfo.channels>1) ? " (Only first channel used.)" : ""));

	int file_sample_count = 0;

	// audio buffer
   	static double full_sample_data[BUFFER_LEN];		// full audio data buffer
   	static double channel_sample_data[BUFFER_LEN];	// first channel audio data buffer
	int readcount;						// buffer size fullness

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
	int bits_count = 0;
	int patches_count = 0;

	// init decoder_patch state
	Decoder_State decode_state;
	decode_state = init_decoder_state();
	patch_collection = init_patch_collection();


	// Main read loop:
    while ((readcount = sf_read_double(infile, full_sample_data, BUFFER_LEN)))
	{
		file_sample_count += readcount;
		// pull first channel only
		int si;
		for (si=0; si < BUFFER_LEN; si += sfinfo.channels) {
			channel_sample_data[si] = full_sample_data[si];
		}
		// detect
		xing_bucket_count = detect_crossings(channel_sample_data, readcount, &cursor, xing_bucket);
		// demodulate
		bit_bucket_count = demodulate_bitstream(xing_bucket, xing_bucket_count, &demod_state, bit_bucket);
		bits_count += bit_bucket_count;
		// decode
		int patch_count;
		patch_count = decode_bitstream(bit_bucket, bit_bucket_count, &decode_state, &patch_collection );
		patches_count += patch_count;
	}

	if (opt_verbosity >= VERBOSITY_DEBUG) printf("file samples: %d\n", file_sample_count);
	if (opt_verbosity >= VERBOSITY_DEBUG) printf("crossings: %d\n", cursor.crossing_count);
	if (opt_verbosity >= VERBOSITY_DEBUG) printf("bits count: %d\n", bits_count);
	if (opt_verbosity > VERBOSITY_QUIET) printf("patch count: %d\n", patches_count);

	if (!patches_count) {
		if (opt_verbosity >= VERBOSITY_FATAL) fprintf(stderr, "No patches were decoded. See documentation for guidance.\n");
		sf_close(infile);
		if (opt_midi)
			fclose(patchfile_smf);
		if (opt_csv)
			fclose(patchfile_csv);
        wait_for_exit();
		return 2;
	}

	if (patches_count != 32) {
		if (opt_verbosity >= VERBOSITY_FATAL) fprintf(stderr, "Some patches could not be decoded.\n");
		error_count++;
	}

	if (opt_csv)
	{
		// dump patch parameters into csv
		int i_bank, i_patch;
		fprintf(patchfile_csv, "%s\n", print_csv_header());

		for (i_bank=0; i_bank<2; i_bank++)
		{
			for (i_patch=0; i_patch<16; i_patch++) {
				jx3p_format *p = get_patch(&patch_collection, i_bank, i_patch);
				fprintf(patchfile_csv, "%s\n", print_csv_patch(p));
			}
		}
	}

/*	TODO: dump patch parameters into .mid file
 * since SMF file has no need for flexibility, we just compose on the fly without
 * using a MIDI library. Write the MThd chunk to the file, and compose the MTrk 
 * chunk offline before writing because you need the chunk size.
 * Fold the writevarlen() function into this file.
*/
/*
	for (i_bank=0; i_bank<2; i_bank++)
	{
		for (i_patch=0; i_patch<16; i_patch++) {
			jx3p_format *p = get_patch(&patch_collection, i_bank, i_patch);
		}
	}
*/

	sf_close(infile);
	if (opt_midi)
		fclose(patchfile_smf);
	if (opt_csv)
		fclose(patchfile_csv);

	if (error_count) {
		wait_for_exit();
		return 1;
	}
	return 0;
}

void wait_for_exit()
{
	if (opt_verbosity == VERBOSITY_QUIET) return;
	fprintf(stderr, "Errors were found. Press Enter to exit.");
	getchar();
	return;
}

