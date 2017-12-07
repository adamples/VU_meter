MCU=atmega328p
F_CPU=20000000
PROG=usbasp


TARGET=main

C_SRC= \
$(TARGET).c \
lcd.c \
fault.c \
fixed_point.c \
benchmark.c \
ring_buffer.c \
i2c.c \
background.c \
peak_indicator.c \
ssd1306.c \
display.c \
progmem_image_sprite.c \
needle_coordinates.c \
needle_sprite.c \
adc.c

C_OBJS=$(C_SRC:.c=.o)
OBJS=$(C_OBJS)


CC=avr-gcc
CXX=avr-g++
LD=avr-ld
AR=avr-ar
AS=avr-gcc -c
OBJCOPY=avr-objcopy
OBJDUMP=avr-objdump
SIZE=avr-size
RM=rm -f --
PYTHON=python3
IMAGE2C=$(PYTHON) image2c.py


CFLAGS+=-mmcu=$(MCU)
CFLAGS+=-DF_CPU="$(F_CPU)UL"

CFLAGS+=-O2
CFLAGS+=-MD -MP
CFLAGS+=-Wall
CFLAGS+=-Werror
CFLAGS+=-std=c99
#~ CFLAGS+=-DNDEBUG
CFLAGS+=-ffunction-sections -fdata-sections
CFLAGS+=-funsigned-char -funsigned-bitfields
CFLAGS+=-fpack-struct -fshort-enums
CFLAGS+=-frename-registers
CFLAGS+=-g
CFLAGS+=-Wa,-ahlmsd=$(<:.c=.lst)


LDFLAGS+=-mmcu=$(MCU)
LDFLAGS+=-static
LDFLAGS+=-Wl,-Map=map.map,--cref
LDFLAGS+=-Wl,-gc-sections
# Minimalna wersja printf
#LDFLAGS += -Wl,-u,vfprintf -lprintf_min
# Zmiennoprzecinkowa wersja printf (wymaga biblioteki matematycznej)
#LDFLAGS += -Wl,-u,vfprintf -lprintf_flt
#LDFLAGS+=-g
LDLIBS+=-lm


ASFLAGS+=-mmcu=$(MCU)
ASFLAGS+=-DF_CPU="$(F_CPU)UL"
ASFLAGS+=-x assembler-with-cpp
ASFLAGS+=-Wa,-ahlms=$(<:.s=.lst),-g,--gstabs


all: $(TARGET).hex

background.c: images/background.png
	$(IMAGE2C) $< $@ BACKGROUND

peak_indicator.c: images/peak_indicator.png
	$(IMAGE2C) $< $@ PEAK_INDICATOR

needle_coordinates.c: config.h Makefile calculate_needle_coordinates.py
	$(PYTHON) calculate_needle_coordinates.py 128 > $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.hex: %
	$(OBJCOPY) -O ihex -R .eeprom $< $@

%.o: %.c Makefile
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c $< -o $@


summary: $(TARGET)
	nm --print-size --size-sort --radix=d main
	$(SIZE) $(TARGET)


install: $(TARGET)
	avrdude -p $(MCU) -c $(PROG) -U flash:w:$(TARGET) #-U eeprom:w:$(TARGET).eep


clean:
	$(RM) $(TARGET)
	$(RM) $(OBJS)
	$(RM) $(OBJS:.o=.map)
	$(RM) $(OBJS:.o=.d)
	$(RM) $(OBJS:.o=.lst)
	$(RM) map.map
	$(RM) $(TARGET).hex
	$(RM) $(TARGET).eep
	$(RM) background.c
	$(RM) peak_indicator.c

.PHONY: all summary install clean
-include $(C_SRC:.c=.d)
