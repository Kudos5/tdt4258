#Compiler and Linker
CC			:= arm-none-eabi-gcc
LD			:= arm-none-eabi-gcc
OBJCOPY		:= arm-none-eabi-objcopy

#The Target Binary Program
TARGET      := ex2-interrupt

#The Directories, Source, Includes, Objects, Binary and Resources
SRCDIR      := ../src
INCDIR      := ../include
BUILDDIR    := ../obj
TARGETDIR   := .
# RESDIR      := res
LIBDIR		:= ../lib
SRCEXT      := c
# DEPEXT      := d
OBJEXT      := o
LINKERSCRIPT:= $(INCDIR)/efm32gg.ld

#Flags, Libraries and Includes
CFLAGS		:= -mcpu=cortex-m3 -mthumb -g -std=c99 -Wall -Wextra -Wpedantic -I$(INCDIR)
LDFLAGS		:= -mcpu=cortex-m3 -mthumb -g -lgcc -lc -lcs3 -lcs3unhosted -lefm32gg -L${LIBDIR}
LIB         := -lefm32gg
INC         := -I$(INCDIR)
INCDEP      := -I$(INCDIR)

#---------------------------------------------------------------------------------
#DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------
SOURCES     := $(shell find $(SRCDIR) -type f -name '*.$(SRCEXT)')
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

#Defauilt Make
all: $(TARGET).bin upload

upload:
	-eACommander.sh -r --address 0x00000000 -f "${TARGET}.bin" -r

#Remake
remake: clean all

# Copy Resources from Resources Directory to Target Directory
# resources: directories
#    @cp $(RESDIR)/* $(TARGETDIR)/

#Make the Directories
directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

# Clean only Objecst
clean:
	@$(RM) -rf $(BUILDDIR)/*.o *.elf *.bin

# Pull in dependency info for *existing* .o files
# -include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))

$(TARGET).bin: $(TARGET).elf
	${OBJCOPY} -O binary $< $@

#Link
$(TARGET).elf: $(OBJECTS) ${BUILDDIR}/$(TARGET).o
	${LD} -T ${LINKERSCRIPT} $^ -o $@ ${LDFLAGS}

$(BUILDDIR)/$(TARGET).o: $(TARGET).$(SRCEXT)
	${CC} ${CFLAGS} -c $< -o $@

#Compile
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	${CC} ${CFLAGS} -c $< -o $@


#Non-File Targets
.PHONY: all remake clean upload
