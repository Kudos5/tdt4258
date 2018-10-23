#include <stdint.h>
#include <stdbool.h>
#include "math.h"

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
void __attribute__ ((interrupt)) TIMER1_IRQHandler() {
    int const sequencer_counter_max = AUDIO_HZ/SEQ_HZ;
    static int sequencer_counter = 0;
    uint16_t data = (audio_update() + 0xFFF) >> 4; 
    // uint16_t data = audio_update();
    // static uint16_t data = 0xFFF >> 4;
    // data = (~data) & 0xFFF;
    *DAC0_CH1DATA = data & 0xFFF;
    *DAC0_CH0DATA = data & 0xFFF;

    if ( sequencer_counter == 0 ) {
        sequencer_update();
    }
    sequencer_counter = (sequencer_counter + 1) % sequencer_counter_max;
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
        DisableSound();
        sequencer_start(seq);
    }
    // If the right button is pressed, stop the sound
    else if ( button_state == ((~(1 << 1)) & 0xFF) ) {
        DisableSound();
        sequencer_start(seq2);
    }
    else if ( button_state == ((~(1 << 2)) & 0xFF) ) {
        DisableSound();
        generate_sweep(WT, 4000, 0, 4000);
    }
    else if ( button_state == ((~(1 << 3)) & 0xFF) ) {
        // sequencer_start(seq4);
        DisableSound();
        generate_sweep(NOISE, 4000, 0, 4000);
    }
    else if ( button_state == ((~(1 << 4)) & 0xFF) ) {
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
