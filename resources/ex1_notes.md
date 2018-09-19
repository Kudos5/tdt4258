Exercise 1 - Notes
==================
Before starting assembly programming, you should read appendices B and C in
the exercise compendium. These will probably answer a lot of questions that
would otherwise arise during programming. This document will repeat some of it, 
but will also contains newfound knowledge from other sources, i.e. heavy googling.

New knowledge
-------------
- The code shown in figure 3.3 in the exercise compendium does not actually
  contain any assembly instructions, only **pseudoinstructions** (appendix B.5).
- Pseudoinstructions are also called **directives**, because they are *directions* 
  for the assembler. You can find a description of some directives 
  [here](https://sourceware.org/binutils/docs/as/ARM-Directives.html) and 
  [here](https://docs.oracle.com/cd/E26502_01/html/E28388/eoiyg.html).
- The `.section .<name>` directive lets you define **sections** (or segments) 
  of code. The "official" 
  ([ELF](http://www.delorie.com/gnu/docs/binutils/as_135.html)) sections, which 
  are actually used by the linker (?) are:
    + `.text` - contains code
    + `.data` - contains data, eg. constants.
    + `.bss`  - ?
    
  As you can see in our exercise 1 code, `exception_vector.s` defines its own 
  `.vectors` section. If you open the linker file (`efm32gg.ld`), you can see what
  this is used for: It makes sure our exception vector is placed *first* in the ROM,
  which is something we have to do if we want our program to work.
- The **linker file** contains a *linker script*. The main purpose of this is
  summarized in the three commands you can see in `efm32gg.ld`:
    + `ENTRY`: Sets the address of the first instruction to execute in our
      program, which is our reset handler in this case.
    + `MEMORY`: Describes the *location* and *size* of blocks of memory in the
      target and *names* them so that certain `.section`s can be assigned to certain
      memory regions.
    + `SECTIONS`: Describes how sections in the input files should be mapped to 
    the output sections, and how to place the output sections in memory.
  [More](https://sourceware.org/binutils/docs/ld/Scripts.html) about LD scripts.

  In this linker file you will see that both the `.vectors` and the `.text`
  sections are put into a new `.text` section that is written to the ROM, while 
  `.data` is put in RAM.
- *Not sure about this*:
    + *Thumb instructions* are variable length, either *n*arrow or *w*ide. ..(?)
	+ *Thumb* is an instruction set. *Thumb 2* is just some technology?    ..(?) 

### From ARM Thumb-2 Quick Reference
- `b[cc]`, where `[cc]` is a conditional (like `eq`, `le`, `ge`) can be used on its
  own after a `cmp` instruction to branch to anywhere in the code.
- *Any* other conditional logic needs to use the `it` instruction. 
  `it` **syntax example**:
  ```assembly
  cmp r0, r1		; calculates r0 - r1 and sets status flags 
  ; Perform the next instruction if r0 and r1 are equal
  it eq             ; "if equal..." 
  bleq <label>      ; "...then"

  cmp r0, r1 
  ; Two mov instructions if r0 is greater than or equal to r1, *one* if less than
  itte ge           ; "if greater than..."  (i)
  movge <stuff>     ; "...then mov"         (t)
  movge <stuff>     ; "...then mov"         (t)
  movlt <stuff>     ; "...esle mov"         (e)
  ```
- `b <label>` - branches to the address given (here in the form of a label)
- `bx <reg>`  - branches to the address found in the given register (*indirect*). 
  Example: `bx lr` will branch to the address stored in the link register (lr).


