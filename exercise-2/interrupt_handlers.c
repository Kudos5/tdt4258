#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

void EnableSound();
void DisableSound();

/*
 * TIMER1 interrupt handler 
 */
void __attribute__ ((interrupt)) TIMER1_IRQHandler()
{
	/*
	 * TODO feed new samples to the DAC remember to clear the pending
	 * interrupt by writing 1 to TIMER1_IFC 
	 */

    // Write data to DAC
    *DAC0_CH1DATA = ~(*DAC0_CH1DATA) & 0x00F;
    *DAC0_CH0DATA = ~(*DAC0_CH0DATA) & 0x00F;

    // clear pending interrupt
    *TIMER1_IFC |= 1;

}

void GPIO_HANDLER() {
    // Read button state
    uint16_t button_state = *GPIO_PC_DIN;
    // Activate LEDs corresponding to the buttons pressed
    *GPIO_PA_DOUT = button_state << 8;
    // If the first left button is pressed play a sound
    if ( button_state == ((~(1 << 0)) & 0xFF) ) {
        EnableSound();
    }
    // If the right button is pressed, stop the sound
    else if ( button_state == ((~(1 << 1)) & 0xFF) ) {
        DisableSound();
    }
    // Clear the interrupt to avoid repeating interrupts
    *GPIO_IFC |= *GPIO_IF;
}

/*
 * GPIO even pin interrupt handler 
 */
void __attribute__ ((interrupt)) GPIO_EVEN_IRQHandler()
{
    GPIO_HANDLER();
}

/*
 * GPIO odd pin interrupt handler 
 */
void __attribute__ ((interrupt)) GPIO_ODD_IRQHandler()
{
    GPIO_HANDLER();
}
