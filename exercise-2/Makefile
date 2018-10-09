
PROGRAMS = ex2-polling.bin ex2-interrupt.bin ex2-dma.bin

CC=arm-none-eabi-gcc
LD=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

CFLAGS=-mcpu=cortex-m3 -mthumb -g -std=c99 -Wall -Wextra -Wpedantic
LDFLAGS=-mcpu=cortex-m3 -mthumb -g -lgcc -lc -lcs3 -lcs3unhosted -lefm32gg -Llib
ASFLAGS=-mcpu=cortex-m3 -mthumb -g
LINKERSCRIPT=lib/efm32gg.ld
OBJ=dac.o gpio.o timer.o interrupt_handlers.o sound.o seqs.o


# Prevent removing intermediate elf and object files
.PRECIOUS : %.elf %.o

all: $(PROGRAMS)

%.bin : %.elf
	${OBJCOPY} -O binary $< $@

%.elf : %.o ${OBJ}
	${LD} -T ${LINKERSCRIPT} $^ -o $@ ${LDFLAGS} 

%.o : %.c
	${CC} ${CFLAGS} -c $< -o $@

polling: ex2-polling.bin

.PHONY : upload
upload :
	-eACommander.sh -r --address 0x00000000 -f "ex2-polling.bin" -r

.PHONY : upload_polling
upload_polling :
	-eACommander.sh -r --address 0x00000000 -f "ex2-polling.bin" -r

.PHONY : upload_interrupt
upload_interrupt :
	-eACommander.sh -r --address 0x00000000 -f "ex2-interrupt.bin" -r

.PHONY : upload_dma
upload_dma :
	-eACommander.sh -r --address 0x00000000 -f "ex2-dma.bin" -r

.PHONY : clean
clean :
	-rm -rf *.o *.elf *.bin *.hex
