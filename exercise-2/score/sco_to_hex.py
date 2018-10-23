#!/bin/python

import sys
import re

if len(sys.argv) < 2:
    print("Use:\n    ./sco_to_hex.py <sco file name>")
    exit()
if len(sys.argv) == 2:
    opt = None
    sco_file = sys.argv[1]
elif len(sys.argv) == 3:
    opt = sys.argv[1]
    sco_file = sys.argv[2]

BPM = 185
SEQS_FILENAME = "../seqs.c"
SEQUENCER_RATE  = 1000
BITS = [None]*4

# Change these to change the order of fields in the protocol
TYPE = 0
TIME = 1
INST = 2
FREQ = 3
# Here you set the number of bits for each event field
BITS[TYPE] = 1
BITS[TIME] = 11
BITS[INST] = 2
BITS[FREQ] = 10

def hexify(ev_diff):
    """
    All information fields in a ev_diff should be ints smaller than the max
    value allowed for that field, so that hexify can just translate directly.
    """
    int_fmt_event = 0
    left_shift = 0
    for ev_num, ev_bits in enumerate(BITS):
        int_fmt_event += (ev_diff[ev_num] << left_shift)
        left_shift += ev_bits

    return "0x{val:0{digits}x}".format(val=int_fmt_event, digits=8)  # 32 / 4 = 8 digits

def max_from_bits(nbits):
    return 2**nbits-1

def freq_from_nn(nn):
    return int(110*2**((nn-45)/12))

def get_sco_list(filename):
    """
    Get a list of event lists from a .sco file
    """
    # re.search(
    sco = []
    with open(filename) as f:
        for line in f.readlines():
            if line.split() and line[0] != ';':
                ev = line.split()
                print(ev)
                sco.append(ev)
        f.close()
        print(sco)
    return sco

def ticks_to_secs(tick_num):
    return 60 * tick_num / BPM

def check_validity(event):
    if event[TYPE] > max_from_bits(BITS[TYPE]):
        raise Exception("Invalid note type: " + event[TYPE])
    if event[TIME] > max_from_bits(BITS[TIME]):
        raise Exception("Time difference too long: " + event[TIME])
    if event[FREQ] > max_from_bits(BITS[FREQ]):
        raise Exception("Frequency too high: " + event[FREQ])
    if event[INST] > max_from_bits(BITS[INST]):
        raise Exception("Invalid instrument number: " + event[FREQ])


def get_on_off_seq(sco_list):
    NOTE_ON  = 1
    NOTE_OFF = 0
    onoff_seq = []
    for ev in sco_list:
        timestamp = int(ticks_to_secs(SEQUENCER_RATE*float(ev[1])))
        duration  = int(ticks_to_secs(SEQUENCER_RATE*float(ev[2])))
        generator = int(ev[0][1])
        freq      = int(freq_from_nn(int(ev[3])))

        # Add note on / off pair to sequence
        onoff_seq.append([
            NOTE_ON,
            timestamp,
            generator,
            freq
            ])
        onoff_seq.append([
            NOTE_OFF,
            timestamp + duration,
            generator,
            freq
            ])

    onoff_seq.sort(key = lambda x: x[1])
    return onoff_seq

def write_to_file(on_off_seq):
    seq_name = sco_file.split('.')[0]
    hex_seq = "const uint32_t {}[] = {{\n".format(seq_name)
    prev_time = 0
    ev_diff = [None]*4
# Make the differential events, then make final hex seq
    try:
        for ev in on_off_seq:
            ev_diff[TYPE] = ev[0]
            ev_diff[TIME] = ev[1] - prev_time
            ev_diff[INST] = ev[2]
            ev_diff[FREQ] = ev[3]
            check_validity(ev_diff)
            hex_seq += "    {},\n".format(hexify(ev_diff))
            prev_time = ev[1]
        hex_seq += "    SEQ_TERMINATOR\n};"
    except Exception as e:
        print(e)
        exit()

    print(hex_seq)

    import os.path
# If regenerate option is supplied (or file doesn't exist):
    if opt or not os.path.isfile(SEQS_FILENAME):
        f = open(SEQS_FILENAME, 'w+')
        to_write = "#include \"sound.h\"\n\n"
        to_write += hex_seq
        f.write(to_write)
    else:
        f = open(SEQS_FILENAME, 'a')
        f.write("\n\n" + hex_seq)
    f.close()

if __name__ == "__main__":
    sco_list = get_sco_list(sco_file)
    oo_seq = get_on_off_seq(sco_list)
    write_to_file(oo_seq)
