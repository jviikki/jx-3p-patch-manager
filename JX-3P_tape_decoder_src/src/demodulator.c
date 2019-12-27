/*
 * demodulator.c
 * Convert zero-crossings into bitstream
 * Sync lock to header tone, then begin convert at start-of-data
 * Custom variant for Roland JX-3P keyboard tape format
 * 
 * See README.txt for license and installation notes.
*/

#include <stdio.h>
#include "demodulator.h"

// data synchronization state
#define SYNC_SEARCHING		0	// 0 length unknown; calculating
#define SYNC_LOCKED			1	// first SOD found; calc 0 length; read data


/*
 * Format-specifics for Roland JX-3P
 */

#define LONG_VALUE			1		// digital value of long cycles
#define SHORT_VALUE			0		// digital value of short cycles

#define SOD_PHASES			0		// phases in Start Of Data marker
#define LONG_BIT_PHASES		2		// phase count for long bit value
#define SHORT_BIT_PHASES	2		// phase count for short bit value
#define SHORT_THRESHHOLD	0.625	// long:short is ~4:1 ratio


Demodulator_State init_demodulator_state()
{
	Demodulator_State ds;

	ds.sync_state = SYNC_SEARCHING;
	ds.tone_total_crossings = 0;
	ds.tone_crossings_sum = 0;
	ds.tone_sod_count = 0;
	ds.long_width = 0.0;
	ds.short_threshhold = SHORT_THRESHHOLD;
	ds.bit_phase_count = 0;
	ds.bit_value = 0;
	return ds;
}


/* read starting tone to find time width of a zero bit
 * synchronize with start of data (SOD)
 * demodulate zero crossings to return a bucket of integer bits
*/
int demodulate_bitstream(
	int xing_bucket[],
	int xing_count,
	Demodulator_State *ds,
	int bit_bucket[]	//TODO: unchecked buffer!
)
{
	int bit_count = 0;
	int i;

	for (i = 0; i < xing_count; i++)
	{

		if (ds->sync_state == SYNC_SEARCHING)
		{
			if (ds->tone_crossings_sum == 0)
			{
				// init header detection; skip 1st xing; add 2nd next round
				ds->tone_crossings_sum = -1;
				ds->tone_total_crossings = 0;
				ds->long_width = 0;
				continue;
			}
			if (ds->tone_crossings_sum == -1)
			{
				// collect first valid crossing and begin search for SOD
				ds->tone_crossings_sum = xing_bucket[i];
				ds->tone_total_crossings = 1;
			}

			if ((float) xing_bucket[i] < (ds->long_width * ds->short_threshhold))
			{
				// if a short cycle, it's Start Of Data
				ds->tone_sod_count++;
				if (ds->tone_sod_count >= SOD_PHASES )
				{
					// SOD finished; sync locked
					//fprintf(stderr, "sync state LOCKED. zero width=%.3f\n", ds->zero_width);
					ds->sync_state = SYNC_LOCKED;
					// since state is locked, this IF will fall out into next if
				}
			}
			else
			{
				// not SOD; normal header tone
				ds->tone_crossings_sum += xing_bucket[i];
				ds->tone_total_crossings++;
				ds->tone_sod_count = 0;
			}
			ds->long_width = (double) ds->tone_crossings_sum / (double) ds->tone_total_crossings;
		}
		if (ds->sync_state == SYNC_LOCKED)		// SYNC_LOCKED: demodulate data
		{
			/*
			 * 	if (begin of bit)
			 * 		start count
			 * 		continue
			 * 	if ( bitval != rs->bit_value)
			 * 		we've got a bad bit detection; error
			 * 	else
			 * 		increment count
			 * 	if (end of bit)
			 * 		clear count
			 * 		put bit in bucket
			 * 		continue
			*/
			int bitval;
			bitval = LONG_VALUE;
			if ((float) xing_bucket[i] < (ds->long_width * ds->short_threshhold))
			{
				bitval = SHORT_VALUE;
			}

			// if first phase of bit, store value
			if (ds->bit_phase_count == 0)
			{
				ds->bit_value = bitval;
			}

			ds->bit_phase_count++;

			// get trigger condition for end of bit
			int endphases;
			endphases = LONG_BIT_PHASES;
			if ( ds->bit_value == SHORT_VALUE )
			{
				endphases = SHORT_BIT_PHASES;
			}

			// test for bad, incompleted bit detection
			if ( ( ds->bit_phase_count <= endphases ) && ( bitval != ds->bit_value ) )
			{
				// we have a bad read or misformat; clear count and throw error
				fprintf(stderr, "bad bit read: val=%d (%d of %d phases)\n", 
						ds->bit_value, ds->bit_phase_count, endphases);
				ds->bit_phase_count = 0;
				ds->bit_value = LONG_VALUE;
				continue;
			}

			// test end of bit; push to bucket
			if (ds->bit_phase_count == endphases)
			{
				bit_bucket[bit_count++] = ds->bit_value;
				ds->bit_phase_count = 0;
				ds->bit_value = LONG_VALUE;
			}
		}
	}

	return bit_count;
}
