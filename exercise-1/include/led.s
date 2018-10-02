.syntax unified

.include "efm32gg.s"

.thumb_func
enable_leds:
    // Set high drive strength on GPIO PA
    ldr r1, =GPIO_PA_BASE
    mov r2, DRIVE_LOWEST
    str r2, [r1, #GPIO_CTRL]
    // Set GPIO PA to output
    mov r2, 0x55555555
    str r2, [r1, GPIO_MODEH]
    // Enable some LEDs
    ldr r2, =0xFEFF
    str r2, [r1, GPIO_DOUT]
    bx lr

.thumb_func
increase_led_drive_strength:
    // Get the current drive strength
    ldr r0, =GPIO_PA_BASE
    ldr r1, [r0, GPIO_CTRL]
    // Mask out the relevant bits, 0 and 1
    and r1, 0b11
    cmp r1, DRIVE_LOWEST
    itt eq
    moveq r1, DRIVE_LOW
    beq done_increase_led_drive_strength
    cmp r1, DRIVE_LOW
    itt eq
    moveq r1, DRIVE_STANDARD
    beq done_increase_led_drive_strength
    cmp r1, DRIVE_STANDARD
    itt eq
    moveq r1, DRIVE_HIGH
    beq done_increase_led_drive_strength

    done_increase_led_drive_strength:
    str r1, [r0, GPIO_CTRL]
    bx lr
    
.thumb_func
decrease_led_drive_strength:
    // Get the current drive strength
    ldr r0, =GPIO_PA_BASE
    ldr r1, [r0, GPIO_CTRL]
    // Mask out the relevant bits, 0 and 1
    and r1, 0b11
    cmp r1, DRIVE_HIGH
    itt eq
    moveq r1, DRIVE_STANDARD
    beq done_decrease_led_drive_strength
    cmp r1, DRIVE_STANDARD
    itt eq
    moveq r1, DRIVE_LOW
    beq done_decrease_led_drive_strength
    cmp r1, DRIVE_LOW
    itt eq
    moveq r1, DRIVE_LOWEST
    beq done_decrease_led_drive_strength

    done_decrease_led_drive_strength:
    str r1, [r0, GPIO_CTRL]
    bx lr


.thumb_func
move_led_right:
    ldr r0, =GPIO_PA_BASE
    // Get the current status of the LEDs
    ldr r1, [r0, GPIO_DOUT]
    // Mask out the relevant bits, leaving the other bits as 1
    // We are only interested in bits 8-15
    ldr r2, =0xFFFF00FF
    orr r2, r2, r1

    // If the LED on the right end, bit 15, is active, 
    // we wrap around
    // Test if bit 15 is clear
    tst r2, #(1<<15)
    itte eq
    // Set bit 15
    orreq r2, r2, #(1<<15)
    // Clear bit 8
    biceq r2, #(1<<8) 
    // Else we shift to the left(led moves right)
    lslne r2, r2, 1
    // Store new LED state
    str r2, [r0, GPIO_DOUT]
    bx lr

.thumb_func
move_led_left:
    ldr r0, =GPIO_PA_BASE
    // Get the current status of the LEDs
    ldr r1, [r0, GPIO_DOUT]
    // Mask out the relevant bits, leaving the other bits as 1
    // We are only interested in bits 8-15
    ldr r2, =0xFFFF00FF
    orr r2, r2, r1

    // If the LED on the left end, bit 8, is active, 
    // we wrap around
    // Test if bit 8 is clear
    tst r2, #(1<<8)
    itte eq
    // Set bit 8
    orreq r2, r2, #(1<<8)
    // Clear bit 15
    biceq r2, #(1<<15) 
    // Else we shift to the left(led moves right)
    lsrne r2, r2, 1
    // Store new LED state
    str r2, [r0, GPIO_DOUT]
    bx lr


