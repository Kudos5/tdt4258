#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"
#include "sound.h"

// The sampling frequency
#define SAMPLING_FREQUENCY AUDIO_HZ
// The period between sound samples, in clock cycles 
#define SAMPLE_PERIOD_CYCLES (F_HFRCO/SAMPLING_FREQUENCY)

void setupGPIO();
void setupTimer(uint32_t period);
void setupDAC();
void PollTimer();
void PollButtons();
void DisableSound();

int main(void)
{
	setupGPIO();
	setupDAC();
	setupTimer(SAMPLE_PERIOD_CYCLES);
	generator_setup();

	while (1) {
		PollTimer();
		PollButtons();
	}

	return 0;
}

void PollTimer()
{
	// Keep track of the previous timer value
	static int timer_previous_value;
	int timer_value = *(TIMER1_CNT);
	// If timer value is less than previous there was an overflow, meaning that we have reached the period. If not we just update the previous value
	if (timer_value > timer_previous_value) {
		timer_previous_value = timer_value;
		return;
	}
	// Read data from the audio module and give it to the DAC
	// We must shift and scale the data to be within DACs accepted range of values
	uint16_t data = (audio_update() + 0xFFF) >> 4;
	*DAC0_CH1DATA = data & 0xFFF;
	*DAC0_CH0DATA = data & 0xFFF;

	// We use a clock divider to generate the sequencer clock
	int const sequencer_counter_max = AUDIO_HZ / SEQ_HZ;
	static int sequencer_counter = 0;
	if (sequencer_counter == 0) {
		sequencer_update();
	}
	sequencer_counter = (sequencer_counter + 1) % sequencer_counter_max;
	timer_previous_value = timer_value;
}

void PollButtons()
{
	// Read button state
	uint16_t button_state = (*GPIO_PC_DIN) & 0xFF;
	static uint16_t previous_button_state;
	// If there is no change in the button state, do nothing
	if ((button_state & 0xFF) == (previous_button_state & 0xFF)) {
		return;
	}
	// Store the previous state
	previous_button_state = button_state;

	// If the first left button is pressed play a sound
	if (button_state == ((~(1 << 0)) & 0xFF)) {
		DisableSound();
		sequencer_start(seq);
	}
	// Up button
	else if (button_state == ((~(1 << 1)) & 0xFF)) {
		DisableSound();
		sequencer_start(seq2);
	}
	// Right button
	else if (button_state == ((~(1 << 2)) & 0xFF)) {
		DisableSound();
		generate_sweep(WT, 4000, 0, 4000);
	}
	// Down button
	else if (button_state == ((~(1 << 3)) & 0xFF)) {
		DisableSound();
		generate_sweep(NOISE, 4000, 0, 4000);
	}
	// If the second left button is pressed, stop the sound
	else if (button_state == ((~(1 << 4)) & 0xFF)) {
		DisableSound();
	}
}
