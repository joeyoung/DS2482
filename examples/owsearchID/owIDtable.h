// owIDtable.h - one-wire device ID from AN155 - "Table 1. Family Code Reference"
//
// Started: Feb 6, 2022  G. D. (Joe) Young <jyoung@islandnet.com>
//
// Revised:
//
#include <avr/pgmspace.h>
#define IDLINE_SZ 80
#define NRID 10      //number of items in table

const char PROGMEM owid0[ ] = { "01 (DS1990A), (DS1990R), DS2401, DS2411 1-Wire net address (registration number) only" };
const char PROGMEM owid1[ ] = { "12 DS2406, DS24071 1Kb EPROM memory, 2-channel addressable switch" };
const char PROGMEM owid2[ ] = { "14 (DS1971), DS2430A1 256-bit EEPROM memory and 64-bit OTP register" };
const char PROGMEM owid3[ ] = { "27 DS2417 RTC with interrupt" };
const char PROGMEM owid4[ ] = { "20 DS2450 4-channel A/D converter (ADC)" };
const char PROGMEM owid5[ ] = { "28 DS18B20 temperature sensor" };
const char PROGMEM owid6[ ] = { "29 DS2408 8-channel addressable switch" };
const char PROGMEM owid7[ ] = { "2C DS28901 1-channel digital potentiometer" };
const char PROGMEM owid8[ ] = { "2D (DS1972), DS2431 1024-bit, 1-Wire EEPROM" };
const char PROGMEM owid9[ ] = { "37 (DS1977) Password-protected 32KB (bytes) EEPROM" };

const char* const idstrings[NRID] PROGMEM = { owid0, owid1, owid2, owid3, owid4, owid5, owid6,
                                              owid7, owid8, owid9 };

const char idlist[NRID] = { 0x01, 0x12, 0x14, 0x27, 0x20, 0x28, 0x29, 0x2c, 0x2d, 0x37 };

char idline[IDLINE_SZ];   //rwm buffer for id descriptions
uint8_t idindex;          //index into array of strings

void displayID( char idindex ) {
  strcpy_P( idline, (char*)pgm_read_word( &(idstrings[idindex]) ) );  //copy description to rwm
  Serial.println( idline );
}
