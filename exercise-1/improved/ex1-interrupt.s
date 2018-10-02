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
    bl enable_interrupts
    ldr r0, =SCR
    mov r1, 6
    str r1, [r0]
sleep:
    wfi        // wait for interrupt (sleep)
    b sleep    // go back to sleep after returning from interrupt

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//
// GPIO handler
// The CPU will jump here when there is a GPIO interrupt
//
/////////////////////////////////////////////////////////////////////////////

.thumb_func
gpio_handler:  
    // Push link register value to stack (for branching back from the interrupt)
    push {lr}
    // load GPIO base address from memory
    ldr r0, =GPIO_BASE
    ldr r1, [r0, GPIO_IF]
    // Clear the interrupt to avoid repeating interrupts
    str r1, [r0, GPIO_IFC]

    // Load button state
    ldr r0, =GPIO_PC_BASE
    ldr r1, [r0, GPIO_DIN]

    /************************************************************************/
    /**** Check which button was pressed and take the appropriate action ****/

    /* Move the light in the right direction if left / right was pressed */
    // Check right buttons
    cmp r1, 0b11111011
    it eq
    bleq move_led_right
    cmp r1, 0b10111111
    it eq
    bleq move_led_right

    // Check left buttons
    cmp r1, 0b11111110
    it eq
    bleq move_led_left
    cmp r1, 0b11101111
    it eq
    bleq move_led_left

    /* If up/down button was pressed then increase/decrease brightness */
    // Check up buttons
    cmp r1, 0b11111101
    it eq
    bleq increase_led_drive_strength
    cmp r1, 0b11011111
    it eq
    bleq increase_led_drive_strength
    // Check down buttons
    cmp r1, 0b11110111
    it eq
    bleq decrease_led_drive_strength
    cmp r1, 0b01111111
    it eq
    bleq decrease_led_drive_strength
    /************************************************************************/

    // Get interrupt lr from stack
    pop {lr}
    bx lr

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//
// Setup procedures
// Enable GPIO and interrupts
//
/////////////////////////////////////////////////////////////////////////////

.thumb_func
enable_interrupts:
    /* Enable interrupts for GPIO C when its state changes */
    ldr r1, =0x22222222
    ldr r0, =GPIO_BASE
    str r1, [r0, #GPIO_EXTIPSELL]
    // Set up the GPIO to interrupt when a bit changes from 1 to 0 (button pressed)
    mov r1, 0xFF
    str r1, [r0, #GPIO_EXTIFALL]
    /* Do _not* set up the GPIO to interrupt when a bit changes from 0 to 1 */
    // str r1, [r0, #GPIO_EXTIRISE]
    // Set up interrupt generation
    str r1, [r0, #GPIO_IEN]
    // Clear interrupt flags to avoid interrupt on startup
    ldr r0, =GPIO_BASE
    ldr r1, [r0, #GPIO_IF]
    str r1, [r0, #GPIO_IFC]
    // Enable CPU interrupts
    ldr r1, =0x0802
    ldr r2, =ISER0
    str r1, [r2]
    bx lr

.thumb_func
enable_gpio_clock:
    // Load the cmu base address
    ldr r1, =CMU_BASE
    // Load current value of HFPERCLK_ENABLE
    ldr r2, [r1, #CMU_HFPERCLKEN0]
    // Set the gpio enable bit
    mov r3, #1
    lsl r3, r3, #CMU_HFPERCLKEN0_GPIO
    orr r2, r2, r3
    // Store the result back in memory
    str r2, [r1, #CMU_HFPERCLKEN0]
    // Return to caller
    bx lr
    
/////////////////////////////////////////////////////////////////////////////

.thumb_func
dummy_handler:  
    b .  // do nothing

