.syntax unified

.include "efm32gg.s"
.include "led.s"
.include "button.s"
.include "exception_vector.s"
.section .text

/////////////////////////////////////////////////////////////////////////////
//
// Reset handler
// The CPU will start executing here after a reset
//
/////////////////////////////////////////////////////////////////////////////

.globl  _reset
.type   _reset, %function
.thumb_func
_reset: 
    bl enable_gpio_clock
    bl enable_leds
    bl enable_buttons
    bl poll_buttons
    b .  // do nothing

/////////////////////////////////////////////////////////////////////////////

// Function used for detecting button presses by polling
.thumb_func
poll_buttons:
    // load register from memory
    ldr r0, =GPIO_PC_BASE
    ldr r1, [r0, GPIO_DIN]

    // Continue only if there was a change in state.
    // Previous state stored in r3
    cmp r1, r3
    itt eq
    moveq r3, r1
    beq poll_buttons

    // Save state in case subroutine changes it
    mov r3, r1
    push {r3}

    // If a left/right button was pressed, move the light to the left/right
    // check right
    cmp r1, 0b11111011
    it eq
    bleq move_led_right
    cmp r1, 0b10111111
    it eq
    bleq move_led_right
    // check left
    cmp r1, 0b11111110
    it eq
    bleq move_led_left
    cmp r1, 0b11101111
    it eq
    bleq move_led_left

    // If up/down button was pressed then increase/decrease brightness
    // check up
    cmp r1, 0b11111101
    it eq
    bleq increase_led_drive_strength
    cmp r1, 0b11011111
    it eq
    bleq increase_led_drive_strength
    // check down
    cmp r1, 0b11110111
    it eq
    bleq decrease_led_drive_strength
    cmp r1, 0b01111111
    it eq
    bleq decrease_led_drive_strength

    // Restore state to r3
    pop {r3}
    // Repeat
    b poll_buttons



/////////////////////////////////////////////////////////////////////////////
//
// GPIO handler
// The CPU will jump here when there is a GPIO interrupt
//
/////////////////////////////////////////////////////////////////////////////

.thumb_func
gpio_handler:  
    b .  // do nothing

/////////////////////////////////////////////////////////////////////////////

.thumb_func
dummy_handler:  
    b .  // do nothing

/////////////////////////////////////////////////////////////////////////////

.thumb_func
enable_gpio_clock:
    // load the cmu base address
    ldr r1, =CMU_BASE
    // load current value of HFPERCLK_ENABLE
    ldr r2, [r1, #CMU_HFPERCLKEN0]
    // Set the gpio enable bit
    mov r3, #1
    lsl r3, r3, #CMU_HFPERCLKEN0_GPIO
    orr r2, r2, r3
    // Store the result back in memory
    str r2, [r1, #CMU_HFPERCLKEN0]
    // Return to caller
    bx lr
    
