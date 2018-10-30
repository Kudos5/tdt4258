#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

void setupGPIO()
{
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_GPIO;	/* enable GPIO clock */

	// Enable buttons
	// Set GPIO PC to input
	*GPIO_PC_MODEL = 0x33333333;
	// Enable internal pullup
	*GPIO_PC_DOUT = 0xFF;

	/* Enable interrupts for GPIO C when its state changes */
	*GPIO_EXTIPSELL = 0x22222222;
	// Set up the GPIO to interrupt when a bit changes from 1 to 0 (button pressed)
	*GPIO_EXTIFALL = 0xFF;
	// Set up interrupt generation
	*GPIO_IEN |= 0xFF;
	// Clear interrupt flags to avoid interrupt on startup
	*GPIO_IFC = *GPIO_IF;
}
