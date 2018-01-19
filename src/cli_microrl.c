#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "../lib/hd44780_111/hd44780.h"
#include "../lib/andygock_avr-uart/uart.h"
#include "../lib/helius_microrl/microrl.h"
#include "../lib/matejx_avr_lib/mfrc522.h"
#include "../lib/andy_brown_memdebug/memdebug.h"
#include "hmi_msg.h"
#include "cli_microrl.h"
#include "print_helper.h"

#define NUM_ELEMS(x) (sizeof(x) / sizeof((x)[0]))

void cli_print_help(const char *const *argv);
void cli_example(const char *const *argv);
void cli_print_ver(const char *const *argv);
void cli_print_ascii_tbls(const char *const *argv);
void cli_handle_number(const char *const *argv);
void cli_print_cmd_error(void);
void cli_print_cmd_arg_error(void);
void cli_rfid_read(const char *const *argv);
void cli_rfid_adder(const char *const *argv);
void cli_rfid_viewList(const char *const *argv);
void cli_rfid_delete(const char *const *argv);
void cli_mem_stat(const char *const *argv);

typedef struct cli_cmd {
    PGM_P cmd;
    PGM_P help;
    void (*func_p)();
    const uint8_t func_argc;
} cli_cmd_t;

Card_t *header = NULL;

const char help_cmd[] PROGMEM = "help";
const char help_help[] PROGMEM = "Get help";
const char example_cmd[] PROGMEM = "example";
const char example_help[] PROGMEM = "Prints out all provided 3 arguments Usage: example <argument> <argument> <argument>";
const char ver_cmd[] PROGMEM = "version";
const char ver_help[] PROGMEM = "Print FW version";
const char ascii_cmd[] PROGMEM = "ascii";
const char ascii_help[] PROGMEM = "Print ASCII tables";
const char number_cmd[] PROGMEM = "number";
const char number_help[] PROGMEM = "Print and display matching number Usage: number <decimal number>";

const char rfid_read_cmd[] PROGMEM = "read";
const char rfid_read_help[] PROGMEM = "Read card";
const char rfid_adder_cmd[] PROGMEM = "add";
const char rfid_adder_help[] PROGMEM = "Add cards to list: add <UID> <Name>";
const char rfid_viewList_cmd[] PROGMEM = "list";
const char rfid_viewList_help[] PROGMEM = "Show list of cards";
const char rfid_delete_cmd[] PROGMEM = "delete";
const char rfid_delete_help[] PROGMEM = "Delete card from list: delete <UID>";
const char mem_stat_cmd[] PROGMEM = "mem";
const char mem_stat_help[] PROGMEM =
    "Print memory usage and change compared to previous call";

const cli_cmd_t cli_cmds[] = {
    {help_cmd, help_help, cli_print_help, 0},
    {ver_cmd, ver_help, cli_print_ver, 0},
    {example_cmd, example_help, cli_example, 3},
    {ascii_cmd, ascii_help, cli_print_ascii_tbls, 0},
    {number_cmd, number_help, cli_handle_number, 1},
    {rfid_read_cmd, rfid_read_help, cli_rfid_read, 0},
    {rfid_adder_cmd, rfid_adder_help, cli_rfid_adder, 2},
    {rfid_viewList_cmd, rfid_viewList_help, cli_rfid_viewList, 0},
    {rfid_delete_cmd, rfid_delete_help, cli_rfid_delete, 1},
    {mem_stat_cmd, mem_stat_help, cli_mem_stat, 0}
};


void cli_print_help(const char *const *argv)
{
    (void) argv;
    uart0_puts_p(PSTR("Implemented commands:\r\n"));

    for (uint8_t i = 0; i < NUM_ELEMS(cli_cmds); i++) {
        uart0_puts_p(cli_cmds[i].cmd);
        uart0_puts_p(PSTR(" : "));
        uart0_puts_p(cli_cmds[i].help);
        uart0_puts_p(PSTR("\r\n"));
    }
}

void cli_example(const char *const *argv)
{
    uart0_puts_p(PSTR("Command had following arguments:\r\n"));

    for (uint8_t i = 1; i < 4; i++) {
        uart0_puts(argv[i]);
        uart0_puts_p(PSTR("\r\n"));
    }
}

void cli_print_ver(const char *const *argv)
{
    (void) argv;
    uart0_puts_p(PSTR(VER_FW));
    uart0_puts_p(PSTR(VER_LIBC));
}


void cli_print_ascii_tbls(const char *const *argv)
{
    (void) argv;
    print_ascii_tbl();
    unsigned char ascii[128] = {0};

    for (unsigned char i = 0; i < sizeof(ascii); i++) {
        ascii[i] = i;
    }

    print_for_human(ascii, sizeof(ascii));
}


void cli_handle_number(const char *const *argv)
{
    (void) argv;
    int number = atoi(argv[1]);

    for (size_t i   =   0;  i   <   strlen(argv[1]);    i++)    {
        if  (!isdigit(argv[1][i]))  {
            uart0_puts_p(PSTR("Argument is not a decimal number!\r\n"));
            lcd_clrscr();
            lcd_puts_P(PSTR(wrongNumber));
            return;
        }
    }

    if (number >= 0 && number <= 9) {
        uart0_puts_p(PSTR(ENTER_NUMBER));
        uart0_puts_p((PGM_P)pgm_read_word(&(numbers_table[number])));
        uart0_puts_p(PSTR("\r\n"));
        lcd_clrscr();
        lcd_puts_P((PGM_P)pgm_read_word(&(numbers_table[number])));
    } else {
        uart0_puts_p(PSTR(wrongNumber));
        lcd_clrscr();
    }
}

/*When putting the card near the reader and inserting the "read" command it writes
  the output to the command line*/
  void cli_rfid_read(const char *const *argv) {
  (void) argv;
  Uid uid;
  Uid *uid_ptr = &uid;
  char *uidName;
  char *uidSize;
  byte bufferATQA[10];
  byte bufferSize[10];
  if (PICC_IsNewCardPresent()) {
    uart0_puts_p(PSTR("Card selected.\r\n"));
    PICC_ReadCardSerial(uid_ptr);
    uidName = bin2hex(uid_ptr->uidByte, uid_ptr->size);
    uidSize = bin2hex(&uid.size, sizeof uid_ptr->size);
    uart0_puts_p(PSTR("Card type is: "));
    uart0_puts(PICC_GetTypeName(PICC_GetType(uid_ptr->sak)));
    uart0_puts_p(PSTR("\r\nCard UID: "));
    uart0_puts(uidName);
    uart0_puts_p(PSTR("\r\nUID size: "));
    uart0_puts(uidSize);
    uart0_puts_p(PSTR("\r\n"));
    //Free vabastab kaardi nime ja suuruse, ilma selleta prindiks iga kaardi kohta sama infot
    free(uidName);
    free(uidSize);
    PICC_WakeupA(bufferATQA, bufferSize);
  } else {
    uart0_puts_p(PSTR("Unable	to	select	card.\r\n"));
  }
}

//To add RFID cards to a collection
void cli_rfid_adder(const char *const *argv)
{
    (void) *argv;
    char *uidName;
    char *uidSize;
    char *userName;
    uidName = malloc(strlen(argv[1]) + 1);
    strcpy(uidName, argv[1]);
    //Kasutada uinti kuna aint positiivsed numbrid, jagada 2-ga kuna 2 tähte on 1 bait
    uint8_t uidLength = strlen(uidName)/2;

    //Enne lisamist kontrollida, et kaart ei oleks üle 10 baidi
    if (uidLength > 10) {
        uart0_puts_p(PSTR("Error: UID size is too big!\r\n"));
        free(uidName);
        return;
    }

    //Checks if there is same card in the list
    Card_t *checkExistence = header;

    while (checkExistence != NULL) {
      if (strcmp(checkExistence->UID, uidName) == 0) {
          uart0_puts_p(PSTR("Error: This card is already in the system!\r\n"));
          free(uidName);
          return;
      }
      checkExistence = checkExistence->next;
    }
    //Setup for linked list
    Card_t *newCard = malloc(sizeof(Card_t));

    if (newCard == NULL) {
        uart0_puts_p(PSTR("Error: Out of memory!\r\n"));
        free(newCard);
        free(uidName);
        return;
    }

    userName = malloc(strlen(argv[2]) + 1);
    if(userName == NULL) {
      uart0_puts_p(PSTR("Error: Failed to allocate memory"));
      return;
    }
    strcpy(userName, argv[2]);
    uidSize = malloc(uidLength + 1);
    if(uidSize == NULL) {
      uart0_puts_p(PSTR("Error: Failed to allocate memory"));
      return;
    }
    uidSize = itoa(uidLength, uidSize, 10);
    newCard->UID = uidName;
    newCard->size = uidSize;
    newCard->name = userName;
    newCard->next = NULL;

    if (header == NULL) {
        header = newCard;
        uart0_puts_p(PSTR("Successfully added card! (header)\r\n"));
    } else {
        Card_t *current = header;

        while (current->next != NULL) {
            current = current->next;
        }

        current->next = newCard;
        uart0_puts_p(PSTR("Successfully added card! (next)\r\n"));
    }

    uart0_puts_p(PSTR("UID: "));
    uart0_puts(newCard->UID);
    uart0_puts_p(PSTR("\r\nUID size: "));
    uart0_puts(newCard->size);
    uart0_puts_p(PSTR("\r\nName: "));
    uart0_puts(newCard->name);
    uart0_puts_p(PSTR("\r\n"));
}

void cli_rfid_viewList(const char *const *argv)
{
    (void) argv;
    Card_t *current = header;
    char* numberString = NULL;
    int number = 1;

    if (header == NULL) {
        uart0_puts_p(PSTR("Error: List is empty\r\n"));
    } else {
        while (current != NULL) {
            uart0_puts_p(PSTR("Number: "));
            //https://fresh2refresh.com/c-programming/c-type-casting/c-itoa-function/
            uart0_puts(itoa(number, numberString, 10));
            uart0_puts_p(PSTR("\r\nUID: "));
            uart0_puts(current->UID);
            uart0_puts_p(PSTR("\r\nUID size: "));
            uart0_puts(current->size);
            uart0_puts_p(PSTR("\r\nName: "));
            uart0_puts(current->name);
            uart0_puts_p(PSTR("\r\n"));
            number++;
            current = current->next;
        }
    }
}

void cli_rfid_delete(const char *const *argv)
{
    (void) argv;

    if (header != NULL) {
      Card_t *current = header;
      Card_t *previous = header;
      Card_t *current_card = NULL;
      char *uidName = malloc(strlen(argv[1]) + 1);
      strcpy(uidName, argv[1]);

      if (strcmp(current->UID, uidName) == 0) {
          current_card = header->next;
          free(header);
          free(uidName);
          header = current_card;
          uart0_puts_p(PSTR("Card was removed successfully!\r\n"));
          return;
      }

      while (current != NULL) {
          if (strcmp(current->UID, uidName) == 0) {
              Card_t *current_card = current;
              previous->next = current->next;
              free(current_card->name);
              free(uidName);
              uart0_puts_p(PSTR("Card was removed successfully!\r\n"));
              return;
          }

          previous = current;
          current = current->next;
      }

      free(uidName);
      uart0_puts_p(PSTR("Error: This card doesn't exist!\r\n"));
    } else {
        uart0_puts_p(PSTR("Error: List is empty\r\n"));
    }
}

void cli_mem_stat(const char *const *argv)
{
    (void) argv;
    char print_buf[256] = {0x00};
    extern int __heap_start, *__brkval;
    int v;
    int space;
    static int prev_space;
    space = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    uart0_puts_p(PSTR("Heap statistics\r\n"));
    sprintf_P(print_buf, PSTR("Used: %u\r\nFree: %u\r\n"), getMemoryUsed(),
              getFreeMemory());
    uart0_puts(print_buf);
    uart0_puts_p(PSTR("\r\nSpace between stack and heap:\r\n"));
    sprintf_P(print_buf, PSTR("Current  %d\r\nPrevious %d\r\nChange   %d\r\n"),
              space, prev_space, space - prev_space);
    uart0_puts(print_buf);
    uart0_puts_p(PSTR("\r\nFreelist\r\n"));
    sprintf_P(print_buf, PSTR("Freelist size:             %u\r\n"),
              getFreeListSize());
    uart0_puts(print_buf);
    sprintf_P(print_buf, PSTR("Blocks in freelist:        %u\r\n"),
              getNumberOfBlocksInFreeList());
    uart0_puts(print_buf);
    sprintf_P(print_buf, PSTR("Largest block in freelist: %u\r\n"),
              getLargestBlockInFreeList());
    uart0_puts(print_buf);
    sprintf_P(print_buf, PSTR("Largest freelist block:    %u\r\n"),
              getLargestAvailableMemoryBlock());
    uart0_puts(print_buf);
    sprintf_P(print_buf, PSTR("Largest allocable block:   %u\r\n"),
              getLargestNonFreeListBlock());
    uart0_puts(print_buf);
    prev_space = space;
}

void cli_print_cmd_error(void)
{
    uart0_puts_p(PSTR("Command not implemented.\r\n\tUse <help> to get help.\r\n"));
}


void cli_print_cmd_arg_error(void)
{
    uart0_puts_p(
        PSTR("To few or too many arguments for this command\r\n\tUse <help>\r\n"));
}


int cli_execute(int argc, const char *const *argv)
{
    // Move cursor to new line. Then user can see what was entered.
    uart0_puts_p(PSTR("\r\n"));

    for (uint8_t i = 0; i < NUM_ELEMS(cli_cmds); i++) {
        if (!strcmp_P(argv[0], cli_cmds[i].cmd)) {
            // Test do we have correct arguments to run command
            // Function arguments count shall be defined in struct
            if ((argc - 1) != cli_cmds[i].func_argc) {
                cli_print_cmd_arg_error();
                return 0;
            }

            // Hand argv over to function via function pointer,
            // cross fingers and hope that funcion handles it properly
            cli_cmds[i].func_p (argv);
            return 0;
        }
    }

    cli_print_cmd_error();
    return 0;
}
