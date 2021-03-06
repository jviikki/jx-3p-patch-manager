#include <stdio.h>

#include "sndfile.h"

#include "audio.h"
#include "csv_parser.h"

#define BITS_IN_PILOT_TONE 4096
#define BITS_IN_SEPARATOR_TONE 48

// TODO: add error handling
void write_byte(SNDFILE* file, unsigned char byte) {
    audio_write_bit_zero(file); // serial frame start

    for (unsigned int i = 0; i < 8; i++) {
        if (byte & (1 << i))
            audio_write_bit_one(file);
        else
            audio_write_bit_zero(file);
    }

    audio_write_bit_one(file); // serial frame stop
    audio_write_bit_one(file);
}

// TODO: add error handling
void write_patch(SNDFILE* file, JX3P_PATCH* patch) {
    // Disregard the last byte which is the checksum
    int num_bytes = sizeof(patch->raw) / sizeof(unsigned char) - 1;
    unsigned char checksum = 0;

    for (int i = 0; i < num_bytes; i++) {
        write_byte(file, patch->raw[i]);
        checksum += patch->raw[i];
    }

    write_byte(file, checksum);
}

void write_pilot_tone(SNDFILE* file) {
    for (int i = 0; i < BITS_IN_PILOT_TONE; i++) {
        audio_write_bit_one(file);
    }
}

void write_separator(SNDFILE* file) {
    for (int i = 0; i < BITS_IN_SEPARATOR_TONE; i++) {
        audio_write_bit_one(file);
    }
}

void write_patches(SNDFILE* file, JX3P_PATCH_COLLECTION* patches) {
    for (int bank = 0; bank < 2; bank++) {
        write_pilot_tone(file);
        for (int patch = 0; patch < 16; patch++) {
            write_patch(file, &patches->data[bank][patch]);
            write_separator(file);
            write_patch(file, &patches->data[bank][patch]);
            write_separator(file);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("USAGE: %s <input.csv> <output.wav>\n", argv[0]);
        return 1;
    }

    char* input_path  = argv[1];
    char* output_path = argv[2];

    SF_INFO info = {
        .samplerate = 44100,
        .channels = 1,
        .format = SF_FORMAT_WAV | SF_FORMAT_PCM_16
    };
    SNDFILE* file = NULL;

    file = sf_open(output_path, SFM_WRITE, &info);
    if (!file) {
        printf("%s\n", sf_strerror(file));
        return 1;
    }

    // printf("frames: %lld, samplerate: %d, channels: %d, format: %d, sections: %d, seekable: %d\n",
    //     info.frames, info.samplerate, info.channels, info.format, info.sections, info.seekable);

    JX3P_PATCH_COLLECTION patches;
    FILE *csv_file = fopen(input_path, "r");
    if (!csv_file) {
        printf("Unable to open input CSV file: %s\n", input_path);
        return 1;
    }
    parse_csv(&patches, csv_file);
    fclose(csv_file);
    write_patches(file, &patches);

    int error = sf_close(file);
    if (error) {
        printf("Unable to close the file: %s\n", sf_error_number(error));
        return 1;
    }

    return 0;
}