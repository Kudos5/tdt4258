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
void PollTimer();
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
	while (1) {
        PollTimer();
        PollButtons();
    }

	return 0;
}

void PollTimer() {
    // Keep track of the previous timer value
    static int timer_previous_value;
    int timer_value = *(TIMER1_CNT);
    // If timer value is less than previous there was an overflow, meaning that we have reached the period. If not we just update the previous value
    if (timer_value > timer_previous_value) {
        timer_previous_value = timer_value;
        return;
    }

    // Read data from the audio module and give it to the DAC
    uint16_t data = (audio_update() + 0xFFF) >> 4; 
    *DAC0_CH1DATA = data & 0xFFF;
    *DAC0_CH0DATA = data & 0xFFF;

    // We use a clock divider to generate the sequencer clock
    int const sequencer_counter_max = AUDIO_HZ/SEQ_HZ;
    static int sequencer_counter = 0;
    if ( sequencer_counter == 0 ) {
        sequencer_update();
    }
    sequencer_counter = (sequencer_counter + 1) % sequencer_counter_max;
    timer_previous_value = timer_value;
}

void PollButtons() {
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

