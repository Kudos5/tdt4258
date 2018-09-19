#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

void setupDAC()
{
	/*
	 * TODO enable and set up the Digital-Analog Converter
	 * 
	 * 1. Enable the DAC clock by setting bit 17 in CMU_HFPERCLKEN0 2.
	 * Prescale DAC clock by writing 0x50010 to DAC0_CTRL 3. Enable left
	 * and right audio channels by writing 1 to DAC0_CH0CTRL and
	 * DAC0_CH1CTRL 4. Write a continuous stream of samples to the DAC
	 * data registers, DAC0_CH0DATA and DAC0_CH1DATA, for example from a
	 * timer interrupt 
	 */
    // Enable DAC clock
    *CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_DAC0;

    // Make sure DAC clock is prescaled correctly
    *(DAC0_CTRL) = 0x50010;

    // Enable left and right channels
    *(DAC0_CH0CTRL) = 1;
    *(DAC0_CH1CTRL) = 1;
}

// NOTE: Sine generation does not work yet
void enableDACSineGenerationMode() {
    // Set prescale and enable sine mode
    *(DAC0_CTRL) = (0x7 << 16) | (0x1 << 4) | (0x1 << 1);

}

