#include "sndfile.h"

const short AUDIO_BIT_ZERO[] = {
    12244,
    19836,
    19449,
    16342,
    14058,
    2774,
    -15558,
    -19175,
    -18706,
    -15082,
    -13165
};

const short AUDIO_BIT_ONE[] = {
    9644,
    19672,
    19315,
    17117,
    13591,
    11004,
    8259,
    6310,
    4539,
    3152,
    2081,
    1008,
    374,
    -343,
    -646,
    -1094,
    -1375,
    -1550,
    -1882,
    -1716,
    -2225,
    -1727,
    -2518,
    -1356,
    -5901,
    -22039,
    -28390,
    -25740,
    -22163,
    -17463,
    -14003,
    -10592,
    -8088,
    -5874,
    -4142,
    -2784,
    -1503,
    -635,
    122,
    706,
    1060,
    1571,
    1629,
    2142,
    1972,
    2407,
    2262,
    2410,
    2418,
    2643
};

sf_count_t audio_write_all(SNDFILE* file, const short* samples, sf_count_t len) {
    sf_count_t total_bytes_written = 0;
    sf_count_t bytes_written = 0;
    do {
        bytes_written = sf_write_short(file, samples + total_bytes_written, len);
        total_bytes_written += bytes_written;
    } while(bytes_written > 0 && total_bytes_written < len);
    return total_bytes_written;
}

int audio_write(SNDFILE* file, const short* samples, sf_count_t len) {
    return audio_write_all(file, samples, len) == len ? 0 : -1;
}

int audio_write_bit_zero(SNDFILE* file) {
    return audio_write(file, &AUDIO_BIT_ZERO[0], sizeof(AUDIO_BIT_ZERO) / sizeof(short));
}

int audio_write_bit_one(SNDFILE* file) {
    return audio_write(file, &AUDIO_BIT_ONE[0], sizeof(AUDIO_BIT_ONE) / sizeof(short));
}
