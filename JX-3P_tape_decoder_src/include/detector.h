/*
 * detector.h
 * Detect zero-crossings in audio.
 * 
 * See README.txt for license and installation notes.
*/

typedef struct {
	int sample_count;
	int crossing_count;
	int last_crossing_len;
	int last_crossing_sample;
	int phase;
	int buffer_len;	// length of buffer (if readcount < buffer_len, sndfile has found EOF)
} Detector_State;

Detector_State init_detector_state(int buffer_len);

int detect_crossings(double audio[], int readcount, Detector_State *readstate, int buffer[]);
