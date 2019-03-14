BOARD?=VUMETER_V2
PYTHON?=python
BUILD?=DEBUG

MCU_VUMETER_V2:=atmega88p
F_CPU_VUMETER_V2:=20000000
PROG_VUMETER_V2:=usbasp
AVRDUDE_FLAGS_VUMETER_V2:=-B1

MCU_ARDUINO_UNO:=atmega328p
F_CPU_ARDUINO_UNO:=16000000
PROG_ARDUINO_UNO:=arduino
AVRDUDE_FLAGS_ARDUINO_UNO:=-P /dev/ttyACM0


MCU?=$(MCU_$(BOARD))
F_CPU?=$(F_CPU_$(BOARD))
PROG?=$(PROG_$(BOARD))
AVRDUDE_FLAGS=$(AVRDUDE_FLAGS_$(BOARD))


TARGET=main
SRC_DIR=src
UTILS_DIR=utils
BUILD_DIR?=build


C_SRC= \
$(SRC_DIR)/background.c \
$(SRC_DIR)/background_flipped.c \
$(SRC_DIR)/peak_indicator.c \
$(SRC_DIR)/splash.c \
$(SRC_DIR)/utils.c \
$(SRC_DIR)/fault.c \
$(SRC_DIR)/benchmark.c \
$(SRC_DIR)/ring_buffer.c \
$(SRC_DIR)/i2c_hw.c \
$(SRC_DIR)/i2c.c \
$(SRC_DIR)/oled.c \
$(SRC_DIR)/display.c \
$(SRC_DIR)/progmem_image_sprite.c \
$(SRC_DIR)/needle_coordinates.c \
$(SRC_DIR)/needle_sprite.c \
$(SRC_DIR)/adc.c \
$(SRC_DIR)/calibration.c \
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
IMAGE2C=$(PYTHON) $(SRC_DIR)/image2c.py
EEPROM_UTIL=$(PYTHON) $(UTILS_DIR)/eeprom_util.py


CFLAGS+=-mmcu=$(MCU)
CFLAGS+=-DF_CPU="$(F_CPU)UL"

CFLAGS+=-MD -MP
CFLAGS+=-Wall
CFLAGS+=-Werror
CFLAGS+=-std=c99
CFLAGS+=-ffunction-sections -fdata-sections
CFLAGS+=-funsigned-char -funsigned-bitfields
CFLAGS+=-fpack-struct -fshort-enums
CFLAGS+=-frename-registers
CFLAGS+=-g
CFLAGS+=-Wa,-ahlmsd=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.lst,$<)

CFLAGS_DEBUG=-O1
CFLAGS_RELEASE=-O2 -DNDEBUG

CFLAGS+=$(CFLAGS_$(BUILD))

LDFLAGS+=-mmcu=$(MCU)
LDFLAGS+=-static
LDFLAGS+=-Wl,-Map=$(BUILD_DIR)/map.map,--cref
LDFLAGS+=-Wl,--section-start=.eefixed=0x810080
LDFLAGS+=-Wl,-gc-sections
#LDFLAGS += -Wl,-u,vfprintf -lprintf_min
#LDFLAGS += -Wl,-u,vfprintf -lprintf_flt
#LDFLAGS+=-g
#LDLIBS+=-lm


ASFLAGS+=-mmcu=$(MCU)
ASFLAGS+=-DF_CPU="$(F_CPU)UL"
ASFLAGS+=-x assembler-with-cpp
ASFLAGS+=-Wa,-ahlms=$(<:.s=.lst),-g,--gstabs


all: $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).eep

$(SRC_DIR)/background.c: $(SRC_DIR)/images/background.bmp $(SRC_DIR)/image2c.py
	$(IMAGE2C) $< $@ BACKGROUND inverted

$(SRC_DIR)/background_flipped.c: $(SRC_DIR)/images/background_flipped.bmp $(SRC_DIR)/image2c.py
	$(IMAGE2C) $< $@ BACKGROUND_FLIPPED inverted

$(SRC_DIR)/peak_indicator.c: $(SRC_DIR)/images/peak_indicator.bmp $(SRC_DIR)/image2c.py
	$(IMAGE2C) $< $@ PEAK_INDICATOR inverted

$(SRC_DIR)/splash.c: $(SRC_DIR)/images/splash.bmp $(SRC_DIR)/image2c.py
	$(IMAGE2C) $< $@ SPLASH inverted

$(SRC_DIR)/needle_coordinates.c: $(SRC_DIR)/config.h Makefile $(SRC_DIR)/calculate_needle_coordinates.py
	$(PYTHON) $(SRC_DIR)/calculate_needle_coordinates.py 128 > $@

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET)
	$(OBJCOPY) -R .eeprom -R .eefixed -O ihex $< $@

$(BUILD_DIR)/$(TARGET).eep: $(BUILD_DIR)/$(TARGET)
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -j .eefixed --change-section-lma .eefixed=0x80 -O ihex $< $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c Makefile
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<

summary: $(BUILD_DIR)/$(TARGET)
	@echo "FLASH (.text)"
	@nm --print-size --size-sort --radix=d $(BUILD_DIR)/main | grep -i ' t '
	@echo
	@echo "RAM uninitialized (.bss)"
	@nm --print-size --size-sort --radix=d $(BUILD_DIR)/main | grep -i ' b '
	@echo
	@echo "RAM / EEPROM initialized (.data)"
	@nm --print-size --size-sort --radix=d $(BUILD_DIR)/main | grep -i ' d '
	@echo
	@echo "Summary"
	@$(SIZE) $(BUILD_DIR)/$(TARGET)

dump_eeprom: utils/eeprom_util.py
	avrdude -p $(MCU) -c $(PROG) -U eeprom:r:-:i 2>/dev/null | $(EEPROM_UTIL)

install: install_fuse_bytes install_flash install_eeprom

install_flash: $(BUILD_DIR)/$(TARGET).hex
	avrdude -p $(MCU) -c $(PROG) $(AVRDUDE_FLAGS) -U flash:w:$(BUILD_DIR)/$(TARGET).hex

install_eeprom: $(BUILD_DIR)/$(TARGET).eep
	avrdude -p $(MCU) -c $(PROG) $(AVRDUDE_FLAGS) -U eeprom:w:$(BUILD_DIR)/$(TARGET).eep

install_fuse_bytes:
	avrdude -p $(MCU) -c $(PROG) -B1000 -U lfuse:w:0xE6:m -U hfuse:w:0xD4:m


clean:
	$(RM) $(BUILD_DIR)/$(TARGET)
	$(RM) $(OBJS)
	$(RM) $(OBJS:.o=.map)
	$(RM) $(OBJS:.o=.d)
	$(RM) $(OBJS:.o=.lst)
	$(RM) $(BUILD_DIR)/map.map
	$(RM) $(BUILD_DIR)/$(TARGET).hex
	$(RM) $(BUILD_DIR)/$(TARGET).eep
	$(RM) $(SRC_DIR)/background.c
	$(RM) $(SRC_DIR)/background_flipped.c
	$(RM) $(SRC_DIR)/peak_indicator.c
	$(RM) $(SRC_DIR)/splash.c
	$(RM) $(SRC_DIR)/needle_coordinates.c

.PHONY: all summary install_fuse_bytes install clean dump_eeprom
-include $(C_OBJS:.o=.d)
