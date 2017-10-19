#include <avr/io.h>
#include <util/delay.h>

#define BLINK_DELAY_MS1 1000
#define BLINK_DELAY_MS2 300
#define BLINK_DELAY_MS3 4000

void main (void)
{
    DDRA |= _BV(DDA4);
    DDRA |= _BV(DDA2);
    DDRA |= _BV(DDA0);
    DDRB |= _BV(DDB7);
    PORTB &= ~_BV(PORTB7);

    while (1) {
        PORTA |= _BV(PORTA0);
        _delay_ms(BLINK_DELAY_MS1);
        /*Breakpoint*/
        PORTA &= ~_BV(PORTA0);
        _delay_ms(BLINK_DELAY_MS1);
        PORTA |= _BV(PORTA2);
        _delay_ms(BLINK_DELAY_MS2);
        /*Breakpoint*/
        PORTA &= ~_BV(PORTA2);
        _delay_ms(BLINK_DELAY_MS2);
        PORTA |= _BV(PORTA4);
        _delay_ms(BLINK_DELAY_MS3);
        /*Breakpoint*/
        PORTA &= ~_BV(PORTA4);
        _delay_ms(BLINK_DELAY_MS3);
    }
}

