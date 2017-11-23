#include <avr/pgmspace.h>
#ifndef HMI_MSG
#define HMI_MSG

#define VER_FW "Version: %S built on %S %S\n"
#define VER_LIBC "avr-libc version: %S avr-gcc version: %S\n"
#define ENTER_NUMBER "Enter number > "
#define INSERTED_NUMBER "You entered number > %S\n"
#define myName "Kristo Leesmann"
#define wrongNumber "Enter a number between 0 and 9\n"

const char n0[] PROGMEM = "Zero";
const char n1[] PROGMEM = "One";
const char n2[] PROGMEM = "Two";
const char n3[] PROGMEM = "Three";
const char n4[] PROGMEM = "Four";
const char n5[] PROGMEM = "Five";
const char n6[] PROGMEM = "Six";
const char n7[] PROGMEM = "Seven";
const char n8[] PROGMEM = "Eight";
const char n9[] PROGMEM = "Nine";

PGM_P const numbers_table[] PROGMEM = {
  n0,
  n1,
  n2,
  n3,
  n4,
  n5,
  n6,
  n7,
  n8,
  n9
};

#endif
