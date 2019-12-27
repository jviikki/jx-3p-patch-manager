#ifndef AUDIO_H
#define AUDIO_H

#include "sndfile.h"

// extern const short AUDIO_BIT_ZERO[];
// extern const short AUDIO_BIT_ONE[];

int audio_write_bit_zero(SNDFILE* file);
int audio_write_bit_one(SNDFILE* file);

#endif // AUDIO_H