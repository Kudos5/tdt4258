/**
 * Demonstrates the general sequencer idea and how parts of it could be implemented
 */
#include <stdio.h>
#include <sndfile.h>
#include <stdlib.h>
#include <string.h>
#include "sound.h"
#include "showcase.h"

/* Global variables must be declared here, not in the header */
uint32_t seq_counter = 0;          // Simulates sequencer timer clock
uint32_t aud_counter = 0;          // Simulates audio timer clock
uint64_t cpu_counter = 0;          // Simulates the CPU clock
uint64_t sim_counter = 0;          // For keeping track of position in the wave array

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


void simulation(uint32_t len_wave_file, char* sim_mode, uint32_t fstart, uint32_t fend) {
    /** 
     * Function that simulates the execution on MCU 
     */
    if (!strcmp(sim_mode, "")) {
        sequencer_start(seq);
    } else {
        generate_sweep(SAW, len_wave_file, fstart, fend);
    }
    for (cpu_counter = 0; cpu_counter < MAXVAL32; cpu_counter++) {
        if (cpu_counter % TIMER_SEQ_TOP == 0) {
        /***** This simulates interrupts by our sequencer timer *****/
            sequencer_update();
        /************************************************************/
        }
        /***** This simulates interrupts by our audio timer *****/
        if (cpu_counter % TIMER_AUDIO_TOP == 0) {
            if (sim_counter >= len_wave_file)
                return; //stop simulation when we've reached the length of the wav
            wave_samples[sim_counter] = 16*audio_update(); // The '16' is just to scale from 12 to 16 bits for showcasing. 
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
    char* sim_mode;
    generator_setup();
	uint32_t *current_event = NO_EVENT;
  	//showcase_sequence(seq, current_event);
    
    uint32_t length_of_wav = AUDIO_HZ*30;
    uint32_t freq_s = 220;
    uint32_t freq_e = 440;
    if (argv == 1) {
        sim_mode = "\0";
    } else {
        length_of_wav = atoi(argc[1]);
        sim_mode = "sweep\0";
    }
    if (argv > 3) {
        freq_s = atoi(argc[2]);
        freq_e = atoi(argc[3]);
    }
    empty_wave(length_of_wav);
    simulation(length_of_wav, sim_mode, freq_s, freq_e);
    write_wav(wave_samples, length_of_wav);
    printf("Wrote data to 'simulation.wav'\n");
    free(wave_samples);

	return 0;
}
