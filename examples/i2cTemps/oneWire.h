// oneWire.h - definition of one-wire device commands
//
// started: Jan 19, 2022  G. D. (Joe) Young <jyoung@islandnet.com>
//
// revised:
//
//

#ifndef ONE_WIRE_H
#define ONE_WIRE_H

// DS18B20 temp sensor definitions
// ROM cmds
#define CSRCH 0xF0      //Search ROM command
#define CREAD 0x33      //Read ROM command
#define CMTCH 0x55      //Match ROM command
#define CSKRM 0xCC      //Skip ROM command
#define CASCH 0xEC      //Alarm Search command
//device function cmds
#define CCVRT 0x44      //Convert temperature
#define CWSPD 0x4E      //Write scracthpad
#define CRSPD 0xBE      //Read scratchpad
#define CCYPD 0x48      //Copy scratchpad
#define CRCEE 0xB8      //Recall EEPROM
#define CRPWR 0XB4      //Read power supply



#endif
