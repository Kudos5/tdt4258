#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

/*
 * function to set up GPIO mode and interrupts
 */
void setupGPIO()
{
	/*
	 * TODO set input and output pins for the joystick 
	 */

	/*
	 * Example of HW access from C code: turn on joystick LEDs D4-D8 check 
	 * efm32gg.h for other useful register definitions 
	 */
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_GPIO;	/* enable GPIO clock */
    // Enable LEDs
	// *GPIO_PA_CTRL = 2;	/* set high drive strength */
	// *GPIO_PA_MODEH = 0x55555555;	/* set pins A8-15 as output */
	// *GPIO_PA_DOUT = 0xFF00;	/* turn off all leds */

    // Enable buttons
    // Set GPIO PC to input
    *GPIO_PC_MODEL = 0x33333333;
    // Enable internal pullup
    *GPIO_PC_DOUT = 0xFF;

    /* Enable interrupts for GPIO C when its state changes */
    *GPIO_EXTIPSELL = 0x22222222;
    // Set up the GPIO to interrupt when a bit changes from 1 to 0 (button pressed)
    *GPIO_EXTIFALL = 0xFF;
    /* Do _not* set up the GPIO to interrupt when a bit changes from 0 to 1 */
    // str r1, [r0, #GPIO_EXTIRISE]
    // Set up interrupt generation
    *GPIO_IEN |= 0xFF;
    // Clear interrupt flags to avoid interrupt on startup
    *GPIO_IFC = *GPIO_IF;
    // Enable CPU interrupts
    // *ISER0 = 0x0802;

}
