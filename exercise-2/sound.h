#include <inttypes.h>   // TODO : Seems like we only need one of these..
#include <stdint.h>
#define NO_EVENT 0x00000000

#define EV_TYPE  0
#define EV_TIME  1
#define EV_INST  2
#define EV_FREQ  3

// All of this can be automatically generated (or read) by the python 'protocol' script
// (sco_to_hex.py)
#define TYPE_MASK 0x00000001
#define TIME_MASK 0x000007FF
#define INST_MASK 0x00000003
#define FREQ_MASK 0x000003FF
#define TIME_POS 1      // Positions for where the relevant bits start...
#define INST_POS 12     // ...i.e. how much to right shift 
#define FREQ_POS 14
#define NUM_GENERATORS 4
#define SEQ_TERMINATOR 0x00000000
#define SEQ_CLOCK_HZ 1000
/*** For MCU "simulation" ***/
#define MAXVAL16 65535
#define MAXVAL32 4294967295
#define GEN_HIGH 2047      /* Determines the upper limit on the generator volume */
#define GEN_LOW -2048      /* The lowest value of generators. **ASSUMING 2'S COMPLEMENT** */
#define GEN_RANGE 4096     /* The number of possible values in the DAG (12 bits) */
// Timers
#define CPU_CLOcK_HZ 14000000
#define AUDIO_HZ 8000
#define SEQ_HZ 1000
#define TIMER_AUDIO_TOP (CPU_CLOcK_HZ / AUDIO_HZ)
#define TIMER_SEQ_TOP   (CPU_CLOcK_HZ / SEQ_HZ)
#define WT_SIZE 32
#ifndef _SOUND_H_
#define _SOUND_H_
/****************************/

//  extern because the sequence is nice to have in its own file, and it's read only.
extern const uint32_t seq[];        // Our sequence is defined in seqs.c (hard coded)
extern const int16_t WAVETABLE[];   // We also hard code a wave table
void generator_setup();             // Not really necessary to make a function for this :v

/* The generator functions should only be used by functions in sound.c for the final
 * program, but can be used for testing the generators from the main files first. */
enum {SAW, SQUARE, TRIANGLE, WT};
// enum {SQUARE, SAW, TRIANGLE};
void generator_start(uint32_t generator, uint32_t freq);
void generator_stop(uint32_t generator);
/* audio_update should be called in our audio timer interrupt handler to get the 
 * value of the latest audio sample. */
int16_t audio_update();   
void generate_sweep(uint32_t gen, uint32_t num_samples_in_sweep, uint32_t fstart, uint32_t fend);

void sequencer_start(const uint32_t* seq_to_play);
void sequencer_stop();
void sequencer_update();


#endif

