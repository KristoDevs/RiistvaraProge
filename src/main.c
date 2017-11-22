#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "uart.h"
#include "hmi_msg.h"
#include "print_helper.h"
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

static inline void init_stdin(void)
{
  simple_uart0_init();
  stdin = stdout = &simple_uart0_io;
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

static inline void init_lcd(void)
{
  lcd_init();
  lcd_home();
  lcd_puts(myName);
}

void main(void)
{
  DDRD |= _BV(DDD3);
  init_leds();
  init_errcon();
  init_stdin();
  fprintf(stdout, "%s\n", myName);
  print_ascii_tbl(stdout);
  unsigned char ascii[128] = {0};
  for (unsigned char i = 0; i < sizeof(ascii); i++)
  {
    ascii[i] = i;
  }

  print_for_human(stdout, ascii, sizeof(ascii));
  init_lcd();

  while(1) {
    int number;
    fprintf(stdout, "Enter number > " );
    fscanf(stdin, "%d", &number);
    fprintf(stdout, "%d\n", number);

    blink_leds();
  }
}
