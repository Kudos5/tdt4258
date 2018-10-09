#include "sound.h"
// #include "showcase.h"   // TODO : Remove after testing


///////////////////////////////////////////////////////////////////////////////
// 
// Generators
// 
///////////////////////////////////////////////////////////////////////////////
//
static uint32_t GENERATORS_ON = 0;                     // Bitmask for setting which generators are active                                           

void square();
void triangle();
void sawtooth();
typedef struct gen_struct {
    uint32_t frequency;         // Current frequency of the generator
    uint32_t time_for_change;   // Used in audio interrupt to see if the waveform should change direction or something
    int16_t  current_value;     // This is where the output sample of each generator is set and read from
    uint16_t playing;           // Currently not used, as we use GENERATORS_ON instead. Not sure what's best for maintaining playing generators
    void (*GENERATOR)();
} generator; 
static generator generators[NUM_GENERATORS];

/** Initialize the generator structures. 
 * Do *not* forget to call this on reset. 
 */
void generator_setup(){
    // Have to cast to 'generator' type, apparently (https://stackoverflow.com/a/27052438):
    generators[TRIANGLE] = (generator){ 0, 0, 0, 0, triangle };
    generators[SQUARE]   = (generator){ 0, 0, 0, 0, square };
    generators[SAW]      = (generator){ 0, 0, 0, 0, sawtooth };
}

volatile uint32_t TIMER_AUD_CNT;
volatile uint32_t TIMER_SEQ_CNT;

/** 
 * Update the square wave generator.
 * TODO : Find out whether 2's complement or not. All the generators depend on it.
 */
void square() {
    static int32_t halfway = 0;
    /* If it is time to change the waveform direction */
    if (TIMER_AUD_CNT == generators[SQUARE].time_for_change){     // TODO : This 'if' is common for all generators, so maybe extract it?
        generators[SQUARE].time_for_change = (TIMER_AUD_CNT + AUDIO_HZ /(generators[SQUARE].frequency)) % MAXVAL16; // time_for_change is one period from now
        halfway = (TIMER_AUD_CNT + AUDIO_HZ /(2*generators[SQUARE].frequency)) % MAXVAL16; // used to flip the wave on half periods 
        if (generators[SQUARE].current_value <= 0) {        // Use toggling instead (except for the initialization)
            generators[SQUARE].current_value = GEN_HIGH;      // replace with the correct velocity later. 
        } else {
            generators[SQUARE].current_value = GEN_LOW;
        }
    } else if (TIMER_AUD_CNT == halfway){
        /* This halfway thing is here to avoid 'off-by-1-sample' errors*/
        // In case of halfway, just flip the bit, and set halfway equal to 'whole way'
        if (generators[SQUARE].current_value <= 0) {        // Use toggling instead (except for the initialization)
            generators[SQUARE].current_value = GEN_HIGH;      // replace with the correct velocity later. 
        } else {
            generators[SQUARE].current_value = GEN_LOW;
        }
        halfway = generators[SQUARE].time_for_change;
    }
}

void sawtooth() {
    static int16_t increment = 0;
    if (TIMER_AUD_CNT == generators[SAW].time_for_change){
        /** Whenever we reach a time to change, we do the following:
         *  - Wrap around (or initialize) to the lowest possible value 
         *  - Update the 'time_for_change'
         *  - Update the increment in case this is an initialization. 
         *    (I guess the alternative is to check for initialization each time,
         *    or make some other functionality for *initializing* generators?)
         */
        int16_t period = AUDIO_HZ / generators[SAW].frequency;
        increment = GEN_RANGE / period;
        generators[SAW].time_for_change = (TIMER_AUD_CNT + period) % MAXVAL16; 
        generators[SAW].current_value = GEN_LOW;
    } else {
        /* If we haven't reached the time_to_change just keep incrementing */
        generators[SAW].current_value += increment;
    }
}

void triangle() {
    return;
}

// To run on note ons
void generator_start(uint32_t gen, uint32_t freq) {
    /* NOTE: Only one 'on' flag per generator => only monophonic instruments supported 
     * If notes overlap in this protocol, the first note is overwritten! */
    GENERATORS_ON |= (1 << gen);   // Set the 'on' flag for the given generator
    // TODO : Start the generator 
    generators[gen].time_for_change = TIMER_AUD_CNT;   // Make the waveform change instantly
    generators[gen].current_value = 0;
    generators[gen].frequency = freq;
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
    TIMER_AUD_CNT = (TIMER_AUD_CNT+1) % UINT16_MAX;
    /** Tasks of this section:
     * 1. Read from gen_current_value and mix them
     * 2. Put the result of the mix into the DAC register (or the wave_samples, in the case of simulation)
     * 3. Update the gen_current_value for each generator
     */
    //if (GENERATORS_ON )
    /* update the gen_current_value */
    int16_t new_sample = 0;
    for (int gen = 0; gen < NUM_GENERATORS; gen++) {
    // int gen = SQUARE;
        if (GENERATORS_ON & (1 << gen)) {
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
        char* type    = (*next_event & TYPE_MASK) ? "On\0" : "Off\0";
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
            #ifdef _SHOWCASE_H_
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
