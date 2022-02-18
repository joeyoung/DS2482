//DS2482.cpp library class for Dallas/Maxim DS2482 I2C-to-onewire bridge IC
//
// started: Jan 21, 2022  G. D. (Joe) Young <jyoung@islandnet.com>
//
// revised: Feb 15/22 - subroutines for I2C i/o
//
//

#include "DS2482.h"
#include <Wire.h>

DS2482::DS2482( uint8_t _i2cAdr ) {
	I2Cadr = (int)_i2cAdr;
}//constructor

void DS2482::begin( ) {		//empty placeholder - may use if I2C is shared; setup restarting

} //begin

//these 4 functions collect all I2C i/o

uint8_t DS2482::owcmd( uint8_t cmd ){
	Wire.beginTransmission( I2Cadr );
	Wire.write( cmd );
	Wire.endTransmission( );
	Wire.requestFrom( (int)I2Cadr, (int)1 );
	return Wire.read( );
} // owcmd( cmd )


uint8_t DS2482::owcmd( uint8_t cmd, uint8_t dat ) {
	Wire.beginTransmission( I2Cadr );
	Wire.write( cmd );
	Wire.write( dat );
	Wire.endTransmission( );
	Wire.requestFrom( (int)I2Cadr, (int)1 );
	return Wire.read( );
} // owcmd( cmd, arg )

uint8_t DS2482::owcmdw( uint8_t cmd ) {
	uint8_t status = owcmd( cmd );
	return owwait( status );
} // owcmdw( cmd )

uint8_t DS2482::owcmdw( uint8_t cmd, uint8_t dat ) {
	uint8_t status = owcmd( cmd, dat );
	return owwait( status );
}


uint8_t DS2482::owwait( uint8_t status ) {
	int8_t poll_count = 0;
	do {
		Wire.requestFrom( I2Cadr, 1 );
//		status = Wire.read( ) & 1<<(ST_1WB);
    status = Wire.read( );                //keep all bits for further tests
	} while( (status & 1<<(ST_1WB)) && (poll_count++ < POLL_LIMIT) );
	#ifdef DEBUG
		Serial.print( " * status " );
		Serial.println( status, HEX );
		Serial.print( " * poll_count " );
		Serial.println( poll_count, DEC );
	#endif
	// check for failure due to poll limit reached
	if (poll_count >= POLL_LIMIT)
	{
		DS2482_reset();
		return 0;
	}
	return status;
} // owwait( )


//--------------------------------------------------------------------------
// DS2428 Detect routine that sets the I2C address and then performs a
// device reset followed by writing the configuration byte to default values:
//   1-Wire speed (c1WS) = standard (0)
//   Strong pullup (cSPU) = off (0)
//   Presence pulse masking (cPPM) = off (0)
//   Active pullup (cAPU) = on (CONFIG_APU = 0x01)
//
// Returns: true if device was detected and written
//          false device not detected or failure to write configuration byte
//
bool DS2482::DS2482_detect( )
{
   // reset the DS2482 ON constructed address
   if (!DS2482_reset())
      return false;

   // default configuration
   c1WS = 0;
   cSPU = 0;
   cPPM = 0;
   cAPU = CONFIG_APU;


   // write the default configuration setup
   if (!DS2482_write_config(c1WS | cSPU | cPPM | cAPU))
      return false;

   return true;
}

//--------------------------------------------------------------------------
// Perform a device reset on the DS2482
//
// Returns: true if device was reset
//          false device not detected or failure to perform reset
//
bool DS2482::DS2482_reset()
{
   uint8_t status;
#if 0
  Wire.beginTransmission( I2Cadr );
  Wire.write( CMD_DRST );
  Wire.endTransmission( );
  Wire.requestFrom( (int)I2Cadr, (int)1 );
  status = Wire.read( );
#endif
	status = owcmd( CMD_DRST );
	return ((status & 0xF7) == 0x10);
} //DS2482_reset( )

//--------------------------------------------------------------------------
// Write the configuration register in the DS2482. The configuration
// options are provided in the lower nibble of the provided config byte.
// The uppper nibble in bitwise inverted when written to the DS2482.
//
// Returns:  true: config written and response correct
//           false: response incorrect
//
bool DS2482::DS2482_write_config(uint8_t config)
{
	uint8_t read_config;
#if 0
  Wire.beginTransmission( I2Cadr );
  Wire.write( CMD_WCFG );
  Wire.write( config | ( ~config << 4 ) );
  Wire.endTransmission( );
  Wire.requestFrom( I2Cadr, 1 );
  read_config = Wire.read( );
#endif
	read_config = owcmd( CMD_WCFG, ( config | ( ~config<<4 ) ) );
   // check for failure due to incorrect read back
   #ifdef DEBUG
     Serial.print( " * configReadBack (BIN): " );
     Serial.println( read_config, BIN );
   #endif

   if(config != read_config )
   {
      // handle error
      // ...
      DS2482_reset();

      return false;
   }

   return true;
}


//--------------------------------------------------------------------------
// Reset all of the devices on the 1-Wire Net and return the result.
//
// Returns: true(1):  presence pulse(s) detected, device(s) reset
//          false(0): no presence pulses detected
//
bool DS2482::OWReset( )
{
   uint8_t status;
#if 0
   int poll_count = 0;

  Wire.beginTransmission( I2Cadr );
  Wire.write( CMD_1WRS );
  Wire.endTransmission( );
  Wire.requestFrom( I2Cadr, 1 );
  status = Wire.read( );
  do {
    Wire.requestFrom( I2Cadr, 1 );
//    status = Wire.read( ) & 1<<(ST_1WB);
    status = Wire.read( );                //keep all bits for further tests
  } while( (status & 1<<(ST_1WB)) && (poll_count++ < POLL_LIMIT) );
  #ifdef DEBUG
    Serial.print( " * status " );
    Serial.println( status, HEX );
    Serial.print( " * poll_count " );
    Serial.println( poll_count, DEC );
  #endif
   // check for failure due to poll limit reached
   if (poll_count >= POLL_LIMIT)
   {
      // handle error
      // ...
      DS2482_reset();
      return false;
   }
#endif
	status = owcmdw( CMD_1WRS );

   // check for short condition
   if (status & (1<<ST_SD))
      short_detected = true;
   else
      short_detected = false;

   // check for presence detect
   if (status & (1<<ST_PPD))
      return true;
   else
      return false;
} //OWReset( )


//--------------------------------------------------------------------------
// Send 1 bit of communication to the 1-Wire Net.
// The parameter 'sendbit' least significant bit is used.
//
// 'sendbit' - 1 bit to send (least significant byte)
//
void DS2482::OWWriteBit(uint8_t sendbit)
{
   OWTouchBit(sendbit);
}

//--------------------------------------------------------------------------
// Reads 1 bit of communication from the 1-Wire Net and returns the
// result
//
// Returns:  1 bit read from 1-Wire Net
//
uint8_t DS2482::OWReadBit(void)
{
   return OWTouchBit(0x01);
}

//--------------------------------------------------------------------------
// Send 1 bit of communication to the 1-Wire Net and return the
// result 1 bit read from the 1-Wire Net. The parameter 'sendbit'
// least significant bit is used and the least significant bit
// of the result is the return bit.
//
// 'sendbit' - the least significant bit is the bit to send
//
// Returns: 0:   0 bit read from sendbit
//          1:   1 bit read from sendbit
//
uint8_t DS2482::OWTouchBit(uint8_t sendbit)
{
   uint8_t status;
   int poll_count = 0;
#if 0
  Wire.beginTransmission( I2Cadr );
  Wire.write( CMD_1WSB );
  Wire.write( sendbit ? 0x80 : 0x00 );
  Wire.endTransmission( );
  Wire.requestFrom( I2Cadr, 1 );
  status = Wire.read( );
  do {
    Wire.requestFrom( I2Cadr, 1 );
    status = Wire.read( );
  } while( (status & 1<<(ST_1WB)) && (poll_count++ < POLL_LIMIT) );

   // check for failure due to poll limit reached
   if (poll_count >= POLL_LIMIT)
   {
      // handle error
      // ...
      DS2482_reset();
      return 0;
   }
#endif
	status = owcmdw( CMD_1WSB, (sendbit ? 0x80 : 0x00) );
   // return bit state
   if (status & 1<<(ST_SBR))
      return 1;
   else
      return 0;
} //OWTouchBit


//--------------------------------------------------------------------------
// Send 8 bits of communication to the 1-Wire Net and verify that the
// 8 bits read from the 1-Wire Net are the same (write operation).
// The parameter 'sendbyte' least significant 8 bits are used.
//
// 'sendbyte' - 8 bits to send (least significant byte)
//
// Returns:  true: bytes written and echo was the same
//           false: echo was not the same
//
void DS2482::OWWriteByte(uint8_t sendbyte)
{
#if 0
   uint8_t status;
   int poll_count = 0;

   // loop checking 1WB bit for completion of 1-Wire operation
   // abort if poll limit reached
  Wire.beginTransmission( I2Cadr );
  Wire.write( CMD_1WWB );
  Wire.write( sendbyte );
  Wire.endTransmission( );
  Wire.requestFrom( (int)I2Cadr, 1 );
  status = Wire.read( );
  do {
    Wire.requestFrom( I2Cadr, 1 );
    status = Wire.read( );
  } while( (status & 1<<(ST_1WB)) && (poll_count++ < POLL_LIMIT) );
  
   // check for failure due to poll limit reached
   if (poll_count >= POLL_LIMIT)
   {
      // handle error
      // ...
      DS2482_reset();
   }
#endif
	owcmdw( CMD_1WWB, sendbyte );
}// OWWriteByte( )


//--------------------------------------------------------------------------
// Send 8 bits of read communication to the 1-Wire Net and return the
// result 8 bits read from the 1-Wire Net.
//
// Returns:  8 bits read from 1-Wire Net
//
uint8_t DS2482::OWReadByte(void)
{
#if 0
   uint8_t data, status;
   int poll_count = 0;

  Wire.beginTransmission( I2Cadr );
  Wire.write( CMD_1WRB );
  Wire.endTransmission( );
  Wire.requestFrom( I2Cadr, 1 );
  status = Wire.read( );
  do {
    Wire.requestFrom( I2Cadr, 1 );
    status = Wire.read( );
  } while( (status & 1<<(ST_1WB)) && (poll_count++ < POLL_LIMIT) );

   // check for failure due to poll limit reached
   if (poll_count >= POLL_LIMIT)
   {
      // handle error
      // ...
      DS2482_reset();
      return 0;
   }

  Wire.beginTransmission( I2Cadr );
  Wire.write( CMD_SRP );
  Wire.write( DATAREG );
  Wire.endTransmission( );
  Wire.requestFrom( I2Cadr, 1 );
  data = Wire.read( );
  
   return data;
#endif
	owcmdw( CMD_1WRB );
	return( owcmd( CMD_SRP, DATAREG ) );
} //OWReadByte( )


//--------------------------------------------------------------------------
// The 'OWBlock' transfers a block of data to and from the
// 1-Wire Net. The result is returned in the same buffer.
//
// 'tran_buf' - pointer to a block of unsigned
//              chars of length 'tran_len' that will be sent
//              to the 1-Wire Net
// 'tran_len' - length in bytes to transfer
//
void DS2482::OWBlock(uint8_t *tran_buf, int tran_len)
{
   int i;

   for (i = 0; i < tran_len; i++)
      tran_buf[i] = OWTouchByte(tran_buf[i]);
} //OWBlock( )

//--------------------------------------------------------------------------
// Send 8 bits of communication to the 1-Wire Net and return the
// result 8 bits read from the 1-Wire Net. The parameter 'sendbyte'
// least significant 8 bits are used and the least significant 8 bits
// of the result are the return byte.
//
// 'sendbyte' - 8 bits to send (least significant byte)
//
// Returns:  8 bits read from sendbyte
//
uint8_t DS2482::OWTouchByte(uint8_t sendbyte)
{
   if (sendbyte == 0xFF)
      return OWReadByte();
   else
   

{
      OWWriteByte(sendbyte);
      return sendbyte;
   }
} //OWTouchByte( )



//--------------------------------------------------------------------------
// Find the 'first' devices on the 1-Wire network
// Return true  : device found, ROM number in ROM_NO buffer
//        false : no device present
//
bool DS2482::OWFirst()
{
   // reset the search state
   LastDiscrepancy = 0;
   LastDeviceFlag = false;
   LastFamilyDiscrepancy = 0;

   return OWSearch();
}

//--------------------------------------------------------------------------
// Find the 'next' devices on the 1-Wire network
// Return true  : device found, ROM number in ROM_NO buffer
//        false : device not found, end of search
//
bool DS2482::OWNext()
{
   // leave the search state alone
   return OWSearch();
}

//--------------------------------------------------------------------------
// The 'OWSearch' function does a general search. This function
// continues from the previous search state. The search state
// can be reset by using the 'OWFirst' function.
// This function contains one parameter 'alarm_only'.
// When 'alarm_only' is true (1) the find alarm command
// 0xEC is sent instead of the normal search command 0xF0.
// Using the find alarm command 0xEC will limit the search to only
// 1-Wire devices that are in an 'alarm' state.
//
// Returns:   true (1) : when a 1-Wire device was found and its
//                       Serial Number placed in the global ROM
//            false (0): when no new device was found.  Either the
//                       last search was the last device or there
//                       are no devices on the 1-Wire Net.
//
bool DS2482::OWSearch()
{
   int id_bit_number;
   int last_zero, rom_byte_number;
   int  cmp_id_bit;
   uint8_t rom_byte_mask, search_direction, status;
   bool id_bit, search_result;

   // initialize for search
   id_bit_number = 1;
   last_zero = 0;
   rom_byte_number = 0;
   rom_byte_mask = 1;
   search_result = false;
   crc8 = 0;

   // if the last call was not the last one
   if (!LastDeviceFlag)
   {  
      // 1-Wire reset
      if (!OWReset())
      {
         // reset the search
         LastDiscrepancy = 0;
         LastDeviceFlag = false;
         LastFamilyDiscrepancy = 0;
         return false;
      }

      // issue the search command
      OWWriteByte(0xF0);

      // loop to do the search
      do
      {
         // if this discrepancy if before the Last Discrepancy
         // on a previous next then pick the same as last time
         if (id_bit_number < LastDiscrepancy)
         {
            if ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0)
               search_direction = 1;
            else
               search_direction = 0;
         }
         else
         {
            // if equal to last pick 1, if not then pick 0
            if (id_bit_number == LastDiscrepancy)
               search_direction = 1;
            else
               search_direction = 0;
         }

         // Perform a triple operation on the DS2482 which will perform
         // 2 read bits and 1 write bit
         status = DS2482_search_triplet(search_direction);

         // check bit results in status byte
         id_bit = ((status & (1<<ST_SBR)) == (1<<ST_SBR));
         cmp_id_bit = ((status & (1<<ST_TSB)) == (1<<ST_TSB));
         search_direction =
           ((status & (1<<ST_DIR)) == (1<<ST_DIR)) ? (uint8_t)1 : (uint8_t)0;

         // check for no devices on 1-Wire
         if ((id_bit) && (cmp_id_bit))
            break;
         else
         {
            if ((!id_bit) && (!cmp_id_bit) && (search_direction == 0))
            {
               last_zero = id_bit_number;

               // check for Last discrepancy in family
               if (last_zero < 9)
                  LastFamilyDiscrepancy = last_zero;
            }

            // set or clear the bit in the ROM byte rom_byte_number
            // with mask rom_byte_mask
            if (search_direction == 1)
               ROM_NO[rom_byte_number] |= rom_byte_mask;
            else
               ROM_NO[rom_byte_number] &= (uint8_t)~rom_byte_mask;

            // increment the byte counter id_bit_number
            // and shift the mask rom_byte_mask
            id_bit_number++;
            rom_byte_mask <<= 1;

            // if the mask is 0 then go to new SerialNum byte rom_byte_number
            // and reset mask
            if (rom_byte_mask == 0)
            {
               calc_crc8(ROM_NO[rom_byte_number]);  // accumulate the CRC
               rom_byte_number++;
               rom_byte_mask = 1;
            }
         }
      }
      while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7
      // if the search was successful then
      if (!((id_bit_number < 65) || (crc8 != 0)))
      {
         // search successful so set LastDiscrepancy,LastDeviceFlag
         // search_result
         LastDiscrepancy = last_zero;

         // check for last device
         if (LastDiscrepancy == 0)
            LastDeviceFlag = true;

         search_result = true;
      }
   }
   // if no device found then reset counters so next
   // 'search' will be like a first

   if (!search_result || (ROM_NO[0] == 0))
   {
      LastDiscrepancy = 0;
      LastDeviceFlag = false;
      LastFamilyDiscrepancy = 0;
      search_result = false;
   }

   return search_result;
}

//--------------------------------------------------------------------------
// Use the DS2482 help command '1-Wire triplet' to perform one bit of a
//1-Wire search.
//This command does two read bits and one write bit. The write bit
// is either the default direction (all device have same bit) or in case of
// a discrepancy, the 'search_direction' parameter is used.
//
// Returns – The DS2482 status byte result from the triplet command
//
uint8_t DS2482::DS2482_search_triplet(int search_direction)
{
	return( owcmdw( CMD_1WT, search_direction ? 0x80 : 0x00 ) );
} //DS2482_search_triplet( )


// calculate crc8
uint8_t DS2482::calc_crc8( uint8_t &rombyte ) {
  crc8 = dscrc_table[crc8 ^ rombyte ];
  return crc8;
}


//--------------------------------------------------------------------------
// Set the 1-Wire Net line level pullup to normal. The DS2482 only
// allows enabling strong pullup on a bit or byte event. Consequently this
// function only allows the MODE_STANDARD argument. To enable strong pullup
// use OWWriteBytePower or OWReadBitPower.
//
// 'new_level' - new level defined as
//                MODE_STANDARD     0x00
//
// Returns:  current 1-Wire Net level
//
uint8_t DS2482::OWLevel(uint8_t new_level)
{
   // function only will turn back to non-strong pullup
   if (new_level != MODE_STANDARD)
      return MODE_STRONG;

   // clear the strong pullup bit in the global config state
   cSPU = 0;

   // write the new config
   DS2482_write_config( MODE_STANDARD );   //clear strong pullup, set active pullup 

   return MODE_STANDARD;
} //OWLevel( )

//--------------------------------------------------------------------------
// Send 1 bit of communication to the 1-Wire Net and verify that the
// response matches the 'applyPowerResponse' bit and apply power delivery
// to the 1-Wire net.  Note that some implementations may apply the power
// first and then turn it off if the response is incorrect.
//
// 'applyPowerResponse' - 1 bit response to check, if correct then start
//                        power delivery
//
// Returns:  true: bit written and response correct, strong pullup now on
//           false: response incorrect
//
int DS2482::OWReadBitPower(int applyPowerResponse)
{
   uint8_t rdbit;

   // set strong pullup enable
   cSPU = 1<<(SPU);

   // write the new config
   if (!DS2482_write_config( cSPU ))
      return false;

   // perform read bit
   rdbit = OWReadBit();

   // check if response was correct, if not then turn off strong pullup
   if (rdbit != applyPowerResponse)
   {
      OWLevel( MODE_STANDARD );
      return false;
   }

   return true;
} //OWReadBitPower( )

//--------------------------------------------------------------------------
// Send 8 bits of communication to the 1-Wire Net and verify that the
// 8 bits read from the 1-Wire Net are the same (write operation).
// The parameter 'sendbyte' least significant 8 bits are used. After the
// 8 bits are sent change the level of the 1-Wire net.
//
// 'sendbyte' - 8 bits to send (least significant bit)
//
// Returns:  true: bytes written and echo was the same, strong pullup now on
//           false: echo was not the same
//
int DS2482::OWWriteBytePower(int sendbyte)
{
   // set strong pullup enable
   cSPU = 1<<(SPU);

   // write the new config
   if (!DS2482_write_config(cSPU))
      return false;

   // perform write byte
   OWWriteByte(sendbyte);

   return true;
} //OWWriteBytePower( )



