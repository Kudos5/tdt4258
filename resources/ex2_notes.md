Exercise 2 - Notes
==================

Overview / Summary
------------------
- *Sequences* are composed on a computer in the form of Csound score files.
  A Python script is used to convert `.sco` files into hex formatted sequences.
  These sequences will be hard coded into the microcontroller.
- The *sequencer* will iterate through a sequence, i.e. a list of note events:
    + We use a dedicated sequencer clock to iterate through .......
- When a sequencer is started, it will usually trigger the first *event* in the
  sequence right away. The format of an *event* is determined by our protocol
  definition. Right now, it uses a uint32 that consists of the following:
    + bit      0: *type* of event- note *on* or *off*
    + bits  1-12: *time diff* from last event
    + bits 13-14: *instrument* number
    + bits 14-24: *frequency* ...
    + bits 25-31: unused... hmmmmmm
- When an event is triggered, the following happens:
    + The sequencer triggers a sound *generator*, i.e. either sets its `playing`
      flag to **on** or **off**.
    + A `next_event` pointer is incremented to point to the next event to be
      triggered. This event is then 
    + The next event is read into 
- Each event in the sequence triggers a sound *generator*, i.e. sets its
  `playing` flag to either *on* or *off*.
- 

Sequencer protocol
------------------
In order to play melodies, we will implement a *sequencer*. A *sequence* will 
consist of an array of *events* terminated by a 0000 event. Each event should 
contain the following fields:
- time diff from last event
- event type (note on / off)
- frequency
- generator / instrument number

As we don't want to use more memory than necessary, we need to decide on a 
protocol that meets some constraints. First off, 32 bits should be enough to 
carry all the information above, so we'll use a single `uint32` for each event. 
*We should use the biggest available data size, because that means fewer load
operations, I think. So we use uint32 instead of 4 uint8 or whatever. This means
that we have to do some "unpacking" of the bytes in each event, though.*

### Time 
Like in the MIDI protocol, the timing of events are given in *difference* from 
last event. Doing it in this differential way, we save space compared to giving 
an *absolute* timestamp for each event. 

The time diff field depends on our sequencer *time resolution*,
which depends on how we set up the timers. The time resolution will be limited
by the size of events (32 bits) and the size of the counters (16 bits) (if we
don't handle wrapping, **but we should do that**).
**TODO: Add the maths**

### Type
Also similar to MIDI, each event is either a note *on* or note *off* type event. 
That way we si

### Frequency
I think we will need about 10-12 bits for the frequency (1024-4096 Hz max freq),
as this covers the most "interesting" frequencies. If we limit our system to
these frequencies we can also limit the actual audio sample rate, which can save 
power, I think?

(*Note: We could make the size of events smaller by using note numbers (again
like MIDI) instead of frequency directly. Then we could for instance say that 6 
bits (64 notes) are enough for our use. 
This would, however, require conversion into frequency by the program, which in 
turn would impact our performance. We would also use more ROM in this case, so 
we choose to use frequencies directly.*)

### Generators
The number of bits used for the generator field is the probably the easiest 
choice we have to make. We just need to decide how many generators we want to
make, and use enough bits to cover them. Four or eight generators will probably 
be enough. See the Generators section for discussion about this.


Generators
----------
To make some interesting sounds, we should implement a couple of different
instruments / sound generators / oscillators, for instance with square, triangle 
and sawtooth waveforms. These must take the functionality of the DAC into
consideration. 

### Data format
From page 715 in the EFM32GG reference manual (13) it seems like the DAC can
operate in two modes, and that the mode determines the format of the data that 
should be supplied to it. The modes and the corresponding data formats:
- **Single Ended**: Takes unsigned 12 bit integers 
- **Differential**: Takes *signed* 12 bit integers (2's complement)

*We should decide which mode to use before implementing any generators.*


