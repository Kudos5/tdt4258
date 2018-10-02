LD=arm-none-eabi-gcc
AS=arm-none-eabi-as
OBJCOPY=arm-none-eabi-objcopy

LDFLAGS=-nostdlib
ASFLAGS=-mcpu=cortex-m3 -mthumb -g
IDIR=../include

PROGRAM=ex1-interrupt.bin 
UPLOAD=-eACommander.sh -r --address 0x00000000 -f $(PROGRAM) -r

LINKERSCRIPT=efm32gg.ld

# Prevent removing intermediate elf and object files
.PRECIOUS : %.elf %.o

all: $(PROGRAM)
	${UPLOAD}

%.bin : %.elf
	${OBJCOPY} -j .text -O binary $< $@

%.elf : %.o
	${LD} -L${IDIR} -T ${LINKERSCRIPT} $^ -o $@ ${LDFLAGS} 

%.o : %.s
	${AS} -I${IDIR} ${ASFLAGS} $< -o $@

.PHONY : upload
upload :
	${UPLOAD}

.PHONY : clean
clean :
	-rm -rf *.o *.elf *.bin *.hex
