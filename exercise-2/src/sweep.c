#include "sound.h"
#define SCALING 100

static uint32_t current_freq_scaled;
static uint32_t freq_start_scaled;
static uint32_t freq_end_scaled;
static uint32_t freq_delta_scaled;
static int32_t sweeped_gen = -1;
/**
 * Update the frequency of sweeping generators
 */
void sweep() {
    if (sweeped_gen < 0)
        return;
    if (current_freq_scaled >= freq_end_scaled){
        // We should stop before this point... 
        current_freq_scaled = SCALING*freq_start_scaled;
        generator_stop(sweeped_gen);
        sweeped_gen = -1;
    } else {
        current_freq_scaled += freq_delta_scaled;
    }
    generator_set_frequency(sweeped_gen, current_freq_scaled, SCALING);
}

void generate_sweep(uint32_t gen, uint32_t num_samples_in_sweep, 
        uint32_t fstart, uint32_t fend) {
    sequencer_stop();
    // TODO : Stop all generators.
    sweeped_gen = gen;
    freq_start_scaled = SCALING*fstart;
    freq_end_scaled = SCALING*fend;
    current_freq_scaled = freq_start_scaled;
    /* \/ Floating point exception here \/ */
    freq_delta_scaled = (uint32_t) (freq_end_scaled - freq_start_scaled) / num_samples_in_sweep;
    generator_start(gen, fstart);
}
