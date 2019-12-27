#include <stdio.h>
#include "sndfile.h"

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

    int error = sf_close(file);
    if (error) {
        printf("Unable to close the file: %s\n", sf_error_number(error));
        return 1;
    }

    return 0;
}