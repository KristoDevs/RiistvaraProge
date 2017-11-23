#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <avr/pgmspace.h>
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
    fprintf_P(stderr, PSTR(VER_FW),
              PSTR(FW_VERSION), PSTR(__DATE__), PSTR(__TIME__));
    fprintf_P(stderr, PSTR(VER_LIBC),
              PSTR(__AVR_LIBC_VERSION_STRING__), PSTR(__VERSION__));
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
    lcd_puts_P(PSTR(myName));
}

void main(void)
{
    DDRD |= _BV(DDD3);
    init_leds();
    init_errcon();
    init_stdin();
    fprintf_P(stdout, PSTR(myName "\n"));
    print_ascii_tbl(stdout);
    unsigned char ascii[128] = {0};

    for (unsigned char i = 0; i < sizeof(ascii); i++) {
        ascii[i] = i;
    }

    print_for_human(stdout, ascii, sizeof(ascii));
    init_lcd();

    while (1) {
        int number;
        fprintf_P(stdout, PSTR(ENTER_NUMBER));
        fscanf(stdin, "%d", &number);
        fprintf(stdout, "%d\n", number);

        if (number >= 0 && number <= 9) {
            fprintf_P(stdout, PSTR(INSERTED_NUMBER),
                      (PGM_P)pgm_read_word(&(numbers_table[number])));
        } else {
            fprintf_P(stdout, PSTR(wrongNumber));
        }

        blink_leds();
    }
}
