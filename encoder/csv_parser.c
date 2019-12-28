#include <stdio.h>
#include "patch.h"

void print_byte(unsigned char byte) {
    for (int j = 7; j >= 0; j--) {
        printf("%d", !!((byte << j) & 0x80));
    }
    printf("\n");
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

void populate_test_patch(JX3P_PATCH* patch) {
    // 6 bits
    patch->datatype = 2; // 2 bits

    patch->bank_ab = 0; // 2 bits
    // 6 bits

    patch->bank_cd = 2; // 2 bits
    // 6

    patch->patch_num = 0; // 4
    // 4

    patch->dco1_range = 0; // 2
    patch->dco1_waveform = 1; // 2
    patch->dco2_range = 0; // 2
    patch->dco2_waveform = 2; // 2

    patch->dco2_crossmod = 1; // 2
    patch->vcf_env_polarity = 1; // 1
    patch->vca_mode = 0; // 1
    patch->dco2_fmod_env = 1; // 1
    patch->dco2_fmod_lfo = 1; // 1
    patch->dco1_fmod_env = 0; // 1
    patch->dco1_fmod_lfo = 1; // 1

    patch->lfo_waveform = 0; // 2
    patch->dco_env_polarity = 0; // 1
    patch->chorus = 1; // 1
    // 4

    patch->dco2_fine_tune = 99; // 8
    patch->dco2_tune = 129; // 8
    patch->dco_env_amount = 174; // 8
    patch->dco_lfo_amount = 0; // 8
    patch->vcf_mix = 254; // 8
    patch->vcf_hpf = 0; // 8
    patch->vcf_resonance = 0; // 8
    patch->vcf_cutoff = 146; // 8
    patch->vcf_env_mod = 160; // 8
    patch->vcf_lfo_mod = 0; // 8
    patch->vcf_pitch_follow = 172; // 8
    patch->vca_level = 45; // 8
    patch->lfo_rate = 202; // 8
    patch->lfo_delay = 0; // 8
    patch->env_attack = 0; // 8
    patch->env_decay = 155; // 8
    patch->env_sustain = 121; // 8
    patch->env_release = 0; // 8
    // TODO: this needs to be calculated
    patch->checksum = 236; // 8

    print_patch(patch);
}