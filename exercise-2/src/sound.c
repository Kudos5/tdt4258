#include "sound.h"

#ifdef SHOWCASE
#include "showcase.h"
#endif

#define NO_EVENT 0x00000000
/* Bit masks used by the sequencer*/
#define TYPE_MASK 0x00000001
#define TIME_MASK 0x000007FF
#define INST_MASK 0x00000003
#define FREQ_MASK 0x000003FF
/* Position of first bit for each info field in sequencer events */
#define TIME_POS 1
#define INST_POS 12
#define FREQ_POS 14

/* Sequencer and generator information */
#define SEQ_CLOCK_HZ 1000
#define NUM_GENERATORS 4
#define GEN_HIGH 2047
#define GEN_LOW -2048
#define WT_SIZE 32
#define WT_NOISE_SIZE 128

/* Panic function */
void DisableSound()
{
	generator_stop(SQUARE);
	generator_stop(SAW);
	generator_stop(WT);
	generator_stop(NOISE);
	// generator_stop_all();
	sequencer_stop();
}

///////////////////////////////////////////////////////////////////////////////
// 
// Generators
// 
///////////////////////////////////////////////////////////////////////////////

static uint32_t GENERATORS_ON = 0;	/* Flags for setting which generators are active */

void sweep();			/* Sweep functionality is implemented below */
void modsquare();
void sawtooth();
void wavetable();
void noise();
typedef struct gen_struct {
	uint32_t position_in_cycles;	/* Temporal position of the generator */
	uint32_t frequency;	/* Current frequency of the generator */
	int16_t current_value;	/* This is where the output sample of each generator is set and read from */
	void (*GENERATOR) ();	/* Pointer to the generator update function */
} generator;
static generator generators[NUM_GENERATORS];

/** Initialize the generator structures. 
 * Do *not* forget to call this on reset. 
 */
void generator_setup()
{
	/* Have to cast to 'generator' type, apparently (https://stackoverflow.com/a/27052438) */
	generators[SAW] = (generator) {
	0, 0, 0, sawtooth};
	generators[SQUARE] = (generator) {
	0, 0, 0, modsquare};
	generators[NOISE] = (generator) {
	0, 0, 0, noise};
	generators[WT] = (generator) {
	0, 0, 0, wavetable};
}

volatile uint32_t TIMER_AUD_CNT;
volatile uint32_t TIMER_SEQ_CNT;

/////////// Generator update functions ///////////
static uint32_t orgfreq = 0;
void modsquare()
{
	static uint32_t modfreq = 18;	/* Modulate frequency for a more interesting sound */
	static uint32_t modpic = 0;
	int32_t new_val =
	    (generators[SQUARE].position_in_cycles % AUDIO_HZ <
	     AUDIO_HZ / 2) ? GEN_HIGH : GEN_LOW;
	generators[SQUARE].current_value = new_val;
	generators[SQUARE].position_in_cycles += generators[SQUARE].frequency;
	generators[SQUARE].frequency =
	    (modpic % AUDIO_HZ < AUDIO_HZ / 2) ? orgfreq : 2 * orgfreq;
	modpic += modfreq;
}

void sawtooth()
{
	int32_t new_val =
	    (generators[SAW].position_in_cycles % AUDIO_HZ) * GEN_HIGH /
	    AUDIO_HZ;
	/* Scale and move value so that it is between -2048,2047 */
	new_val = 2 * (new_val - GEN_HIGH / 2);
	generators[SAW].current_value = new_val;
	generators[SAW].position_in_cycles += generators[SAW].frequency;
}

void wavetable()
{
	uint32_t phasor =
	    (generators[WT].position_in_cycles % AUDIO_HZ) * (WT_SIZE -
							      1) / AUDIO_HZ;
	int16_t new_val = WAVETABLE[phasor];
	generators[WT].current_value = new_val;
	generators[WT].position_in_cycles += generators[WT].frequency;
}

void noise()
{
	uint32_t phasor =
	    (generators[NOISE].position_in_cycles % AUDIO_HZ) * (WT_NOISE_SIZE -
								 1) / AUDIO_HZ;
	int16_t new_val = WAVETABLE_NOISE[phasor];
	generators[NOISE].current_value = new_val;
	generators[NOISE].position_in_cycles += generators[NOISE].frequency;
}

//////////////////////////////////////////////////

/** 
 * Function to activate a generator.
 * To be run on "note on" type events or to trigger one-shot sounds.
 * NOTE: Only one 'on' flag per generator => only monophonic instruments supported 
 * If notes overlap in this protocol, the first note is overwritten! 
 * */
void generator_start(uint32_t gen, uint32_t freq)
{
	GENERATORS_ON |= (1 << gen);	/* Set the 'on' flag for the given generator */
	generators[gen].current_value = 0;	/* (Re)set the value of of the generator */
	generators[gen].frequency = freq;	/* Set current frequency of the generator */
	orgfreq = freq;		/* Only used by the modulated square generator */
	generators[gen].GENERATOR();	/* Run the updater function immediately */
	generators[gen].position_in_cycles = 0;
}

/** 
 * Function to run on "note off" events.
 * Clears the 'on' flag for the given generator and sets its value to 0 
 * */
void generator_stop(uint32_t gen)
{
	GENERATORS_ON &= ~(1 << gen);
	generators[gen].current_value = 0;
	generators[gen].position_in_cycles = 0;
}

void generator_stop_all()
{
	for (int i = 0; i < NUM_GENERATORS; ++i) {
		GENERATORS_ON = 0;
		generators[i].position_in_cycles = 0;
		generators[i].frequency = 0;
		generators[i].current_value = 0;
	}
}

/** 
 * Function that should be called on each timer interrupt.
 * Updates the value of each active generator by calling their updater function,
 * then adds (mixes) the new generator values together and returns the sum.
 */
int16_t audio_update()
{
	// Increment counter
	TIMER_AUD_CNT = (TIMER_AUD_CNT + 1) % UINT16_MAX;
	int16_t new_sample = 0;
	for (int gen = 0; gen < NUM_GENERATORS; gen++) {
		if (GENERATORS_ON & (1 << gen)) {
			sweep();
			generators[gen].GENERATOR();
			new_sample += generators[gen].current_value;
		}
	}
	return new_sample >> 2;	/* Scaling to avoid distortion */
}

// /Generators
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// 
// Sequencer
// 
///////////////////////////////////////////////////////////////////////////////

/* next_event is a pointer to the next event that should be read from a sequence */
static uint32_t *next_event = NO_EVENT;
/* next_event_time is Used for telling sequencer_update when next event should be triggered */
static uint32_t next_event_time = 0;

/** 
 * Function to start playing a sequence.
 * Simply sets next_event to the address of the first event in a null terminated 
 * sequence, unless another sequence is already playing (next_event != 0). 
 */
void sequencer_start(const uint32_t * seq_to_play)
{
	if (next_event)
		return;
	next_event = (uint32_t *) seq_to_play;
	next_event_time = TIMER_SEQ_CNT + 1;
}

/** 
 * Function to stop the active sequence.
 * Simply makes next_event point to 0 (NO_EVENT) 
 * */
void sequencer_stop()
{
	next_event = NO_EVENT;
}

/** 
 * Main sequencer function
 *  - Updates the sequencer timer. 
 *    If next_event != NO_EVENT (0), _and_ we've reached next_event_time:
 *      + Read the event that next_event points to
 *      + Start / stop generator based on the event
 *      + Increment next_event pointer. If next event is _not_ a SEQ_TERMINATOR (0), 
 *        update next_event_time by reading the time bits of the next event. 
 *        If it _is_ a SEQ_TERMINATOR, stop the sequence immediately.
 */
void sequencer_update()
{
	++TIMER_SEQ_CNT;
	if (next_event && (TIMER_SEQ_CNT >= next_event_time)) {
		uint32_t inst = (*next_event >> INST_POS) & INST_MASK;
		uint32_t freq = (*next_event >> FREQ_POS) & FREQ_MASK;
		/* If the event is 'note on' type */
		if (*next_event & TYPE_MASK) {
			generator_start(inst, freq);
		} /* If event is a 'note off' */
		else {
			generator_stop(inst);
		}
		if (*(++next_event)) {
			uint32_t time = (*next_event) >> TIME_POS & TIME_MASK;
			next_event_time = TIMER_SEQ_CNT + time;
#ifdef SHOWCASE
			char *type =
			    (*next_event & TYPE_MASK) ? "On\0" : "Off\0";
			print_event(sim_counter, TIMER_SEQ_CNT, inst, type,
				    time, freq);
#endif
		} else {
			sequencer_stop();
		}
	}
}

// /Sequencer
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// Sweep
//
///////////////////////////////////////////////////////////////////////////////
#define SCALING 100

static uint32_t current_freq_scaled;
static uint32_t freq_start_scaled;
static uint32_t freq_end_scaled;
static uint32_t freq_delta_scaled;
static int32_t sweeped_gen = -1;

/**
 * Update the frequency of sweeping generators
 */
void sweep()
{
	if (sweeped_gen < 0)
		return;
	if (current_freq_scaled >= freq_end_scaled) {
		current_freq_scaled = SCALING * freq_start_scaled;
		generator_stop(sweeped_gen);
		sweeped_gen = -1;
	} else {
		current_freq_scaled += freq_delta_scaled;
	}
	generators[sweeped_gen].frequency = current_freq_scaled / SCALING;
}

void generate_sweep(uint32_t gen, uint32_t num_samples_in_sweep,
		    uint32_t fstart, uint32_t fend)
{
	sequencer_stop();
	sweeped_gen = gen;
	freq_start_scaled = SCALING * fstart;
	freq_end_scaled = SCALING * fend;
	current_freq_scaled = freq_start_scaled;
	/* \/ Floating point exception here \/ */
	freq_delta_scaled =
	    (uint32_t) (freq_end_scaled -
			freq_start_scaled) / num_samples_in_sweep;
	generator_start(gen, fstart);
}

// /Sweep
///////////////////////////////////////////////////////////////////////////////
