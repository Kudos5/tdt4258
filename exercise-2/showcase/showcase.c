/**
 * Demonstrates the general sequencer idea and how parts of it could be implemented
 * Build by running    `gcc -o seq sequencer.c`
 */
#include <stdio.h>
#include <sndfile.h>
#include <stdlib.h>
#include "sound.h"
#include "showcase.h"

/* Global variables must be declared here, not in the header */
uint32_t seq_counter = 0;          // Simulates sequencer timer clock
uint32_t aud_counter = 0;          // Simulates audio timer clock
uint64_t cpu_counter = 0;          // Simulates the CPU clock
uint64_t sim_counter = 0;          // For keeping track of position in the wave array
volatile uint32_t* TIMER_AUD_CNT = &aud_counter;
volatile uint32_t* TIMER_SEQ_CNT = &seq_counter;

/** Write to WAV using libsndfile **/
void write_wav(int16_t* wave_array, uint32_t wave_len) {
    struct SF_INFO sf_info = {
        .frames = wave_len,
        .samplerate = AUDIO_HZ,
        .channels = 1,
        .format = SF_FORMAT_WAV | SF_FORMAT_PCM_16,
    };
    SNDFILE* ofile = sf_open("simulated.wav", SFM_WRITE, &sf_info);
    // int16 is the same as short, so use the sf_write_short for this 
    sf_write_short(ofile, (const int16_t*) wave_array, wave_len);
    sf_close(ofile);
}


void simulation(uint32_t len_wave_file) {
    /** 
     * Function that simulates the execution on MCU 
     */
    sequencer_start(seq);
    for (cpu_counter = 0; cpu_counter < MAXVAL32; cpu_counter++) {
        if (cpu_counter % TIMER_SEQ_TOP == 0) {
        /***** This simulates interrupts by our sequencer timer *****/
            sequencer_update();
            // update the counter [done automatically by MCU]
            if (++seq_counter >= MAXVAL16) {
                seq_counter = 0;
            }
        /************************************************************/
        }
        /***** This simulates interrupts by our audio timer *****/
        if (cpu_counter % TIMER_AUDIO_TOP == 0) {
            if (sim_counter >= len_wave_file)
                return; //stop simulation when we've reached the length of the wav
            wave_samples[sim_counter] = 16*audio_update(); // The '16' is just to scale from 12 to 16 bits for showcasing. 
            // update the counters
            if (++aud_counter >= MAXVAL16) {
                aud_counter = 0;
            }
            sim_counter++;
        /********************************************************/
        }
    }
}


    /* TODO: We should *not* use event type at position 0, as this makes us use
     * one more right shift operation than necessary. Right shifting is not necessary
     * unless we are actually going to use the resulting value. The event type bit is
     * simply used in a comparison, not as a value itself, so you don't have to right shift
     * for this one.
     *
     * The same can be true about the instrument field, *unless* we use an array of pointers where
     * each pointer points to the address of a different generator function.
     * https://cs.nyu.edu/courses/spring12/CSCI-GA.3033-014/Assignment1/function_pointers.html
     *
     * The question is what is more efficient out of these methods for triggering a generator:
     *  1.  Bitmasking the inst bits where they are, then comparing to some predefined values to
     *      determine which generators to use. (Using a 'switch' statement, I guess).
     *  2.  Right shifting, bitmasking and using the resulting index to look up the generator function.
     */


// SHOWCASE STUFF. Move to its own h-file
void print_event(uint64_t simc, uint16_t seqc, uint32_t in, char* ty, uint32_t ti, uint32_t fre){
    printf("sim_ctr: %"PRId64"\t seq_ctr:%"PRId16"\tinst #%d %s, time_diff=%d, freq=%d \n", 
        simc, seqc, in, ty, ti, fre);
}


void empty_wave(uint32_t length){
    wave_samples = malloc(length*sizeof(int16_t));
    for (uint32_t n = 0; n < length; n++){
        wave_samples[n] = 0;
    }
}

int main(int argv, char **argc)
{

    generator_setup();
	uint32_t *current_event = NO_EVENT;
  	//showcase_sequence(seq, current_event);
    
    uint32_t length_of_wav = AUDIO_HZ*30;
    if (argv > 1) {
        printf("%d\n",atoi(argc[1]));
        length_of_wav = atoi(argc[1]);
    }
    empty_wave(length_of_wav);
    simulation(length_of_wav);
    for (int i = 0; i < length_of_wav; i++){
        printf("snd[%d] = %d\n", i, wave_samples[i]);
    }
    write_wav(wave_samples, length_of_wav);
    free(wave_samples);

	return 0;
}
