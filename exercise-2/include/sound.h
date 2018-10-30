#include <stdint.h>

#define AUDIO_HZ 8000
#define SEQ_HZ 1000
#define SEQ_TERMINATOR 0x00000000

#ifndef _SOUND_H_
#define _SOUND_H_
/* Sequences are kept as constant arrays in the file seqs.c */
extern const uint32_t seq[];
extern const uint32_t seq2[];
extern const uint32_t seq3[];
extern const uint32_t seq4[];
/* Wavetables are also hard coded in 'seqs.c' */
extern const int16_t WAVETABLE[];
extern const int16_t WAVETABLE_NOISE[];
void generator_setup();

enum { SAW, SQUARE, NOISE, WT };
void generator_start(uint32_t generator, uint32_t freq);
void generator_stop(uint32_t generator);
void generate_sweep(uint32_t gen, uint32_t num_samples_in_sweep,
		    uint32_t fstart, uint32_t fend);
int16_t audio_update();

void sequencer_start(const uint32_t * seq_to_play);
void sequencer_stop();
void sequencer_update();

// Stop all generators and the sequencer
void DisableSound();
#endif
