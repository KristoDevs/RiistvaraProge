#include <string.h>
#include <time.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "hmi_msg.h"
#include "print_helper.h"
#include "cli_microrl.h"
#include "../lib/hd44780_111/hd44780.h"
#include "../lib/andygock_avr-uart/uart.h"
#include "../lib/helius_microrl/microrl.h"
#include "../lib/matejx_avr_lib/mfrc522.h"
#include "../lib/andy_brown_memdebug/memdebug.h"

#define BLINK_DELAY_MS 1000

#define UART_BAUD 9600
#define UART_STATUS_MASK 0x00FF

//  Create  microrl object  and pointer on  it
volatile uint32_t system_time;
static microrl_t rl;
static microrl_t *prl = &rl;

typedef enum {
    door_opening,
    door_open,
    door_closing,
    door_closed
} door_status_t;

door_status_t door_state = door_closed;

static	inline	void	init_rfid_reader(void)
{
		/*	Init	RFID-RC522	*/
    MFRC522_init();
    PCD_Init();
}

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

static inline uint32_t current_time(void)
{
    uint32_t cur_time;
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        cur_time = system_time;
    }
    return cur_time;
}

static inline void showDoorStatus() {
    switch (door_state) {
    case door_opening:
        DDRA ^= _BV(DDA2);
        PORTA |= _BV(PORTA0);
        door_state = door_open;
        break;

    case door_open:
        break;

    case door_closing:
        door_state = door_closed;
        PORTA &= ~_BV(PORTA0);
        DDRA ^= _BV(DDA2);
        break;

    case door_closed:
        break;
    }
}

static inline void rfid_scanner(void) {
    Uid uid;
    Uid *uid_ptr = &uid;
    char *uidName;
    char *usedName;
    bool currentStatus;
    bool usedError;
    bool openDoor_status;
    bool messageStatus;
    uint32_t time_now = current_time();
    static uint32_t firstRead;
    uint32_t openDoor_start;
    uint32_t messageStart;
    byte bufferATQA[10];
    byte bufferSize[10];

    if ((firstRead + 1) < time_now) {
        if (PICC_IsNewCardPresent()) {
            firstRead = time_now;
            PICC_ReadCardSerial(&uid);
            uidName = bin2hex(uid_ptr->uidByte, uid_ptr->size);
            Card_t *checkCard = header;

            while (checkCard != NULL) {
                if (strcmp(checkCard->UID, uidName) == 0) {
                    currentStatus = true;
                    break;
                }

                currentStatus = false;
                checkCard = checkCard->next;
            }

            if (currentStatus) {
                if (checkCard->name != usedName || usedName == NULL) {
                    lcd_clr(LCD_ROW_2_START, LCD_VISIBLE_COLS);
                    lcd_goto(LCD_ROW_2_START);
                    lcd_puts(checkCard->name);
                    usedName = checkCard->name;
                    usedError = false;
                }

                if (door_state != door_open) {
                    door_state = door_opening;
                    openDoor_status = true;
                }

                openDoor_start = time_now;
            } else {
                if (!usedError) {
                    lcd_clr(LCD_ROW_2_START, LCD_VISIBLE_COLS);
                    lcd_goto(LCD_ROW_2_START);
                    lcd_puts_P(PSTR("Error: Unknown user"));
                    usedError = true;
                    usedName = NULL;
                }

                if (door_state != door_closed) {
                    door_state = door_closing;
                    openDoor_status = false;
                }
            }

            free(uidName);
            messageStatus = true;
            messageStart = time_now;
            PICC_WakeupA(bufferATQA, bufferSize);
        }
    }

    if ((messageStart + 5) < time_now && messageStatus) {
        lcd_clr(LCD_ROW_2_START, LCD_VISIBLE_COLS);
        lcd_goto(LCD_ROW_2_START);
        lcd_puts_P(PSTR("Door is Closed"));
        usedError = false;
        usedName = NULL;
        messageStatus = false;
    }

    if ((openDoor_start + 3) < time_now && openDoor_status) {
        door_state = door_closing;
        openDoor_status = false;
    }

    showDoorStatus(door_state);
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
    init_rfid_reader();
    show_errcon();
    uart0_puts_p(PSTR(myName));
    init_lcd();

    while (1) {
        heartbeat();
        rfid_scanner();
        //  CLI commands    are handled in  cli_execute()
        microrl_insert_char(prl, (uart0_getc() & UART_STATUS_MASK));
    }
}

ISR(TIMER1_COMPA_vect)
{
    system_tick();
    system_time++;
}
