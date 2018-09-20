#define SEQ_TERMINATOR 0x00000000

/* Generate event sequences */
const uint32_t seq1[] = {
    0x034e0001,
    0xffff0002,
    0x00000003,
    0x0000ae14,
    0x0000ee18,
    0x0000801f,
    SEQ_TERMINATOR	// This must be included to terminate our sequence
};
// Not sure if this is equivalent to 
// const uint32_t * seq     or
// uint32_t * const seq    ..?
const uint32_t seq2[] = {
    0x046e2800,
    0x04dc0a0c,
    0x05b8190c,
    0x059f6419,
    0x055d3219,
    0x055d3219,
    SEQ_TERMINATOR
};

