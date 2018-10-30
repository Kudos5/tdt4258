#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

void setupDAC()
{
	// Enable DAC clock
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_DAC0;

	// Make sure DAC clock is prescaled correctly
	*(DAC0_CTRL) = 0x50010;

	// Enable left and right channels
	*(DAC0_CH0CTRL) |= 1;
	*(DAC0_CH1CTRL) |= 1;
}
