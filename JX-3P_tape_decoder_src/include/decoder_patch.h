/*
 * decoder_patch.h
 * JX3P patch format decoder
 * Convert bitstream into decoded JX3P patch/bank collection
 * 
 * See README.txt for license and installation notes.
 */


#define PATCH_RECORD_LENGTH		334	// full record length
#define PATCH_DATA_LENGTH		286 // data (non-padding)
#define PATCH_PADDING_LENGTH	48	// padding of 1s after record


// Decoder state
typedef struct {
	unsigned int bucket[PATCH_DATA_LENGTH];	// bit bucket (when full, do decode)
	unsigned int bucket_pos;		// current position in bucket
	unsigned short int one_count;	// if > 11x1, must be tone; state=DS_SEARCHING
	unsigned short int state;		// decoder state (searching, collecting, decoding)
} Decoder_State;


// jx3p_format
typedef union {
	struct {
		// 00
		unsigned int 					: 6;	// [0-5] unused
		unsigned int datatype			: 2;	// [6-7] Data type (2=patch, 3=seq)
		// 01
		unsigned int bank_ab			: 2;	// [0-1] A/B bank
		unsigned int 					: 6;	// [2-7] unused
		// 02
		unsigned int bank_cd			: 2; 	// [0-1] C/D bank indication C=2, D=3
		unsigned int 					: 6;	// [2-7] unused
		// 03
		unsigned int patch_num			: 4;	// [0-3] patch number (0~15)
		unsigned int 					: 4;	// [4-7] unused
		// 04
		unsigned int dco1_range			: 2;	// [0-1] A01 DCO-1 range (16', 8', 4')
		unsigned int dco1_waveform		: 2;	// [2-3] A02 DCO-1 waveform (saw, pulse, square)
		unsigned int dco2_range			: 2;	// [4-5] A05 DCO-2 range (16', 8', 4')
		unsigned int dco2_waveform		: 2;	// [6-7] A06 DCO-2 waveform (saw, pulse, square, noise)
		// 05
		unsigned int dco2_crossmod		: 2;	// [0-1] A07 DCO-2 cross modulation 0=off 1=sync 2=metal
		unsigned int vcf_env_polarity	: 1;	// [2] B06 VCF Env polarity (0=positive, 1=negative?)
		unsigned int vca_mode			: 1;	// [3] B07 VCA Mode (0=gate, 1=envelope)
		unsigned int dco2_fmod_env		: 1; 	// [4] A11 DCO-2 ENV Mod off/on
		unsigned int dco2_fmod_lfo		: 1;	// [5] A10 DCO-2 LFO Mod off/on
		unsigned int dco1_fmod_env		: 1;	// [6] A04 DCO-1 ENV Mod off/on
		unsigned int dco1_fmod_lfo		: 1;	// [7] A03 DCO-1 LFO Mod off/on
		// 06
		unsigned int lfo_waveform		: 2;	// [0-1] B10 LFO Waveform (saw, square, random, fast random)
		unsigned int dco_env_polarity	: 1;	// [2] A14 DCO ENV Polarity (0=negative, 1=positive)
		unsigned int chorus				: 1;	// [3] B09 Chorus off/on
		// The purpose of these bits is unknown to me (jviikki). When saving patches to tape memory,
		// these bits are also transmitted. They have to remain as they were in order for
		// verify function of JX-3P to work. I want to store these bits as well to prevent loss of
		// information when converting from CSV file into audio.
		unsigned int mystery			: 4;	// [4-7] Unknown
		// 07
		unsigned int dco2_fine_tune		: 8;	// [0-7] A09 DCO-2 Fine Tune
		// 08
		unsigned int dco2_tune			: 8;	// [0-7] A08 DCO-2 Tune
		// 09
		unsigned int dco_env_amount		: 8;	// [0-7] A13 DCO ENV Mod amount
		// 10
		unsigned int dco_lfo_amount		: 8;	// [0-7] A12 DCO LFO Mod amount
		// 11
		unsigned int vcf_mix			: 8;	// [0-7] A15 VCF Mix
		// 12
		unsigned int vcf_hpf			: 8;	// [0-7] A16 VCF HPF
		// 13
		unsigned int vcf_resonance		: 8;	// [0-7] B04 VCF Resonance
		// 14
		unsigned int vcf_cutoff			: 8;	// [0-7] B01 VCF Cutoff Freq
		// 15
		unsigned int vcf_env_mod		: 8;	// [0-7] B05 VCF ENV Mod
		// 16
		unsigned int vcf_lfo_mod		: 8;	// [0-7] B02 VCF LFO Mod
		// 17
		unsigned int vcf_pitch_follow	: 8;	// [0-7] B03 VCF Pitch Follow
		// 18
		unsigned int vca_level			: 8;	// [0-7] B08 VCA Level
		// 19
		unsigned int lfo_rate			: 8;	// [0-7] B12 LFO Rate
		//20
		unsigned int lfo_delay			: 8;	// [0-7] B11 LFO Delay
		// 21
		unsigned int env_attack			: 8;	// [0-7] B13 ENV Attack
		// 22
		unsigned int env_decay			: 8;	// [0-7] B14 ENV Decay
		// 23
		unsigned int env_sustain		: 8;	// [0-7] B15 ENV Sustain
		// 24
		unsigned int env_release		: 8;	// [0-7] B16 ENV Release
		// 25
		unsigned int checksum 			: 8;	// [0-7] checksum (sum of all bytes)
	};
	unsigned char raw[26];
} jx3p_format;


// master storage for all patches by bank
typedef struct {
	jx3p_format data [2][16];
} Patch_Collection;


// function prototypes

Patch_Collection init_patch_collection();

Decoder_State init_decoder_state();

jx3p_format * get_patch(Patch_Collection * pc, int bank, int patch);
int set_patch(Patch_Collection * pc, int bank, int patch, jx3p_format * patchdata);

int decode_bitstream(int bits[], int bit_len, Decoder_State *ds, Patch_Collection *pc);
int convert_patch(Decoder_State *ds, jx3p_format *cp);

char * print_csv_header();
char * print_csv_patch(jx3p_format * p);

// text label translators

char * patch_identifier(int cd, int n);

char * dco_range(int n);

char * dco1_waveform(int n);

char * dco2_waveform(int n);

char * dco_crossmod(int n);

char * lfo_waveform(int n);

char * env_polarity(int n);

char * vca_mode(int n);
