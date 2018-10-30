#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

void setupTimer(uint16_t period)
{
    // Enable clock timer
    *CMU_HFPERCLKEN0 |= (1<<6);

    // Set period
    *TIMER1_TOP = period;

    // Enable timer interrupt generation
    *TIMER1_IEN |= 1;

    // Start timer
    *TIMER1_CMD |= 1;
}

