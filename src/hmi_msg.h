#include <avr/pgmspace.h>
#ifndef HMI_MSG
#define HMI_MSG

#define VER_FW "Version: "FW_VERSION" build on: "__DATE__" "__TIME__"\r\n"
#define VER_LIBC "avr-libc version: "__AVR_LIBC_VERSION_STRING__" avr-gcc version "__VERSION__"\r\n"
#define ENTER_NUMBER "Enter number > "
#define INSERTED_NUMBER "You entered number > "
#define myName "Kristo Leesmann\r\n"
#define printName "Kristo Leesmann"
#define wrongNumber "Enter a number between 0 and 9\r\n"

extern PGM_P const numbers_table[];

#endif
