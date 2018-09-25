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
    // load register from memory
    ldr r0, =GPIO_PC_BASE
    /* ^ This is a pseudoinstruction. ldr does not actually take immediates, but
     * puts them in a [literal pool](https://en.wikipedia.org/wiki/Literal_pool)
     * that it reads from during runtime. */
    /* Instead of reading the actual pin state, we read the GPIO_IF register to
     * see which pin was changed. That way we don't have to bitmask before comparison  */
    ldr r1, [r0, GPIO_IF]
    // Clear the interrupt to avoid repeating interrupts
    str r1, [r0, GPIO_IFC]

    // Assumes that the interrupt is active high. TODO: Is this the case?
    // If a left/right button was pressed, move the light to the left/right
    // check right
    cmp r1, 0b00000100
    it eq
    bleq move_led_right
    cmp r1, 0b01000000
    it eq
    bleq move_led_right
    // check left
    cmp r1, 0b00000001
    it eq
    bleq move_led_left
    cmp r1, 0b00010000
    it eq
    bleq move_led_left

    // If up/down button was pressed then increase/decrease brightness
    // check up
    cmp r1, 0b00000010
    it eq
    bleq increase_led_drive_strength
    cmp r1, 0b00100000
    it eq
    bleq increase_led_drive_strength
    // check down
    cmp r1, 0b00001000
    it eq
    bleq decrease_led_drive_strength
    cmp r1, 0b10000000
    it eq
    bleq decrease_led_drive_strength
    bx lr       // assuming the link register is updated upon interrupt. TODO: Find out if this is the case 


/////////////////////////////////////////////////////////////////////////////

.thumb_func
dummy_handler:  
    b .  // do nothing

/////////////////////////////////////////////////////////////////////////////

enable_interrupts:
    /* Enable interrupts for GPIO C when its state changes */
    ldr r0, =GPIO_PC_BASE
    ldr r1, =0x22222222
    str r1, [r0, GPIO_EXTIPSELL]
    // Set up the GPIO to interrupt when a bit changes from 1 to 0
    mov r1, 0xFF
    str r1, [r0, GPIO_EXTIFALL]
    // Set up the GPIO to interrupt when a bit changes from 0 to 1
    str r1, [r0, GPIO_EXTIRISE]
    // Set up interrupt generation
    str r1, [r0, GPIO_IEN]
    ldr r1, =0x0802
    ldr r2, =ISER0
    str r1, [r2]
    bx lr

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
    
