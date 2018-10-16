#include "sound.h"

#ifdef SHOWCASE
#include "showcase.h"
#endif


///////////////////////////////////////////////////////////////////////////////
// 
// Generators
// 
///////////////////////////////////////////////////////////////////////////////
//
static uint32_t GENERATORS_ON = 0;                     // Flags for setting which generators are active

void modsquare();
void sawtooth();
void wavetable();
void noise();
typedef struct gen_struct {
    uint32_t position_in_cycles;    // Inspired by Audacity source code
    uint32_t frequency;             // Current frequency of the generator
    int16_t  current_value;         // This is where the output sample of each generator is set and read from
    void (*GENERATOR)();
} generator; 
static generator generators[NUM_GENERATORS];

/** Initialize the generator structures. 
 * Do *not* forget to call this on reset. 
 */
void generator_setup(){
    // Have to cast to 'generator' type, apparently (https://stackoverflow.com/a/27052438):
    generators[SAW]      = (generator){ 0, 0, 0, sawtooth };
    generators[SQUARE]   = (generator){ 0, 0, 0, modsquare };
    generators[NOISE]    = (generator){ 0, 0, 0, noise };
    generators[WT]       = (generator){ 0, 0, 0, wavetable };
}

volatile uint32_t TIMER_AUD_CNT;
volatile uint32_t TIMER_SEQ_CNT;

/** 
 * Update the square wave generator.
 * TODO : Find out whether 2's complement or not. All the generators depend on it.
 */

static uint32_t orgfreq = 0;
void modsquare() {
    static uint32_t modfreq = 18;
    static uint32_t modpic = 0;
    int32_t new_val = (generators[SQUARE].position_in_cycles % AUDIO_HZ < AUDIO_HZ/2) ? GEN_HIGH : GEN_LOW;
    generators[SQUARE].current_value = new_val;
    generators[SQUARE].position_in_cycles += generators[SQUARE].frequency;
    generators[SQUARE].frequency = (modpic % AUDIO_HZ < AUDIO_HZ/2) ? orgfreq : 2*orgfreq;
    modpic += modfreq;
}

void sawtooth() {
    int32_t new_val = (generators[SAW].position_in_cycles % AUDIO_HZ) * GEN_HIGH / AUDIO_HZ;
    generators[SAW].current_value = new_val;
    generators[SAW].position_in_cycles += generators[SAW].frequency;
}

void wavetable() {
    uint32_t phasor = (generators[WT].position_in_cycles % AUDIO_HZ) * (WT_SIZE-1) / AUDIO_HZ;
    int16_t new_val = WAVETABLE[phasor];
    generators[WT].current_value = new_val;
    generators[WT].position_in_cycles += generators[WT].frequency;
}

void noise() {
    uint32_t phasor = (generators[NOISE].position_in_cycles % AUDIO_HZ) * (WT_NOISE_SIZE-1) / AUDIO_HZ;
    int16_t new_val = WAVETABLE_NOISE[phasor];
    generators[NOISE].current_value = new_val;
    generators[NOISE].position_in_cycles += generators[NOISE].frequency;
}

void generator_set_frequency(uint32_t gen, uint32_t current_freq_scaled, uint32_t scaling){
    generators[gen].frequency = current_freq_scaled / scaling; 
}

// To run on note ons
void generator_start(uint32_t gen, uint32_t freq) {
    /* NOTE: Only one 'on' flag per generator => only monophonic instruments supported 
     * If notes overlap in this protocol, the first note is overwritten! */
    GENERATORS_ON |= (1 << gen);   // Set the 'on' flag for the given generator
    // TODO : Start the generator 
    generators[gen].current_value = 0;
    generators[gen].frequency = freq;
    orgfreq = freq;
    generators[gen].GENERATOR();                        // Run the updater function immediately
}

// To run on note offs
void generator_stop(uint32_t gen) {
    GENERATORS_ON &= ~(1 << gen);   // Clear the 'on' flag for the given generator
    generators[gen].current_value = 0;
}


/** 
 * Function that should be called on AUDIO_TIMER interrupts
 */
int16_t audio_update() {
    // Increment counter
    TIMER_AUD_CNT = (TIMER_AUD_CNT+1) % UINT16_MAX;     // TODO: We can use 32 bits eventually. Not sure if needed
    /** Tasks of this section:
     * 1. Read from gen_current_value and mix them
     * 2. Put the result of the mix into the DAC register (or the wave_samples, in the case of simulation)
     * 3. Update the gen_current_value for each generator
     */
    /* update the gen_current_value */
    int16_t new_sample = 0;
    for (int gen = 0; gen < NUM_GENERATORS; gen++) {
        if (GENERATORS_ON & (1 << gen)) {
            sweep();
            generators[gen].GENERATOR(); 
            new_sample += generators[gen].current_value;
        }
    }
    return new_sample >> 2;
}


///////////////////////////////////////////////////////////////////////////////
// 
// Sequencer
// 
///////////////////////////////////////////////////////////////////////////////

// Define static global variables for keeping track 
static uint32_t* next_event = NO_EVENT;   // NO_EVENT is just 0.. 
static uint32_t next_event_time = 0;      // Used for telling sequencer_update when next event should be triggered 


/** 
 * This is the function we should call to start playing a sequence.
 */
void sequencer_start(const uint32_t* seq_to_play) {
    // Don't trigger a new sequence if one already exists
    if (next_event)
        return;
    next_event = (uint32_t*) seq_to_play;
}

/* This */
void sequencer_stop(){
    // TODO here:
    //  - Set 'next_event = NO_EVENT'. That way the sequencer won't play even if the clock is active
    //  - Disable the sequencer time interrupts. Guess we save some energy on that?
    next_event = NO_EVENT;
}

/** 
 * Call this function from the sequence timer interrupt handler
 */
void sequencer_update(){
    TIMER_SEQ_CNT = (TIMER_SEQ_CNT + 1) % UINT16_MAX;
    // TODO: Is all this dereferencing better than just storing the next event in a variable? Does it matter?
    // SEQ_TERMINATOR is 0, so we don't have to write  *next_event != SEQ_TERMINATOR, but you can think about it that way
    if (next_event && TIMER_SEQ_CNT >= next_event_time) {
            // ^ TODO: if possible, we should set up an interrupt at specific timer counter value, 
            // so that we don't have to check each sequencer interrupt? Not sure if possible. */
        /* New event, trigger a generator */
        uint32_t inst = (*next_event >> INST_POS) & INST_MASK;
        uint32_t freq = (*next_event >> FREQ_POS) & FREQ_MASK;
        // If the event is a 'note on' event:
        if (*next_event & TYPE_MASK) {
            generator_start(inst, freq);
        } else {
            generator_stop(inst);
        }
        // If next event is not a 0 terminator:
        if (*(++next_event)) {
            uint32_t time = (*next_event) >> TIME_POS & TIME_MASK; // (float)SEQ_CLOCK_HZ;
            next_event_time = (TIMER_SEQ_CNT + time) % MAXVAL16;
            #ifdef SHOWCASE
            char* type    = (*next_event & TYPE_MASK) ? "On\0" : "Off\0";
            print_event(sim_counter, TIMER_SEQ_CNT, inst, type, time, freq);
            #endif
        } else {
            // If next event is a 0 terminator, we've reached the end of the sequence 
            sequencer_stop();
        }
    } 
    // Also, we might just turn the entire clock off when not in use (sequencer not playing), 
    // so that we don't have to check if an event is playing at all.
}


// General TODO : Add the following for easily setting and clearing single bits:
//      #define SET(x,y) (x|=(1<<y))
//      #define CLR(x,y) (x&=(~(1<<y)))
