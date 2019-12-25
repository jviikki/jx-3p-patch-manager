/*
 * demodulator.h
 * Convert zero-crossings into bitstream
 * Sync lock to header tone, then begin convert at start-of-data
 * Custom variant for Roland JX-3P keyboard tape format
 * 
 * See README.txt for license and installation notes.
*/


typedef struct {
	int sync_state;
	int tone_total_crossings;	// crossings while searching header tone
	int tone_crossings_sum;		// accumulator while searching header tone
	int tone_sod_count;			// phase count in detected start-of-data
	double long_width;			// average width of a long cycle
	double short_threshhold;	// shorter than this is a short cycle
	int bit_phase_count;		// count of same phases (compare against x_BIT_PHASES)
	int bit_value;				// value of detected bit
} Demodulator_State;

Demodulator_State init_demodulator_state();

int demodulate_bitstream( 
	int xing_bucket[],
	int xing_count,
	Demodulator_State *ds,
	int bit_bucket[]
);
