//owsearch - example for DS2482 library to check DS2482 operation,
//           then search the one-wire bus
//
// started: Jan 21, 2022  G. D. (Joe) Young <jyoung@islandnet.com>
//
// revised:
//

#include <Wire.h>
#include "DS2482.h"   //package of AN3684 subr
#include "oneWire.h"  //DS18B20 definitions

#define I2Cadr 0x18   //base address of DS2482
#define MAXID 6       //maximum number of one-wire serial numbers

DS2482 i2ow( I2Cadr ); //create bridge object on I2C address 0x18

byte sna[MAXID][8];   //storage for discovered one-wire devices

byte tmpMem[25];      //command string buffer
bool found = false;

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

} //setup( )

void loop( ) {

}


