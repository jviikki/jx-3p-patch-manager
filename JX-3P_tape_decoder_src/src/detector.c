/*
 * detector.c
 * Detect zero-crossings in audio.
 * 
 * See README.txt for license and installation notes.
*/

/* TODO:
 * - Figure out why clipping causes misdetection of signal
 */

#include "detector.h"

#define QUIESCENCE_THRESHHOLD 0.15

/* Phase macro values */
#define	POSITIVE	+1
#define NEUTRAL		 0
#define NEGATIVE	-1


Detector_State init_detector_state(int buffer_len)
{
	Detector_State c;

	c.sample_count = 0;
	c.crossing_count = 0;
	c.last_crossing_len = 0;
	c.last_crossing_sample = 0;
	c.phase = NEUTRAL;
	c.buffer_len = buffer_len;

	return c;
}


int detect_crossings(
	double samples[],
	int readcount,
	Detector_State *ds,
	int cross_bucket[]	// FIXME: unchecked buffer!
)
{
	int i, bucket_pos;
	bucket_pos = 0;

	for (i = 0 ; i < readcount ; i++)
	{
		if (
			((ds->phase == NEGATIVE) && (samples[i] > QUIESCENCE_THRESHHOLD))
			||
			((ds->phase == POSITIVE) && (samples[i] < (QUIESCENCE_THRESHHOLD * -1)))
		)
		{
			cross_bucket[bucket_pos++] = ds->sample_count - ds->last_crossing_sample;
			ds->phase = NEUTRAL;
			ds->crossing_count++;
			ds->last_crossing_sample = ds->sample_count;
		}

		if (samples[i] > QUIESCENCE_THRESHHOLD)
			ds->phase = POSITIVE;
		if (samples[i] < (QUIESCENCE_THRESHHOLD * -1))
			ds->phase = NEGATIVE;

		ds->sample_count++;
	}
	if (readcount < ds->buffer_len)
	{	// EOF: append another crossing on the end
		int len = cross_bucket[bucket_pos-1];
		cross_bucket[bucket_pos++] = len;
		ds->crossing_count++;
	}
		
	return bucket_pos;
}
