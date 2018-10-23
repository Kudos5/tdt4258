#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"
#include "sound.h"

/*
 * TODO calculate the appropriate sample period for the sound wave(s) you 
 * want to generate. The core clock (which the timer clock is derived
 * from) runs at 14 MHz by default. Also remember that the timer counter
 * registers are 16 bits. 
 */

// The sampling frequency in Hz
// #define     SAMPLING_FREQUENCY 2000
#define     SAMPLING_FREQUENCY AUDIO_HZ
// The period between sound samples, in clock cycles 
#define   SAMPLE_PERIOD_CYCLES (F_HFRCO/SAMPLING_FREQUENCY)

/*
 * Declaration of peripheral setup functions 
 */
void setupGPIO();
void setupTimer(uint32_t period);
void setupDAC();
void enableDACSineGenerationMode();
void setupNVIC();
void PollButtons();
void DisableSound();
void SetupGenerators();
void SetupSequencer();

/*
 * Your code will start executing here 
 */
int main(void)
{
	/*
	 * Call the peripheral setup functions 
	 */
	setupGPIO();
	setupDAC();
	setupTimer(SAMPLE_PERIOD_CYCLES);
    generator_setup();

	/*
	 * TODO for higher energy efficiency, sleep while waiting for
	 * interrupts instead of infinite loop for busy-waiting 
	 */
    PollButtons();
	while (1) ;

	return 0;
}

void PollButtons() {
    while (1) {
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
    }
}

