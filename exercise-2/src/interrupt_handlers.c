#include <stdint.h>
#include <stdbool.h>
#include "math.h"

#include "sound.h"
#include "efm32gg.h"

void EnableSound();
void DisableSound();

void __attribute__ ((interrupt)) TIMER1_IRQHandler()
{
	int const sequencer_counter_max = AUDIO_HZ / SEQ_HZ;
	static int sequencer_counter = 0;
	// Get data from the sequencer
	uint16_t data = (audio_update() + 0xFFF) >> 4;
	*DAC0_CH1DATA = data & 0xFFF;
	*DAC0_CH0DATA = data & 0xFFF;

	if (sequencer_counter == 0) {
		sequencer_update();
	}
	sequencer_counter = (sequencer_counter + 1) % sequencer_counter_max;
	// clear pending interrupt
	*TIMER1_IFC |= 1;

}

void GPIO_HANDLER()
{
	// Read button state
	uint16_t button_state = *GPIO_PC_DIN;

	// If the first left button is pressed play a sound
	if (button_state == ((~(1 << 0)) & 0xFF)) {
		DisableSound();
		sequencer_start(seq);
	}
	// If the right button is pressed, stop the sound
	else if (button_state == ((~(1 << 1)) & 0xFF)) {
		DisableSound();
		sequencer_start(seq2);
	} else if (button_state == ((~(1 << 2)) & 0xFF)) {
		DisableSound();
		generate_sweep(WT, 4000, 0, 4000);
	} else if (button_state == ((~(1 << 3)) & 0xFF)) {
		DisableSound();
		generate_sweep(NOISE, 4000, 0, 4000);
	} else if (button_state == ((~(1 << 4)) & 0xFF)) {
		DisableSound();
	}
	// Clear the interrupt to avoid repeating interrupts
	*GPIO_IFC |= *GPIO_IF;
}

void __attribute__ ((interrupt)) GPIO_EVEN_IRQHandler()
{
	GPIO_HANDLER();
}

void __attribute__ ((interrupt)) GPIO_ODD_IRQHandler()
{
	GPIO_HANDLER();
}
