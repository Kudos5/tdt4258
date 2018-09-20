Exercise 2 - Notes
==================

Sequencer protocol
------------------
In order to play melodies, we will implement a *sequencer*. A *sequence* will 
consist of an array of *events* terminated by a 0000 event. Each event should 
contain the following fields:
- start time 
- duration (alternatively note on / off)
- frequency
- generator / instrument number

As we don't want to use more memory than necessary, we need to decide on a 
protocol that meets some constraints. First off, 32 bits should be enough to 
carry all the information above, so we'll use a single `uint32` for each event. 
*We should use the biggest available data size, because that means fewer load
operations, I think. So we use uint32 instead of 4 uint8 or whatever. This means
that we have to do some "unpacking" of the bytes in each event, though.*

### Time
The start time and duration fields depend on our sequencer *time resolution*,
which depends on how we set up the timers. The time resolution will be limited
by the size of events (32 bits) and the size of the counters (16 bits) (if we
don't handle wrapping). **TODO: Add the maths**

#### Start time
Like in the MIDI protocol, the timing of events are given in *offsets* from last
event. That way we save space compared to giving an *absolute* timestamp for
each event. We could also probably handle the counter wrapping to 0 if we want,
which will free us from the constraint of the counter size.

#### Duration
*Unlike* MIDI, each note is given a *duration*, as mentioned above, instead
of using note on / note off. *You should consider using note on / off, though,
as this can strip the event data by at least 7 bits. See what's easiest to
implement and best for performance.*

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
**TODO**
