
LD=arm-none-eabi-gcc
AS=arm-none-eabi-as
OBJCOPY=arm-none-eabi-objcopy

LDFLAGS=-nostdlib
ASFLAGS=-mcpu=cortex-m3 -mthumb -g

PROGRAMS = ex1-polling.bin ex1-interrupt.bin

LINKERSCRIPT=efm32gg.ld

# Prevent removing intermediate elf and object files
.PRECIOUS : %.elf %.o

all: $(PROGRAMS)

%.bin : %.elf
	${OBJCOPY} -j .text -O binary $< $@

%.elf : %.o
	${LD} -T ${LINKERSCRIPT} $^ -o $@ ${LDFLAGS} 

%.o : %.s
	${AS} ${ASFLAGS} $< -o $@

.PHONY : upload
upload :
	-eACommander.sh -r --address 0x00000000 -f "ex1-polling.bin" -r

.PHONY : upload_polling
upload_polling :
	-eACommander.sh -r --address 0x00000000 -f "ex1-polling.bin" -r

.PHONY : upload_interrupt
upload_interrupt :
	-eACommander.sh -r --address 0x00000000 -f "ex1-interrupt.bin" -r

.PHONY : clean
clean :
	-rm -rf *.o *.elf *.bin *.hex
