RM=rm -f
MKDIR_P=mkdir -p

CC=avr-gcc
INC=-I ./include
CFLAGS=-pedantic-errors -c $(INC) -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -mmcu=atmega328p -Os -g
LDFLAGS=

# dirs
SRCDIR=src
OUTPUTDIR=bin
BUILDDIR=build

MAIN_SRC=program

# files
SOURCES=$(shell find $(SRCDIR) -type f -name '*.c')
OBJECTS=$(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:%.c=%.o))
TARGET=$(OUTPUTDIR)/$(MAIN_SRC).elf
OUT_HEX=$(OUTPUTDIR)/$(MAIN_SRC).hex

#

fuses:
	sudo avrdude -p atmega328p -c usbasp -P usb -b19200 -U lfuse:w:0xF7:m

flash: hex
	sudo avrdude \
		-p atmega328p \
		-c usbasp \
		-U flash:w:$(OUTPUTDIR)/$(MAIN_SRC).hex:i \
		-P usb -b19200

#

hex: $(OUT_HEX)

$(OUT_HEX): $(TARGET)
	avr-objcopy \
		-j .text \
		-j .data \
		-O ihex $(TARGET) $@

objects: $(OBJECTS) 
elf: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -mmcu=atmega328p $(LDFLAGS) $(OBJECTS) -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

#

chkmem:
	avr-size -C --mcu=atmega328p $(TARGET)

disassembly:
	avr-objdump -xhDrglSw $(TARGET) | less

clean:
	$(RM) $(BUILDDIR)/*.o

remove:
	$(RM) $(TARGET)
	${RM} ${OUT_HEX}

.PHONY: all clean cleandeps remove builddirs test run chkmem fuses disassembly
