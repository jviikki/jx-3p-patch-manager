#include <stdio.h>
#include "sndfile.h"

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("USAGE: %s <audio_file>\n", argv[0]);
        return 1;
    }

    char* path = argv[1];
    SF_INFO info;
    SNDFILE* file = NULL;

    file = sf_open(path, SFM_READ, &info);
    if (!file) {
        printf("%s\n", sf_strerror(file));
        return 1;
    }

    printf("frames: %lld, samplerate: %d, channels: %d, format: %d, sections: %d, seekable: %d\n",
        info.frames, info.samplerate, info.channels, info.format, info.sections, info.seekable);

    short samples[BUFFER_SIZE];
    sf_count_t items_read = 0;
    while ((items_read = sf_read_short(file, &samples[0], BUFFER_SIZE))) {
        for(uint i = 0; i < items_read; i++) {
            printf("%d\n", samples[i]);
        }
    }

    int error = sf_close(file);
    if (error) {
        printf("Unable to close the file: %s\n", sf_error_number(error));
        return 1;
    }

    return 0;
}