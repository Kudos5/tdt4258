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
#ifndef _SOUND_H_
#define _SOUND_H_
/****************************/


/***** Functions *****/
void generator_setup();
void set_gen_freq(uint32_t generator, uint32_t frequency);
void start_generator(uint32_t generator);
void stop_generator(uint32_t generator);
void audio_interrupt();
void trigger_sequence(const uint32_t* seq_to_play);
void sequencer_interrupt();

extern const uint32_t seq[];
//  extern because the sequence is nice to have in its own file, and it's read only.

#endif

