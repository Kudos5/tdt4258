/**
 * Demonstrates the general sequencer idea and how parts of it could be implemented
 * Build by running    `gcc -o seq sequencer.c`
 */
#include <stdint.h>
#include <stdio.h>
#include "seqs.h"

#define NO_EVENT 0x00000000
#define SEQ_TERMINATOR 0x00000000
#define SEQ_CLOCK_HZ 100

/**
 * Function to decode the hex events to check that they work as intended.
 * This function also shows how information can be extracted from an event 
 * by the program. 
 */
void print_event(uint32_t event, float* total_time){
    /* Get time offset only (casting to uint8 will truncate MSBs) */
    uint8_t time_offset = event;
    float f_time_offset = time_offset / (float)SEQ_CLOCK_HZ;    
    *total_time += f_time_offset;

    /* Get duration only (right shift and truncate) */
    uint8_t duration = (event >> 8);
    float f_duration = duration / (float)SEQ_CLOCK_HZ;    

    /* Get the 10-bit frequency by clearing the 6 MSBs after shifting */
    uint16_t frequency = (event >> 16) & 0x03FF;

    /* Get generator / instrument number */
    uint8_t instrument = (event >> 26);
    instrument &=  0x0003;

    printf(" time=%f, dur=%f, freq=%d, inst=%d\n", 
            *total_time, f_duration, frequency, instrument);

    /* Alternative solution that doesn't truncate */
    /***********************************************
    printf(" time=%f, dur=%f, freq=%d, inst=%d\n", 
            (event & 0x000F)/(float)SEQ_CLOCK_HZ + *total_time,
            ((event >> 8) & 0x000F)/(float)SEQ_CLOCK_HZ, 
            ((event >> 16) & 0x03FF), 
            (event >> 26) & 0x0003
          );
     ***********************************************/

    /**
     * NOTE 
     * I have no idea if any of these methods are better than the other,
     * or if there is a completely different way of extracting individual bits 
     * that is superior to both. ...or if this is just premature optimization.
     * TODO: Find out.
     * */ 
} 



/**
 * Function that shows how sequences can be triggered and how events can be read from memory
 */
void showcase_sequence(const uint32_t* seq_start, uint32_t* current_event)
{
	/* Don't trigger a new sequence if one is already running */
	if (current_event != NO_EVENT)
		return;

    /* If no sequences running, start the given sequence */
	current_event = (uint32_t*)seq_start;	// 
    int ev_number = 0;
    float total_time = 0.0;

	/* Play all the events in the sequence */
	while (*current_event != SEQ_TERMINATOR) {
		printf("event #%d:  ", ev_number++);
        print_event(*current_event, &total_time);
		current_event += 1;	    // Set current_event to address of the next event
	}
	/* When done playing, set current_event to  */
	current_event = NO_EVENT;
}



int main(int argv, char **argc)
{

	uint32_t *current_event = NO_EVENT;
  	printf("Sequence 2\n");
  	showcase_sequence(seq2, current_event);

	return 0;
}
