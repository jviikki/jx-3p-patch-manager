#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "patch.h"

void print_byte(unsigned char byte) {
    printf("0");

    for (unsigned int i = 0; i < 8; i++) {
        if (byte & (1 << i))
            printf("1");
        else
            printf("0");
    }

    printf("1");
    printf("1");
}

void print_patch(JX3P_PATCH* patch) {
    // Disregard the last byte which is the checksum
    int num_bytes = sizeof(patch->raw) / sizeof(unsigned char) - 1;
    unsigned char checksum = 0;

    for (int i = 0; i < num_bytes; i++) {
        print_byte(patch->raw[i]);
        checksum += patch->raw[i];
    }

    print_byte(checksum);
}

void populate_test_patch(JX3P_PATCH* patch, unsigned int bank_num, unsigned int patch_num) {
    bzero(patch->raw, sizeof(patch->raw) / sizeof(unsigned char));
    // 6 bits
    patch->datatype = 2; // 2 bits

    patch->bank_ab = bank_num; // 2 bits
    // 6 bits

    patch->bank_cd = bank_num + 2; // 2 bits
    // 6

    patch->patch_num = patch_num; // 4
    // 4

    patch->dco1_range = 0; // 2
    patch->dco1_waveform = 0; // 2
    patch->dco2_range = 1; // 2
    patch->dco2_waveform = 1; // 2

    patch->dco2_crossmod = 0; // 2
    patch->vcf_env_polarity = 1; // 1
    patch->vca_mode = 1; // 1
    patch->dco2_fmod_env = 1; // 1
    patch->dco2_fmod_lfo = 0; // 1
    patch->dco1_fmod_env = 0; // 1
    patch->dco1_fmod_lfo = 1; // 1

    patch->lfo_waveform = 0; // 2
    patch->dco_env_polarity = 1; // 1
    patch->chorus = 1; // 1
    // 4

    patch->dco2_fine_tune = 235; // 8
    patch->dco2_tune = 0; // 8
    patch->dco_env_amount = 0; // 8
    patch->dco_lfo_amount = 29; // 8
    patch->vcf_mix = 125; // 8
    patch->vcf_hpf = 0; // 8
    patch->vcf_resonance = 149; // 8
    patch->vcf_cutoff = 131; // 8
    patch->vcf_env_mod = 254; // 8
    patch->vcf_lfo_mod = 0; // 8
    patch->vcf_pitch_follow = 196; // 8
    patch->vca_level = 254; // 8
    patch->lfo_rate = 185; // 8
    patch->lfo_delay = 0; // 8
    patch->env_attack = 0; // 8
    patch->env_decay = 103; // 8
    patch->env_sustain = 79; // 8
    patch->env_release = 106; // 8

    // This field is calculated from other content
    patch->checksum = 0; // 8

    // print_patch(patch);
}

void parse_row(char* line, JX3P_PATCH* patch, unsigned int bank_num, unsigned int patch_num) {
    bzero(&patch->raw, sizeof(patch->raw) / sizeof(unsigned char));
    // if (bank_num == 0 && patch_num >= 7 && patch_num <= 10) {
    //     patch->raw[6] = 0xFF;
    // } else if (bank_num == 0 && patch_num == 14) {
    //     patch->raw[6] = 0x10;
    // } else if (bank_num == 1 && patch_num == 8) {
    //     patch->raw[6] = 0xFF;
    // }

    patch->datatype = 2;
    patch->bank_ab = bank_num;
    patch->bank_cd = bank_num + 2;
    patch->patch_num = patch_num;

    char *field = strtok(line, ",");
    int column = 0;
    while (field) {
        switch (column) {
            case 1:
                if (!strcmp(" 16'", field)) {
                    patch->dco1_range = 0;
                } else if (!strcmp(" 8'", field)) {
                    patch->dco1_range = 1;
                } else { // 4'
                    patch->dco1_range = 2;
                }
                break;
            case 2:
                if (!strcmp(" saw", field)) {
                    patch->dco1_waveform = 0;
                } else if (!strcmp(" pulse", field)) {
                    patch->dco1_waveform = 1;
                } else { // square
                    patch->dco1_waveform = 2;
                }
                break;
            case 3:
                patch->dco1_fmod_lfo = atoi(field);
                break;
            case 4:
                patch->dco1_fmod_env = atoi(field);
                break;
            case 5:
                if (!strcmp(" 16'", field)) {
                    patch->dco2_range = 0;
                } else if (!strcmp(" 8'", field)) {
                    patch->dco2_range = 1;
                } else { // 4'
                    patch->dco2_range = 2;
                }
                break;
            case 6:
                if (!strcmp(" saw", field)) {
                    patch->dco2_waveform = 0;
                } else if (!strcmp(" pulse", field)) {
                    patch->dco2_waveform = 1;
                } else if (!strcmp(" square", field)) {
                    patch->dco2_waveform = 2;
                } else { // noise
                    patch->dco2_waveform = 3;
                }
                break;
            case 7:
                if (!strcmp(" off", field)) {
                    patch->dco2_crossmod = 0;
                } else if (!strcmp(" sync", field)) {
                    patch->dco2_crossmod = 1;
                } else { // metal
                    patch->dco2_crossmod = 2;
                }
                break;
            case 8:
                patch->dco2_tune = atoi(field);
                break;
            case 9:
                patch->dco2_fine_tune = atoi(field);
                break;
            case 10:
                patch->dco2_fmod_lfo = atoi(field);
                break;
            case 11:
                patch->dco2_fmod_env = atoi(field);
                break;
            case 12:
                patch->dco_lfo_amount = atoi(field);
                break;
            case 13:
                patch->dco_env_amount = atoi(field);
                break;
            case 14:
                if (!strcmp(" neg", field)) {
                    patch->dco_env_polarity = 0;
                } else { // pos
                    patch->dco_env_polarity = 1;
                }
                break;
            case 15:
                patch->vcf_mix = atoi(field);
                break;
            case 16:
                patch->vcf_hpf = atoi(field);
                break;
            case 17:
                patch->vcf_cutoff = atoi(field);
                break;
            case 18:
                patch->vcf_lfo_mod = atoi(field);
                break;
            case 19:
                patch->vcf_pitch_follow = atoi(field);
                break;
            case 20:
                patch->vcf_resonance = atoi(field);
                break;
            case 21:
                patch->vcf_env_mod = atoi(field);
                break;
            case 22:
                if (!strcmp(" neg", field)) {
                    patch->vcf_env_polarity = 0;
                } else { // pos
                    patch->vcf_env_polarity = 1;
                }
                break;
            case 23:
                if (!strcmp(" gate", field)) {
                    patch->vca_mode = 0;
                } else { // env
                    patch->vca_mode = 1;
                }
                break;
            case 24:
                patch->vca_level = atoi(field);
                break;
            case 25:
                patch->chorus = !strcmp(field, " on") ? 1 : 0;
                break;
            case 26:
                if (!strcmp(" sine", field)) {
                    patch->lfo_waveform = 0;
                } else if (!strcmp(" square", field)) {
                    patch->lfo_waveform = 1;
                } else if (!strcmp(" random", field)) {
                    patch->lfo_waveform = 2;
                } else { // fast random
                    patch->lfo_waveform = 3;
                }
                break;
            case 27:
                patch->lfo_delay = atoi(field);
                break;
            case 28:
                patch->lfo_rate = atoi(field);
                break;
            case 29:
                patch->env_attack = atoi(field);
                break;
            case 30:
                patch->env_decay = atoi(field);
                break;
            case 31:
                patch->env_sustain = atoi(field);
                break;
            case 32:
                patch->env_release = atoi(field);
                break;
            default:
                break;
        }

        field = strtok(NULL, ",");
        column++;
    }
}

void parse_csv(JX3P_PATCH_COLLECTION* patches, FILE* file) {
    char line[2048];
    fgets(line, 2048, file); // header
    for (unsigned int bank = 0; bank < 2; bank++) {
        for (unsigned int patch = 0; patch < 16; patch++) {
            fgets(line, 2048, file);
            // populate_test_patch(&patches->data[bank][patch], bank, patch);
            parse_row(line, &patches->data[bank][patch], bank, patch);
        }
    }
}