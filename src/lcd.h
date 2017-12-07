#ifndef _LCD_H_
#define _LCD_H_

/**
 * @file lcd.h
 *
 * Obsługa wyświetlacza LCD
 */

#include <stdint.h>


#define LCD_D7_PORT D
#define LCD_D7_PIN 4
#define LCD_D6_PORT D
#define LCD_D6_PIN 7
#define LCD_D5_PORT C
#define LCD_D5_PIN 1
#define LCD_D4_PORT C
#define LCD_D4_PIN 0
#define LCD_EN_PORT B
#define LCD_EN_PIN 5
#define LCD_RS_PORT B
#define LCD_RS_PIN 4

#define LCD_CMD_CLEAR           0x01
#define LCD_CMD_HOME            0x02
#define LCD_CMD_ENTRY_MODE_SET  0x04
#define LCD_CMD_DISP_CONTROL    0x08
#define LCD_CMD_CURSOR_SHIFT    0x10
#define LCD_CMD_SET_CGRAM_ADDR  0x40
#define LCD_CMD_SET_DDRAM_ADDR  0x80

#define LCD_DISPLAY_ON   0x04
#define LCD_CURSOR_ON    0x02
#define LCD_BLINKING_ON  0x01


/** Kod litery ą */
#define _a "\x01"

/** Kod litery ć */
#define _c "\x02"

/** Kod litery ę */
#define _e "\x03"

/** Kod litery ł */
#define _l "\x04"

/** Kod litery ó */
#define _o "\x05"

/** Kod litery ś */
#define _s "\x06"

/** Kod litery ż */
#define _z "\x07"

/** Kod litery ń */
#define _n "\xee"


typedef uint8_t lcd_char_t[8];
typedef lcd_char_t lcd_charset_t[8];


  /**
   * Ilicjalizuje wyświetlacz
   */
  void lcd_init();

  /**
   * Czyści wyświetlacz
   */
  void lcd_clear();

  /**
   * Ustawia kontrast wyświetlacza
   * @param v wartość kontrastu (0 -- 255)
   */
  void lcd_set_contrast(uint8_t v);

  /**
   * Przesuwa kursor wyświetlacza w punkt (x, y)
   * @param x współrzędna x
   * @param y współrzędna y
   */
  void lcd_goto(uint8_t x, uint8_t y);

  /**
   * Wyświetla pojedynczy znak w aktualnej pozycji kursora
   * @param ch znak do wyświetlenia
   */
  void lcd_putc(char ch);
  void lcd_pad(char ch, uint8_t n);

  /**
   * Wyświetla łańcuch znaków zakończony zerem
   * @param s łańcuch znaków
   */
  void lcd_puts(char* s);

  /**
   * Wyświetla łańcuch znaków z pamięci programu
   * @param s wskaźnik na łańcuch znaków w pamięci programu
   */
  void lcd_puts_P(const char* s);

  /**
   * Ładuje znak z pamięci programu do pamięci generatora znaków wyświetlacza
   * @param index kod znaku (0 -- 7)
   * @param lcd_char opis znaku (tablica)
   */
  void lcd_load_char_P(uint8_t index, const lcd_char_t lcd_char);

  /**
   * Ładuje do pamięci generatora znaków wyświetlacza znaki prostokątów
   */
  void lcd_load_charset_bars();

  /**
   * Ładuje do pamięci generatora znaków wyświetlacza polskie znaki diakrytyczne
   */
  void lcd_load_charset_polish_diacritics();

  /**
   * Wyświetla liczbę całkowitą dziesiętnie
   * @param x liczba do wyświetlenia
   */
  void lcd_put_int(int x);

  /**
   * Wyświetla liczbę całkowitą dziesiętnie
   * @param x liczba do wyświetlenia
   */
  void lcd_put_long(long x);

  /**
   * Wyświetla liczbę całkowitą szesnastkowo
   * @param x liczba do wyświetlenia
   */
  void lcd_put_int_hex(unsigned int x);

#endif
