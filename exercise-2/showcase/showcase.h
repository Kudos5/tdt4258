#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#ifndef _SHOWCASE_H_
#define _SHOWCASE_H_

/* Simulation clocks */
// Have to use 'extern' and declare the variables in showcase.c, or else the linker will complain.
extern uint32_t seq_counter;          // Simulates sequencer timer clock
extern uint32_t aud_counter;          // Simulates audio timer clock
extern uint64_t cpu_counter;          // Simulates the CPU clock
extern uint64_t sim_counter;          // For keeping track of position in the wave array

int16_t* wave_samples;             // Used for writing to file 

/* Printing and writing to file */
void write_wav(int16_t* wave_array, uint32_t wave_len);
void print_event(uint64_t simc, uint16_t seqc, uint32_t in, char* ty, uint32_t ti, uint32_t fre);

#endif
