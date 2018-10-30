#Compiler and Linker
CC			:= arm-none-eabi-gcc
LD			:= arm-none-eabi-gcc
OBJCOPY		:= arm-none-eabi-objcopy

#The Target Binary Program
TARGET      := ex2-polling

#The Directories, Source, Includes, Objects, Binary and Resources
SRCDIR      := ../src
INCDIR      := ../include
BUILDDIR    := ../obj
TARGETDIR   := .
LIBDIR		:= ../lib
SRCEXT      := c
OBJEXT      := o
LINKERSCRIPT:= $(INCDIR)/efm32gg.ld

#Flags, Libraries and Includes
CFLAGS		:= -mcpu=cortex-m3 -mthumb -g -std=c99 -Wall -Wextra -Wpedantic -I$(INCDIR)
LDFLAGS		:= -mcpu=cortex-m3 -mthumb -g -lgcc -lc -lcs3 -lcs3unhosted -lefm32gg -L${LIBDIR}
LIB         := -lefm32gg
INC         := -I$(INCDIR)
INCDEP      := -I$(INCDIR)

SOURCES     := $(shell find $(SRCDIR) -type f -name '*.$(SRCEXT)')
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

all: directories $(TARGET).bin upload

directories: $(BUILDDIR)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

upload:
	-eACommander.sh -r --address 0x00000000 -f "${TARGET}.bin" -r

remake: clean all

clean:
	@$(RM) -rf $(BUILDDIR)/*.o *.elf *.bin

# Copy binary
$(TARGET).bin: $(TARGET).elf
	${OBJCOPY} -O binary $< $@

# Link
$(TARGET).elf: $(OBJECTS) ${BUILDDIR}/$(TARGET).o
	${LD} -T ${LINKERSCRIPT} $^ -o $@ ${LDFLAGS}

# Compile objects
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	${CC} ${CFLAGS} -c $< -o $@
$(BUILDDIR)/$(TARGET).o: $(TARGET).$(SRCEXT)
	${CC} ${CFLAGS} -c $< -o $@

#Non file targets
.PHONY: all remake clean upload directories
