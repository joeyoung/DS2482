//i2cTemps - example for DS2482 library to use DS2482 operations:
//           - search the one-wire bus
//           - retrieve temperature values from DS18B20
//           - display temperatures
//
// started: Jan 21, 2022  G. D. (Joe) Young <jyoung@islandnet.com>
//
// revised: Jan 22/22 - add read scratchpad, calculate temperatures
//          Feb  1/22 - fix temperature calculation using printfix, add crc check 
//

#include <Wire.h>
#include "DS2482.h"   //package of AN3684 subr
#include "oneWire.h"  //DS18B20 definitions
#define PRSTRSIZE 5   //buffer size for printfix
#include "printfix.h" //package for printing fixed-width fields

#define I2Cadr 0x18   //base address of DS2482
#define MAXID 6       //maximum number of one-wire serial numbers

DS2482 i2ow( I2Cadr ); //create bridge object on I2C address 0x18

byte sna[MAXID][8];   //storage for discovered one-wire devices
int temps[MAXID];     //storage for returned temp integers

byte tmpMem[25];      //command string buffer
bool found = false;

void getTemp( int *tmps, byte numTemp );

void setup() {
  Serial.begin( 9600 );
  while( !Serial ) { /* wait */ }
  Wire.begin( );
  i2ow.begin( );
  Serial.println( "owI2C - DS2482 bridge" );
  if( i2ow.DS2482_detect(  ) ) {
    Serial.println( "bridge chip detected and reset" );
  } else {
    Serial.print( "error accessing bridge chip at I2Cadr " );
    Serial.println( I2Cadr, HEX );
  }

  //set operating configuration
  if( i2ow.DS2482_write_config( bit(APU) ) ) {
    Serial.println( "configuration set" );
  } else {
    Serial.println( "failed to set config" );
  }

  //reset 1wire bus
  if( i2ow.OWReset( ) ) {
    Serial.println( "bus reset" );
  } else {
    Serial.println( "failed bus reset" );
  }
  if( i2ow.short_detected ) Serial.println( "  short detected" );
  Serial.println( "search ROM" );
  //search ROM with indefinite number of devices, up to MAXID
  byte jx = 0;
  if( i2ow.OWFirst( ) ) {
    found = true;
    for( byte ix=0; ix<8; ix++ ) sna[jx][ix] = i2ow.ROM_NO[ix];    //copy found to save area
    jx++;
    while( found && jx<MAXID ) {
      if( i2ow.OWNext( ) ) {
        for( byte ix=0; ix<8; ix++ ) sna[jx][ix] = i2ow.ROM_NO[ix];
        jx++;
      } else {
        found = false;
      }
    } // while finding devices (fewer than MAXID)
  } // if first
  if( jx == 0 ) {
    Serial.println( "no one-wire devices found" );
  } else {
    for( byte ix=0; ix<jx; ix++ ) {
      for( byte kx=0; kx<8; kx++ ) {
        Serial.print( ' ' );
        Serial.print( sna[ix][kx], HEX );    
      }
      Serial.println( "" );
    }
  }

  if( jx != 0 ) {
    getTemp( jx );
    //calculate decimal values - temps are 16X actual value
    Serial.println( "temperatures for each sensor:" );
    for( byte ix=0; ix<jx; ix++ ) {
      int itemp = temps[ix];
      if( itemp < 0 ) {
        itemp = -itemp;
        Serial.print( '-' );
      }
      Serial.print( itemp>>4 );
      Serial.print( '.' );
      printFixUint( ( (itemp & 0x0f) * 1000 )/16, 3, prStr );
      Serial.print( prStr );
      Serial.print( ' ' );
    }
    Serial.println( "" );
  } //if devices found
} //setup( )


void loop() {
  // put your main code here, to run repeatedly:

}

//Fetch temperature from discovered devices

void getTemp( byte numTemp ) {
  int kkx = 0;

  //meas with power
  Serial.println( "measure with power" );
  i2ow.OWReset( );
  i2ow.OWWriteByte( CSKRM );     //skip ROM
  i2ow.OWWriteBytePower( CMD_WWBP ); //strong pullup, start conversion
  delay(1000);
  i2ow.OWLevel( MODE_STANDARD );    //restore normal pullup

  for( byte kx=0; kx<numTemp ; kx++ ) {
    kkx = kx;
    i2ow.OWReset( );
    tmpMem[0] = CMTCH;
    for( byte ix=0; ix<8; ix++ ) tmpMem[ix+1] = sna[kx][ix];
    tmpMem[9] = CRSPD;
    for( byte ix=0; ix<9; ix++ ) tmpMem[ix+10] = 0xFF;
    i2ow.OWBlock( tmpMem, 19 );
    uint8_t crc8 = 0;
    for( byte ix=0; ix<9; ix++ ) {
      crc8 = i2ow.calc_crc8( tmpMem[ix+10] );       //accumulate crc for scratchpad bytes
      Serial.print( ' ' );
      Serial.print( tmpMem[ix+10], HEX );
    }
    Serial.print( ' ' );
    Serial.println( crc8 );                      //report crc (should be zero)
    temps[kkx] = ((int)(tmpMem[11])<<8) + (int)tmpMem[10];   //save raw temp value
  } //loop to report each found device

}
