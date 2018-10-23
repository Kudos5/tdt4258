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
    /* Have to cast to 'generator' type, apparently (https://stackoverflow.com/a/27052438) */
    generators[SAW]      = (generator){ 0, 0, 0, sawtooth };
    generators[SQUARE]   = (generator){ 0, 0, 0, modsquare };
    generators[NOISE]    = (generator){ 0, 0, 0, noise };
    generators[WT]       = (generator){ 0, 0, 0, wavetable };
}

volatile uint32_t TIMER_AUD_CNT;
volatile uint32_t TIMER_SEQ_CNT;

/********** Generator update functions **********/
static uint32_t orgfreq = 0;
void modsquare() {
    static uint32_t modfreq = 18;   /* Modulate frequency for a more interesting sound */
    static uint32_t modpic = 0;
    int32_t new_val = (generators[SQUARE].position_in_cycles % AUDIO_HZ < AUDIO_HZ/2) ? GEN_HIGH : GEN_LOW;
    generators[SQUARE].current_value = new_val;
    generators[SQUARE].position_in_cycles += generators[SQUARE].frequency;
    generators[SQUARE].frequency = (modpic % AUDIO_HZ < AUDIO_HZ/2) ? orgfreq : 2*orgfreq;
    modpic += modfreq;
}

void sawtooth() {
    int32_t new_val = (generators[SAW].position_in_cycles % AUDIO_HZ) * GEN_HIGH / AUDIO_HZ;
    /* Scale and move value so that it is between -2048,2047 */
    new_val = 2*(new_val - GEN_HIGH/2);
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
/************************************************/

void generator_set_frequency(uint32_t gen, uint32_t current_freq_scaled, uint32_t scaling){
    generators[gen].frequency = current_freq_scaled / scaling; 
}

/** Function to run on "note on"s or to trigger one-shot sounds
 * NOTE: Only one 'on' flag per generator => only monophonic instruments supported 
 * If notes overlap in this protocol, the first note is overwritten! */
void generator_start(uint32_t gen, uint32_t freq) {
    GENERATORS_ON |= (1 << gen);   // Set the 'on' flag for the given generator
    generators[gen].current_value = 0;
    generators[gen].frequency = freq;
    orgfreq = freq;
    generators[gen].GENERATOR();                        // Run the updater function immediately
}


/** To run on note offs */
void generator_stop(uint32_t gen) {
    GENERATORS_ON &= ~(1 << gen);   // Clear the 'on' flag for the given generator
    generators[gen].current_value = 0;
}


/** 
 * Function that should be called on each timer interrupt.
 * Updates the value of each active generator by calling their updater function,
 * then adds (mixes) the new generator values together and returns the sum.
 */
int16_t audio_update() {
    // Increment counter
    TIMER_AUD_CNT = (TIMER_AUD_CNT+1) % UINT16_MAX;     // TODO: We could use 32 bits here. 
    int16_t new_sample = 0;
    for (int gen = 0; gen < NUM_GENERATORS; gen++) {
        if (GENERATORS_ON & (1 << gen)) {
            sweep();
            generators[gen].GENERATOR(); 
            new_sample += generators[gen].current_value;
        }
    }
    return new_sample >> 2;     /* Scaling to avoid distortion */
}


///////////////////////////////////////////////////////////////////////////////
// 
// Sequencer
// 
///////////////////////////////////////////////////////////////////////////////

/* next_event is a pointer to the next event that should be read from a sequence */
static uint32_t* next_event = NO_EVENT;
/* next_event_time is Used for telling sequencer_update when next event should be triggered */
static uint32_t next_event_time = 0;  


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
    /* If it is time for the next event, trigger (on / off) a generator. 
     * NO_EVENT is defined as 0, so we don't have to write 
     * `next_event != NO_EVENT && ...`, but you can think about it that way */
    if (next_event && TIMER_SEQ_CNT >= next_event_time) {
        uint32_t inst = (*next_event >> INST_POS) & INST_MASK;
        uint32_t freq = (*next_event >> FREQ_POS) & FREQ_MASK;
        /* If the event is 'note on' type */
        if (*next_event & TYPE_MASK) {
            generator_start(inst, freq);
        } /* If event is a 'note off' */
        else {
            generator_stop(inst);
        }
        /* SEQ_TERMINATOR is also defined as 0, so the following means 
         * "If *next_event (after ptr incrementation) is not a SEQ_TERMINATOR..." */
        if (*(++next_event)) {
            uint32_t time = (*next_event) >> TIME_POS & TIME_MASK;
            next_event_time = (TIMER_SEQ_CNT + time) % MAXVAL16;
            #ifdef SHOWCASE
            char* type    = (*next_event & TYPE_MASK) ? "On\0" : "Off\0";
            print_event(sim_counter, TIMER_SEQ_CNT, inst, type, time, freq);
            #endif
        } else {
            /* If next event is a SEQ_TERMINATOR, we've reached the end of the sequence */
            sequencer_stop();
        }
    } 
}

// General TODO : Add the following for easily setting and clearing single bits:
//      #define SET(x,y) (x|=(1<<y))
//      #define CLR(x,y) (x&=(~(1<<y)))
