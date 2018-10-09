#include <stdint.h>
#include <stdbool.h>

#include "sound.h"
#include "efm32gg.h"

void EnableSound();
void DisableSound();

uint16_t sawtooth_gen(uint32_t const f, uint32_t const F_s, uint32_t * const n_ptr, uint32_t const A) {
    // A is the amplite of the signal
    // n is the number of the current sample
    // Period in us
    uint32_t const T_us = (1000*1000)/f;
    // Time step in us
    uint32_t const dt_us = (1000*1000)/F_s;
    // Number of samples in a period
    uint32_t const N = T_us/dt_us;
    // Slope of curve
    uint32_t const alpha = A/N;
    
    // Output
    uint32_t const n = *n_ptr;
    uint16_t const y = alpha*n;
    // Increment n
    *n_ptr = (n+1) % N;

    return y;
}

/*
 * TIMER1 interrupt handler 
 */
void __attribute__ ((interrupt)) TIMER1_IRQHandler()
{
    // Here we start the generator
    
	/*
	 * TODO feed new samples to the DAC remember to clear the pending
	 * interrupt by writing 1 to TIMER1_IFC 
	 */

    // Write data to DAC
    static uint32_t n = 0;
    uint32_t const sawtooth_frequency = 1000;
    uint32_t const amplitude = 0xFFF >> 4;
    uint16_t sawtooth_data = sawtooth_gen(sawtooth_frequency, AUDIO_HZ, &n, amplitude);
    // *DAC0_CH1DATA = ~(*DAC0_CH1DATA) & 0x00F;
    // *DAC0_CH0DATA = ~(*DAC0_CH0DATA) & 0x00F;
    *DAC0_CH1DATA = sawtooth_data;
    *DAC0_CH0DATA = sawtooth_data;

    // clear pending interrupt
    *TIMER1_IFC |= 1;

}

void __attribute__ ((interrupt)) TIMER2_IRQHandler()
{
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
