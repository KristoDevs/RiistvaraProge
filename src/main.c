#include <stdio.h>
#define __ASSERT_USE_STDERR
#include <assert.h>
#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"
#include "../lib/hd44780_111/hd44780.h"

#define BLINK_DELAY_MS 1000

static inline void init_leds(void)
{
    DDRA |= _BV(DDA4);
    DDRA |= _BV(DDA2);
    DDRA |= _BV(DDA0);
    DDRB |= _BV(DDB7);
    PORTB &= ~_BV(PORTB7);
}

/* Init error console as stderr in UART1 and print user code info */
static inline void init_errcon(void)
{
    simple_uart1_init();
    stderr = &simple_uart1_out;
    fprintf(stderr, "Version: %s built on: %s %s\n",
            FW_VERSION, __DATE__, __TIME__);
    fprintf(stderr, "avr-libc version: %s avr-gcc version: %s\n",
            __AVR_LIBC_VERSION_STRING__, __VERSION__);
}

static inline void blink_leds(void)
{
    while (1) {
        PORTA |= _BV(PORTA0);
        _delay_ms(BLINK_DELAY_MS);
        /*Breakpoint*/
        PORTA &= ~_BV(PORTA0);
        _delay_ms(BLINK_DELAY_MS);
        /*Breakpoint*/
        PORTA |= _BV(PORTA2);
        _delay_ms(BLINK_DELAY_MS);
        /*Breakpoint*/
        PORTA &= ~_BV(PORTA2);
        _delay_ms(BLINK_DELAY_MS);
        /*Breakpoint*/
        PORTA |= _BV(PORTA4);
        _delay_ms(BLINK_DELAY_MS);
        /*Breakpoint*/
        PORTA &= ~_BV(PORTA4);
        _delay_ms(BLINK_DELAY_MS);
        break;
    }
}

static inline void local_lcd_test(void) {
  uint8_t pos = LCD_COLS_MAX;
  lcd_home();
  do {
    lcd_putc(0xFF);
    _delay_ms(1000);
  } while (pos--);
}

void main(void)
{
  init_leds();
  init_errcon();
  lcd_init();
  lcd_home();
  lcd_puts("    Hire me");
  lcd_goto(LCD_ROW_2_START);
  lcd_puts("      pls");
  local_lcd_test();

  while(1) {
    blink_leds();
  }
}
