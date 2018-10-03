How to use the sound functionality
==================================
The sound code contains the following functions:
- `void generator_start(uint32_t generator, uint32_t freq)`
- `void generator_stop(uint32_t generator)`
- `int16_t audio_update()`
- `void sequencer_start(const uint32_t* seq_to_play);`
- `void sequencer_stop();`
- `void sequencer_update();`

Experimenting with the generators
---------------------------------
In order to experiment with the generators we can try the following:
1. First, set the right register values so that we can use both an audio timer and
   a sequence timer. Enable the clocks and interrupts etc...
    + Make the audio interrupt handler and enable interrupt
    + Define the audio and sequence counter addresses:
      In the `sound` code, I have named the counter addresses `TIMER_AUD_CNT` and 
      `TIMER_SEQ_CNT`, so if we just `#define` these as the addresses of the MCU timers 
      it should be plug-and-play, ish.

2. On press of a specific button, start a generator with a specific frequency, e.g. 
    generator_start(SAW, 300);
3. On press of another button, stop the generator:
    generator_stop(SAW);
