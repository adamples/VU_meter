MCU=atmega328p
F_CPU=20000000
PROG=usbasp


TARGET=main
SRC_DIR=src
BUILD_DIR?=build


C_SRC= \
$(SRC_DIR)/background.c \
$(SRC_DIR)/peak_indicator.c \
$(SRC_DIR)/lcd.c \
$(SRC_DIR)/fault.c \
$(SRC_DIR)/benchmark.c \
$(SRC_DIR)/ring_buffer.c \
$(SRC_DIR)/i2c.c \
$(SRC_DIR)/ssd1306.c \
$(SRC_DIR)/display.c \
$(SRC_DIR)/progmem_image_sprite.c \
$(SRC_DIR)/needle_coordinates.c \
$(SRC_DIR)/needle_sprite.c \
$(SRC_DIR)/adc.c \
$(SRC_DIR)/$(TARGET).c

C_OBJS=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRC))
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
CFLAGS+=-Wa,-ahlmsd=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.lst,$<)


LDFLAGS+=-mmcu=$(MCU)
LDFLAGS+=-static
LDFLAGS+=-Wl,-Map=$(BUILD_DIR)/map.map,--cref
LDFLAGS+=-Wl,-gc-sections
#LDFLAGS += -Wl,-u,vfprintf -lprintf_min
#LDFLAGS += -Wl,-u,vfprintf -lprintf_flt
#LDFLAGS+=-g
LDLIBS+=-lm


ASFLAGS+=-mmcu=$(MCU)
ASFLAGS+=-DF_CPU="$(F_CPU)UL"
ASFLAGS+=-x assembler-with-cpp
ASFLAGS+=-Wa,-ahlms=$(<:.s=.lst),-g,--gstabs


all: $(BUILD_DIR)/$(TARGET).hex

$(SRC_DIR)/background.c: $(SRC_DIR)/images/background.png
	$(IMAGE2C) $< $@ BACKGROUND

$(SRC_DIR)/peak_indicator.c: $(SRC_DIR)/images/peak_indicator.png
	$(IMAGE2C) $< $@ PEAK_INDICATOR

$(SRC_DIR)/needle_coordinates.c: $(SRC_DIR)/config.h Makefile $(SRC_DIR)/calculate_needle_coordinates.py
	$(PYTHON) $(SRC_DIR)/calculate_needle_coordinates.py 128 > $@

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c Makefile
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<

summary: $(BUILD_DIR)/$(TARGET)
	nm --print-size --size-sort --radix=d main
	$(SIZE) $(BUILD_DIR)/$(TARGET)


install: $(BUILD_DIR)/$(TARGET)
	avrdude -p $(MCU) -c $(PROG) -U flash:w:$(BUILD_DIR)/$(TARGET).hex #-U eeprom:w:$(BUILD_DIR)/$(TARGET).eep


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
-include $(C_OBJS:.o=.d)
