#include <util/delay.h>
#include <time.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "hmi_msg.h"
#include "print_helper.h"
#include "cli_microrl.h"
#include "../lib/hd44780_111/hd44780.h"
#include "../lib/andygock_avr-uart/uart.h"
#include "../lib/helius_microrl/microrl.h"

#define BLINK_DELAY_MS 1000

#define UART_BAUD 9600
#define UART_STATUS_MASK 0x00FF

//  Create  microrl object  and pointer on  it
static microrl_t    rl;
static microrl_t    *prl    =   &rl;

static inline void init_micro(void)
{
    //    Call    init    with    ptr to  microrl instance    and print   callback
    microrl_init(prl, uart0_puts);
    //    Set callback    for execute
    microrl_set_execute_callback(prl, cli_execute);
}

static inline void init_leds(void)
{
    DDRA |= _BV(DDA4);
    DDRA |= _BV(DDA2);
    DDRA |= _BV(DDA0);
    DDRB |= _BV(DDB7);
    PORTB &= ~_BV(PORTB7);
}

static inline void init_sys_timer(void)
{
    //    counter_1 = 0; // Set counter to random number 0x19D5F539 in HEX. Set it to 0 if you want
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1B |= _BV(WGM12); // Turn on CTC (Clear Timer on Compare)
    TCCR1B |= _BV(CS12); // fCPU/256
    OCR1A = 62549; // Note that it is actually two registers OCR5AH and OCR5AL
    TIMSK1 |= _BV(OCIE1A); // Output Compare A Match Interrupt Enable
}


static inline void init_lcd(void)
{
    lcd_puts_P(PSTR(printName));
}

static inline void heartbeat(void)
{
    static time_t prev_time;
    char ascii_buf[11] = {0x00};
    time_t now = time(NULL);

    if (prev_time != now) {
        //Print uptime to uart1
        ltoa(now, ascii_buf, 10);
        uart1_puts_p(PSTR("Uptime: "));
        uart1_puts(ascii_buf);
        uart1_puts_p(PSTR(" s.\r\n"));
        //Toggle LED
        PORTA ^= _BV(PORTA2);
        prev_time = now;
    }
}

static inline void show_errcon(void)
{
    uart1_puts_p(PSTR(VER_FW));
    uart1_puts_p(PSTR(VER_LIBC));
}

void main(void)
{
    init_leds();
    DDRD |= _BV(DDD3);
    uart0_init(UART_BAUD_SELECT(UART_BAUD, F_CPU));
    uart1_init(UART_BAUD_SELECT(UART_BAUD, F_CPU));
    init_sys_timer();
    sei();
    lcd_init();
    lcd_clrscr();
    init_micro();
    show_errcon();
    uart0_puts_p(PSTR(myName));
    init_lcd();

    while (1) {
        heartbeat();
        //  CLI commands    are handled in  cli_execute()
        microrl_insert_char(prl,    (uart0_getc()   &   UART_STATUS_MASK));
    }
}

ISR(TIMER1_COMPA_vect)
{
    system_tick();
}
