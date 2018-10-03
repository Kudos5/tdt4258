#include "sound.h"

/* TODO : Something is either wrong with this sequence or with the square generator.
 * Find out which when stuff kind of works on the board.  */

const uint32_t seq[] = {
    0x00208001,
    0x00208144,
    0x00248001,
    0x00248144,
    0x00124001,
    0x00124144,
    0x00124145,
    0x00124288,
    0x00208001,
    0x00208144,
    0x00248003,
    0x00248144,
    0x00124001,
    0x00124144,
    0x00124145,
    0x00124288,
    0x0015c001,
    0x0015c288,
    0x00184003,
    0x00184288,
    0x00208001,
    0x00208144,
    0x00248001,
    0x00248144,
    0x00124001,
    0x00124140,
    0x0012414b,
    0x00124288,
    0x00208001,
    0x00208144,
    0x00248001,
    0x00248144,
    0x00124001,
    0x00124144,
    0x00124147,
    0x00124288,
    0x00184001,
    0x00184144,
    0x0015c001,
    0x0015c144,
    0x00148001,
    0x00148288,
    0x00208003,
    0x00208144,
    0x00248001,
    0x00248144,
    0x00124001,
    0x00124144,
    0x00124145,
    0x00124288,
    0x00208003,
    0x00208144,
    0x00248001,
    0x00248144,
    0x00124001,
    0x00124144,
    0x00124145,
    0x00124288,
    0x0015c003,
    0x0015c288,
    0x00184001,
    0x00184288,
    0x00208001,
    0x00208144,
    0x00248001,
    0x00248144,
    0x00124003,
    0x00124144,
    0x00124145,
    0x00124288,
    0x00208001,
    0x00208144,
    0x00248001,
    0x00248144,
    0x00124003,
    0x00124288,
    0x002b8001,
    0x002b8288,
    0x00290001,
    0x00290144,
    0x00248001,
    0x00248144,
    0x00208003,
    0x00208144,
    0x00208145,
    0x00495001,
    0x00208144,
    0x00248001,
    0x00248144,
    0x00124001,
    0x00495000,
    0x00495145,
    0x001240a2,
    0x001240a5,
    0x00495142,
    0x00124146,
    0x00208001,
    0x00495001,
    0x00208144,
    0x00248001,
    0x00248144,
    0x00124001,
    0x00495000,
    0x00124144,
    0x00495001,
    0x00124147,
    0x0049513a,
    0x0012414e,
    0x0015c001,
    0x00575001,
    0x0015c288,
    0x00184001,
    0x00575000,
    0x0061d001,
    0x00184288,
    0x00208001,
    0x0061d000,
    0x00495001,
    0x00208144,
    0x00248003,
    0x00495142,
    0x00248002,
    0x00124001,
    0x00124144,
    0x00495001,
    0x00124145,
    0x00495144,
    0x00124144,
    0x00208001,
    0x00495001,
    0x00208144,
    0x00248003,
    0x00248144,
    0x00124001,
    0x0049500e,
    0x00124136,
    0x00495001,
    0x00124145,
    0x00495144,
    0x00124144,
    0x00184001,
    0x00525001,
    0x00184144,
    0x0015c003,
    0x00525142,
    0x0015c002,
    0x00148001,
    0x00575001,
    0x00148288,
    0x00208001,
    0x00575000,
    0x00495001,
    0x00208144,
    0x00248001,
    0x00248144,
    0x00124001,
    0x00495000,
    0x00124144,
    0x00495003,
    0x00124145,
    0x00495144,
    0x00124144,
    0x00208001,
    0x00495001,
    0x00208144,
    0x00248001,
    0x00248144,
    0x00124001,
    0x00495000,
    0x00124144,
    0x00495003,
    0x00124145,
    0x0049511c,
    0x0012416c,
    0x0015c001,
    0x00575001,
    0x0015c288,
    0x00184001,
    0x00575000,
    0x0061d001,
    0x00184288,
    0x0061d000,
    0x00208003,
    0x00495001,
    0x0020813a,
    0x0024800b,
    0x00248144,
    0x00124001,
    0x00495000,
    0x00124144,
    0x00495001,
    0x00124145,
    0x0049515c,
    0x0012412c,
    0x00208003,
    0x00495001,
    0x00208144,
    0x00248001,
    0x00248144,
    0x00124001,
    0x00495000,
    0x00495145,
    0x00124144,
    0x0030c001,
    0x0030c288,
    0x00495000,
    0x002b8003,
    0x0082d001,
    0x002b8144,
    0x00290001,
    0x00290144,
    0x00248001,
    0x0082d000,
    0x0092d001,
    0x00248198,
    0x0092d0f0,
    SEQ_TERMINATOR
};