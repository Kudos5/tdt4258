#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"
#include "sound.h"

// The sampling frequency in Hz
#define SAMPLING_FREQUENCY AUDIO_HZ
// The period between sound samples, in clock cycles 
#define SAMPLE_PERIOD_CYCLES (F_HFRCO/SAMPLING_FREQUENCY)

void setupGPIO();
void setupTimer(uint32_t period);
void setupDAC();
void setupNVIC();
void DisableSound();
void SetupGenerators();
void SetupSequencer();

int main(void)
{
	setupGPIO();
	setupDAC();
	setupTimer(SAMPLE_PERIOD_CYCLES);
	setupNVIC();
    generator_setup();

	while (1) {
        // Sleep until next interrupt
        __asm__ ("wfi;");
    }

	return 0;
}

void setupNVIC()
{
    // Enable Timer1 IRQ Handler
    *ISER0 |= (1 << 12);
    // Enable GPIO even handler
    *ISER0 |= (1 << 1);
    // Enable GPIO odd handler
    *ISER0 |= (1 << 11);

}

