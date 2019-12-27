#ifndef AUDIO_H
#define AUDIO_H

#include "sndfile.h"

int audio_write_bit_zero(SNDFILE* file);
int audio_write_bit_one(SNDFILE* file);

#endif // AUDIO_H