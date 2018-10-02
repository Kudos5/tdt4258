.syntax unified

.include "efm32gg.s"

.thumb_func
enable_buttons:
    // Set GPIO PC to input
    ldr r0, =GPIO_PC_BASE
    mov r1, 0x33333333
    str r1, [r0, GPIO_MODEL]
    // Enable internal pullup
    mov r1, 0xFF
    str r1, [r0, GPIO_DOUT]
    bx lr

