#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "sndfile.h"

#include "audio.h"

void write_data(SNDFILE* file) {
    for (int i = 0; i < 10000; i++) {
        int res = 0;

        if ((i / 1000) % 2) {
            res = audio_write_bit_one(file);
        } else {
            res = audio_write_bit_zero(file);
        }

        if (res) {
            printf("Error writing audio. i = %d", i);
            return;
        }
    }
}

void write_random_data(SNDFILE* file) {
    time_t t;
    srand((unsigned) time(&t));

    for (int i = 0; i < 50000; i++) {
        int res = 0;

        if (rand() > RAND_MAX / 2) {
            res = audio_write_bit_one(file);
        } else {
            res = audio_write_bit_zero(file);
        }

        if (res) {
            printf("Error writing audio. i = %d", i);
            return;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("USAGE: %s <audio_file>\n", argv[0]);
        return 1;
    }

    char* path = argv[1];
    SF_INFO info = {
        .samplerate = 44100,
        .channels = 1,
        .format = SF_FORMAT_WAV | SF_FORMAT_PCM_16
    };
    SNDFILE* file = NULL;

    file = sf_open(path, SFM_WRITE, &info);
    if (!file) {
        printf("%s\n", sf_strerror(file));
        return 1;
    }

    printf("frames: %lld, samplerate: %d, channels: %d, format: %d, sections: %d, seekable: %d\n",
        info.frames, info.samplerate, info.channels, info.format, info.sections, info.seekable);

    // write_data(file);
    write_random_data(file);

    int error = sf_close(file);
    if (error) {
        printf("Unable to close the file: %s\n", sf_error_number(error));
        return 1;
    }

    return 0;
}