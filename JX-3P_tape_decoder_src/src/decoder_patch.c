/*
 * decoder_patch.c
 * JX3P patch format decoder
 * Convert bitstream into decoded JX3P patch/bank collection
 * 
 * See README.txt for license and installation notes.
*/

// decoder state
#define DS_SEARCHING	0
#define DS_COLLECTING	1
#define DS_DECODING		2


#include "decoder_patch.h"
#include <stdio.h>
#include <string.h>


/* MIDI stuff */

//#define MIDI_CHUNK_BUFFER_LEN	8192	// MIDI chunk buffer
//#define MIDI_PPQN	96				// MIDI Parts Per Quarter Note
//unsigned char chunk_buffer[MIDI_CHUNK_BUFFER_LEN];
//unsigned int chunk_buffer_pos = 0;
//void append_to_chunk();	// decide params


Decoder_State init_decoder_state()
{
	Decoder_State ds;

	ds.bucket_pos = 0;
	ds.one_count = 0;
	ds.state = DS_COLLECTING;
	
	return ds;
}


Patch_Collection init_patch_collection()
{
	Patch_Collection pc;
	unsigned short int bnk, pch, reg;
	for (bnk=0; bnk<2; bnk++)
	{
		for (pch=0; pch<16; pch++)
		{
			jx3p_format pd;
			for (reg=0; reg<26; reg++)
			{
				pd.raw[reg] = 0;
			}
			pc.data[bnk][pch] = pd;
		}
	}
	return pc;
}


jx3p_format * get_patch(Patch_Collection * pc, int bnk, int pch)
{
	// TODO: maybe return NULL if empty? Not sure.
	return &pc->data[bnk % 2][pch % 16];
}


int set_patch(Patch_Collection * pc, int bnk, int pch, jx3p_format * pd)
{
	pc->data[bnk % 2][pch % 16] = *pd;
	return 1;	// why am I returning anything?
}

/* FIXME: algorithm refactor notes:
 * 
 * count 1 bits (one_count); ARM searching=1 after 11. scan until next 0 and FIRE (searching=0): SOD
 * decode and checksum record
 * if valid, and if no other record in collection at record's bank and patch, stuff it
 * start searching for next record
 * (SO WHAT if I waste time decoding copy 2 on records where copy 1 is correct?)
 * Any time I get 11x1, even while collecting a bucketful of bits, that means I'm between patches. Signal noise/dropout has caused the detector/demodulator to collapse a string of missing bits into a single bad bit. Empty bucket and search.
 */


/* gateway for decode process.
 * inputs: bitbucket, bitbucket length, decoder_state struct, patch collection
 * output: count of decoded patches per iteration
 */
int decode_bitstream(int bits[], int bits_len, Decoder_State *ds, Patch_Collection *patcol )
{
	int i;
	int patches_decoded = 0;

	// iterate through input bits
	for (i=0; i < bits_len; i++)
	{	// if we find 11x1 bits (impossible with normal serial start bit of 0),
		// we must be searching through intertone for the next 0, which is the start of the next record
		// at which point we start collecting until buffer full (then decode) or surprise 11x1
		// all records with bad checksums are dropped
		// good records are stored in the bank/patch defined in the record

		if (bits[i])
		{	// increment 1 bit count
			ds->one_count++;
		}
		else
		{	// clear 1 bit count, test if searching, then start collecting
			ds->one_count = 0;
			if (ds->state == DS_SEARCHING) {
				ds->state = DS_COLLECTING;
			}
		}

		if ( (ds->one_count > 11) && (ds->state != DS_SEARCHING) )
		{	// if 11x1 in middle of non-search, abandon and start search
			fprintf(stderr, "Data alignment error; possible dropout or noise.\n");
			//fprintf(stderr, "Arming search early (prior state=%d)\n", ds->state);
			ds->bucket_pos = 0;
			ds->state = DS_SEARCHING;
		}

		if (ds->state == DS_COLLECTING)
		{
			// append bit to queue
			ds->bucket[ds->bucket_pos++] = bits[i];
			// if queue now full
			if (ds->bucket_pos >= PATCH_DATA_LENGTH)
			{
				// convert queue to patch
				ds->state = DS_DECODING;
				jx3p_format cp;
				int valid_convert;
				valid_convert = convert_patch(ds, &cp);
				if (valid_convert)
				{
					unsigned short int bnk = (cp.bank_cd - 2) % 2;
					unsigned short int pch = cp.patch_num % 16;
					jx3p_format * op = get_patch(patcol, bnk, pch);
					if (op->datatype != 2)	// if not yet set
					{	// include patch into collection
						set_patch(patcol, bnk, pch, &cp);
						patches_decoded++;
					}
				}
				ds->bucket_pos = 0;
				ds->state = DS_SEARCHING;
			}
		}
	}

	return patches_decoded;
}

// perform patch bitstream conversion into struct
int convert_patch(Decoder_State *ds, jx3p_format *cp)
{
	// iterate through cache in 11-bit chunks, reverse, and pack bits into struct
	unsigned short int by, bi;	// byte and bit indices
	unsigned char dc;	// decoded byte
	unsigned char checksum = 0;

	for (by=0; by < 26; by++)
	{
		dc=0;	// reset decoded byte
		for (bi=8; bi>0; bi--)
		{
			dc |= ds->bucket[(by*11)+bi] << (bi-1);
		}
		cp->raw[by] = dc;
	}
	for (by=0; by < 25; by++) {
		checksum += cp->raw[by];
	}
	return (checksum == (unsigned char) cp->raw[25]);
}


/* CSV output functions */

// return char array representing CSV header (no newline)
char * print_csv_header()
{
	return "\
patch, \
A01 (DCO-1 Range), A02 (DCO-1 Waveform), A03 (DCO-1 LFO Mod), A04 (DCO-1 ENV Mod), \
A05 (DCO-2 Range), A06 (DCO-2 Waveform), A07 (DCO-2 Cross Modulation), A08 (DCO-2 Tune), \
A09 (DCO-2 Fine Tune), A10 (DCO-2 LFO Mod), A11 (DCO-2 ENV Mod), A12 (DCO LFO Mod), \
A13 (DCO ENV Mod), A14 (DCO ENV polarity), A15 (VCF Mix), A16 (VCF High Pass), \
B01 (VCF Cutoff Frequency), B02 (VCF LFO Mod), B03 (VCF Pitch Follow), B04 (VCF Resonance), \
B05 (VCF ENV Mod), B06 (VCF ENV polarity), B07 (VCA Mode), B08 (VCA Level), \
B09 (Chorus off/on), B10 (LFO Waveform), B11 (LFO Delay), B12 (LFO Rate), \
B13 (ENV Attack), B14 (ENV Decay), B15 (ENV Sustain), B16 (ENV Release)";
}


// return char string representing formatted patch parameters (no newline)
char * print_csv_patch(jx3p_format * p)
{
	if (p->datatype != 2)	// if unset
	{
		return "x,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,-";
	}

	static char pm[33 * 16];	// long enough for 33 x 15 chars

	// bank/patch location
	sprintf(pm,				 "%s, ", patch_identifier(p->bank_cd, p->patch_num));
	// dco1 params	[A01-A04]
	sprintf(pm + strlen(pm), "%s, ", dco_range(p->dco1_range));
	sprintf(pm + strlen(pm), "%s, ", dco1_waveform(p->dco1_waveform));
	sprintf(pm + strlen(pm), "%d, ", p->dco1_fmod_lfo);
	sprintf(pm + strlen(pm), "%d, ", p->dco1_fmod_env);
	// dco2 params	[A05-A11]
	sprintf(pm + strlen(pm), "%s, ", dco_range(p->dco2_range));
	sprintf(pm + strlen(pm), "%s, ", dco2_waveform(p->dco2_waveform));
	sprintf(pm + strlen(pm), "%s, ", dco_crossmod(p->dco2_crossmod));
	sprintf(pm + strlen(pm), "%d, ", p->dco2_tune);
	sprintf(pm + strlen(pm), "%d, ", p->dco2_fine_tune);
	sprintf(pm + strlen(pm), "%d, ", p->dco2_fmod_lfo);
	sprintf(pm + strlen(pm), "%d, ", p->dco2_fmod_env);
	// lfo, env amount, polarity	[A12-A14]
	sprintf(pm + strlen(pm), "%d, ", p->dco_lfo_amount);
	sprintf(pm + strlen(pm), "%d, ", p->dco_env_amount);
	sprintf(pm + strlen(pm), "%s, ", env_polarity(p->dco_env_polarity));
	// vcf param	[A15-A16, B01-B06]
	sprintf(pm + strlen(pm), "%d, ", p->vcf_mix);
	sprintf(pm + strlen(pm), "%d, ", p->vcf_hpf);
	sprintf(pm + strlen(pm), "%d, ", p->vcf_cutoff);
	sprintf(pm + strlen(pm), "%d, ", p->vcf_lfo_mod);
	sprintf(pm + strlen(pm), "%d, ", p->vcf_pitch_follow);
	sprintf(pm + strlen(pm), "%d, ", p->vcf_resonance);
	sprintf(pm + strlen(pm), "%d, ", p->vcf_env_mod);
	sprintf(pm + strlen(pm), "%s, ", env_polarity(p->vcf_env_polarity));
	// vca, chorus	[B07-B09]
	sprintf(pm + strlen(pm), "%s, ", vca_mode(p->vca_mode));
	sprintf(pm + strlen(pm), "%d, ", p->vca_level);
	sprintf(pm + strlen(pm), "%s, ", (p->chorus)?"on":"off");
	// lfo params	[B10-B12]
	sprintf(pm + strlen(pm), "%s, ", lfo_waveform(p->lfo_waveform));
	sprintf(pm + strlen(pm), "%d, ", p->lfo_delay);
	sprintf(pm + strlen(pm), "%d, ", p->lfo_rate);
	// env ADSR		[B13-B16]
	sprintf(pm + strlen(pm), "%d, ", p->env_attack);
	sprintf(pm + strlen(pm), "%d, ", p->env_decay);
	sprintf(pm + strlen(pm), "%d, ", p->env_sustain);
	sprintf(pm + strlen(pm), "%d", p->env_release);

	return pm;
}

/* text label translators */


char * patch_identifier(int bank, int n)
{	static char label[] = "bnn";
	sprintf( label, "%c%02d", ('A'+bank%4), (n%16)+1);
	return label;
}

char * dco_range(int n)
{	static char *label[] = {"16'","8'","4'"};
	return label[n%3];
}

char * dco1_waveform(int n)
{	static char *label[] = {"saw","pulse","square"};
	return label[n%3];
}

char * dco2_waveform(int n)
{	static char *label[] = {"saw","pulse","square","noise"};
	return label[n%4];
}

char * dco_crossmod(int n)
{	static char *label[] = {"off","sync","metal"};
	return label[n%3];
}

char * lfo_waveform(int n)
{	static char *label[] = {"sine","square","random","fast random"};
	return label[n%4];
}

char * env_polarity(int n)
{	static char *label[] = {"neg","pos"};
	return label[n%2];
}

char * vca_mode(int n)
{	static char *label[] = {"gate","env"};
	return label[n%2];
}
