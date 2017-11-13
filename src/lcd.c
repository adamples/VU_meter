#include "lcd.h"
#include <stdlib.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/cpufunc.h>
#include <util/delay.h>


/** Złączenie dwóch elementów */
#define CONCAT2(a, b) a ## b

/** Złączenie trzech elementów */
#define CONCAT3(a, b, c) a ## b ## c

/** Rejestr kierunku portu na podstawie litery, np. DDR(C) -> DDRC */
#define DDR(port) CONCAT2(DDR, port)

/** Rejestr wyjściowy portu na podstawie litery, np. PORT(C) -> PORTC */
#define PORT(port) CONCAT2(PORT, port)

/** Rejestr wejściowy portu na podstawie litery, np. PIN(C) -> PINC */
#define PIN(port) CONCAT2(PIN, port)

/** Pin portu na podstawie litery i liczby, np. P(C, 5) -> PC5 */
#define P(port, pin) CONCAT3(P, port, pin)

/** Ustawia pin portu jako wyjście, np. SET_AS_OUTPUT(C, 5) */
#define SET_AS_OUTPUT(port, pin) { DDR(port) |= _BV(CONCAT3(P, port, pin)); }

/** Ustawia pin portu jako wejście, np. SET_AS_INPUT(C, 5) */
#define SET_AS_INPUT(port, pin, pullup) { DDR(port) &= ~_BV(CONCAT3(P, port, pin)); if (pullup) PORT(port) |= _BV(CONCAT3(P, port, pin)); }



#define LCD_DELAY()       _NOP()
#define LCD_EN_HIGH()     PORT(LCD_EN_PORT) |= _BV(LCD_EN_PIN)
#define LCD_EN_LOW()      PORT(LCD_EN_PORT) &= ~_BV(LCD_EN_PIN)
#define LCD_RS_HIGH()     PORT(LCD_RS_PORT) |= _BV(LCD_RS_PIN)
#define LCD_RS_LOW()      PORT(LCD_RS_PORT) &= ~_BV(LCD_RS_PIN)

#define LCD_EN_TOGGLE()   do { LCD_EN_HIGH(); LCD_DELAY(); LCD_EN_LOW(); } while (0)


typedef enum _mode_t {
  MODE_COMMAND,
  MODE_DATA
} mode_t;


const lcd_charset_t LCD_POLISH_DIACRITICS PROGMEM = {
  {
    0b00000000,
    0b00011111,
    0b00011111,
    0b00011111,
    0b00011111,
    0b00011111,
    0b00011111,
    0b00000000
  }, {
    0b00000000,
    0b00001110,
    0b00000001,
    0b00001111,
    0b00010001,
    0b00001111,
    0b00000010,
    0b00000001
  }, {
    0b00000111,
    0b00000000,
    0b00001110,
    0b00010000,
    0b00010000,
    0b00010001,
    0b00001110,
    0b00000000
  }, {
    0b00000000,
    0b00001110,
    0b00010001,
    0b00011111,
    0b00010000,
    0b00001110,
    0b00000100,
    0b00000010
  }, {
    0b00001100,
    0b00000100,
    0b00000100,
    0b00000110,
    0b00001100,
    0b00000100,
    0b00001110,
    0b00000000,
  }, {
    0b00000111,
    0b00000000,
    0b00001110,
    0b00010001,
    0b00010001,
    0b00010001,
    0b00001110,
    0b00000000
  }, {
    0b00000111,
    0b00000000,
    0b00001111,
    0b00010000,
    0b00001110,
    0b00000001,
    0b00011110,
    0b00000000
  }, {
    0b00000100,
    0b00000000,
    0b00011111,
    0b00000010,
    0b00000100,
    0b00001000,
    0b00011111,
    0b00000000
  }
};


void lcd_write(uint8_t data, mode_t mode) {

  if (mode == MODE_DATA)
     LCD_RS_HIGH();
  else
     LCD_RS_LOW();

  PORT(LCD_D7_PORT) &= ~_BV(LCD_D7_PIN);
  PORT(LCD_D6_PORT) &= ~_BV(LCD_D6_PIN);
  PORT(LCD_D5_PORT) &= ~_BV(LCD_D5_PIN);
  PORT(LCD_D4_PORT) &= ~_BV(LCD_D4_PIN);

  if (data & 0x80) PORT(LCD_D7_PORT) |= _BV(LCD_D7_PIN);
  if (data & 0x40) PORT(LCD_D6_PORT) |= _BV(LCD_D6_PIN);
  if (data & 0x20) PORT(LCD_D5_PORT) |= _BV(LCD_D5_PIN);
  if (data & 0x10) PORT(LCD_D4_PORT) |= _BV(LCD_D4_PIN);

  LCD_EN_TOGGLE();

  PORT(LCD_D7_PORT) &= ~_BV(LCD_D7_PIN);
  PORT(LCD_D6_PORT) &= ~_BV(LCD_D6_PIN);
  PORT(LCD_D5_PORT) &= ~_BV(LCD_D5_PIN);
  PORT(LCD_D4_PORT) &= ~_BV(LCD_D4_PIN);

  if (data & 0x08) PORT(LCD_D7_PORT) |= _BV(LCD_D7_PIN);
  if (data & 0x04) PORT(LCD_D6_PORT) |= _BV(LCD_D6_PIN);
  if (data & 0x02) PORT(LCD_D5_PORT) |= _BV(LCD_D5_PIN);
  if (data & 0x01) PORT(LCD_D4_PORT) |= _BV(LCD_D4_PIN);

  LCD_EN_TOGGLE();
  LCD_RS_LOW();

  _delay_us(41);

  if (mode == MODE_COMMAND && data == LCD_CMD_CLEAR)
    _delay_ms(2);
}


void lcd_set_contrast(uint8_t v) {
}


void lcd_init() {

  /* Set all used lines as outputs */
  SET_AS_OUTPUT(LCD_D7_PORT, LCD_D7_PIN);
  SET_AS_OUTPUT(LCD_D6_PORT, LCD_D6_PIN);
  SET_AS_OUTPUT(LCD_D5_PORT, LCD_D5_PIN);
  SET_AS_OUTPUT(LCD_D4_PORT, LCD_D4_PIN);
  SET_AS_OUTPUT(LCD_RS_PORT, LCD_RS_PIN);
  SET_AS_OUTPUT(LCD_EN_PORT, LCD_EN_PIN);
  // SET_AS_OUTPUT(LCD_LIGHT_PORT, LCD_LIGHT_PIN);

  /* Backlight on */
  // PORT(LCD_LIGHT_PORT) |= _BV(LCD_LIGHT_PIN);

  LCD_EN_LOW();
  LCD_RS_LOW();

  /* Wait for more than 15 ms after VCC rises to 4.5 V */
  _delay_ms(20);

  /* Initial 8-bit write */
  PORT(LCD_D7_PORT) &= ~_BV(LCD_D7_PIN);
  PORT(LCD_D6_PORT) &= ~_BV(LCD_D6_PIN);
  PORT(LCD_D5_PORT) |= _BV(LCD_D5_PIN);
  PORT(LCD_D4_PORT) |= _BV(LCD_D4_PIN);

  /* Send command */
  LCD_EN_TOGGLE();
  /* Wait for more than 4.1 ms */
  _delay_ms(5);

  /* Repeat last command */
  LCD_EN_TOGGLE();
  /* Wait for more than 100us */
  _delay_us(200);

  /* Repeat last command */
  LCD_EN_TOGGLE();
  /* Wait for more than 100us */
  _delay_us(200);

  /* Configure for 4 bit mode */
  PORT(LCD_D4_PORT) &= ~_BV(LCD_D4_PIN);
  LCD_EN_TOGGLE();
  _delay_ms(5);

  lcd_write(LCD_CMD_CLEAR, MODE_COMMAND);
  lcd_write(LCD_CMD_DISP_CONTROL | LCD_DISPLAY_ON, MODE_COMMAND);
}


void lcd_clear() {
  lcd_write(LCD_CMD_CLEAR, MODE_COMMAND);
}


void lcd_load_char_P(uint8_t index, const lcd_char_t lcd_char) {
  int8_t i;

  lcd_write(LCD_CMD_SET_CGRAM_ADDR | (index << 3), MODE_COMMAND);

  for (i = 0; i < 8; ++i)
    lcd_write(pgm_read_byte(lcd_char + i), MODE_DATA);

  lcd_goto(0, 0);
}


void lcd_load_charset_bars() {
  int8_t  i, l;

  lcd_write(LCD_CMD_SET_CGRAM_ADDR, MODE_COMMAND);

  for (i = 0; i < 8; ++i) {
    for (l = 0; l < 7 - i; ++l)
      lcd_write(0, MODE_DATA);
    for (; l < 8; ++l)
      lcd_write(31, MODE_DATA);
  }

  lcd_goto(0, 0);
}


void lcd_load_charset_polish_diacritics() {
  int8_t  i;

  for  (i = 0; i < 8; ++i)
    lcd_load_char_P(i, LCD_POLISH_DIACRITICS[i]);
}


void lcd_goto(uint8_t x, uint8_t y) {
  lcd_write(LCD_CMD_SET_DDRAM_ADDR | (x + y * 0x40), MODE_COMMAND);
}


void lcd_putc(char ch) {
  lcd_write(ch, MODE_DATA);
}


void lcd_pad(char ch, uint8_t n)
{
  for (uint8_t i = 0; i < n; ++i)
    lcd_putc(ch);
}


void lcd_puts(char* s) {
  while (*s) {
    lcd_write(*s, MODE_DATA);
    ++s;
  }
}


void lcd_puts_P(const char* s) {
  char  ch = pgm_read_byte(s);

  while (ch) {
    ++s;
    lcd_write(ch, MODE_DATA);
    ch = pgm_read_byte(s);
  }
}


void lcd_put_int(int x) {
  char buffer[7];

  itoa(x, buffer, 10);
  lcd_puts(buffer);
}


void lcd_put_long(long x) {
  char buffer[11];

  ltoa(x, buffer, 10);
  lcd_puts(buffer);
}


void lcd_put_int_hex(unsigned int x) {
  signed char i;
  unsigned char d;

  for (i = 12; i >= 0; i -= 4) {
    d = (x >> i) & 0x0f;

    if (d < 10)
      lcd_write('0' + d, MODE_DATA);
    else
      lcd_write('a' + d - 10, MODE_DATA);
  }
}
