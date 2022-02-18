// DS2482.h - header for I2C-to-onewire bridge IC
//
// Started: Jan 21, 2022  G. D. (Joe) Young <jyoung@islandnet.com>
//
// Revised: Feb  1/22 - make calc_crc8 public
//
//
//
// A library of functions from Dallas/Maxim Application Note AN3684, altered to
// conform to an arduino library format and to use the arduino Wire library
// for I2C I/O between arduino and the bridge IC. The bridge IC handles the 
// timing for the one-wire transactions.
//
#ifndef DS2482_HDR
#define DS2482_HDR

#include <stdint.h>
#include <Wire.h>

//DS2482 register addresses
#define DATAREG 0xE1
#define STATREG 0xF0
#define CNFGREG 0xC3

//DS2482 command definitions
#define CMD_DRST 0xF0   //device reset
#define CMD_WCFG 0xD2   //write configuration
#define CMD_1WRS 0xB4   //one-wire reset
#define CMD_1WSB 0x87   //one-wire single bit
#define CMD_1WWB 0xA5   //one-wire write byte
#define CMD_1WRB 0x96   //one-wire read byte
#define CMD_WWBP 0x44   //one-wire write byte power
#define CMD_SRP  0xE1   //set read pointer
#define CMD_1WT  0x78   //one-wire triplet

#define POLL_LIMIT 10     //number of times to check status of reset

//DS2482 status register bit number names
#define ST_1WB 0  //1WB one-wire busy
#define ST_PPD 1     //presence pulse detect
#define ST_SD 2      //Short Detected
#define ST_LL 3      //logic level
#define ST_RST 4     //device reset
#define ST_SBR 5     //single bit result
#define ST_TSB 6     //triplet second bit
#define ST_DIR 7     //branch direction taken

//cofiguration settings - register bit number names
#define CONFIG_APU 1    //active pullup enabled
#define APU 0       //active pull-up enabled when 1
#define SPU 2       //strong pull-up enabled when 1
#define wWS 3       //1WS in data sheet; 1-wire speed fast when 1
#define MODE_STANDARD 0x01  //APU bit ON
#define MODE_STRONG 0x04  //SPU bit ON


//#define DEBUG
#include <Arduino.h>

class DS2482 {

public:
	DS2482( uint8_t _i2cAdr );				//constructor receives DS2482 I2C address
	void begin( );

	bool DS2482_reset();
	bool DS2482_detect( );
	bool DS2482_write_config(uint8_t config);
	bool OWReset( );
	uint8_t OWTouchBit(uint8_t sendbit);
	uint8_t OWTouchByte(uint8_t sendbyte);
	bool OWSearch();
	bool OWFirst( );
	bool OWNext();
	void OWWriteBit(uint8_t sendbit);
	uint8_t OWReadBit(void);
	void OWWriteByte(uint8_t sendbyte);
	uint8_t OWReadByte(void);
	void OWBlock(uint8_t *tran_buf, int tran_len);
	uint8_t DS2482_search_triplet(int search_direction);
	int OWWriteBytePower(int sendbyte);
	int OWReadBitPower(int applyPowerResponse);
	uint8_t OWLevel(uint8_t new_level);

	bool short_detected;
	uint8_t ROM_NO[8];
	uint8_t calc_crc8( uint8_t &rombyte );

private:
	int I2Cadr;
	uint8_t c1WS,cSPU,cPPM,cAPU;

// Search state
	int LastDiscrepancy;
	int LastFamilyDiscrepancy;
	bool LastDeviceFlag;
	uint8_t crc8;

	const uint8_t dscrc_table[256] = {                 /* crc table */

   0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
  157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
  35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
  190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
  70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
  219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
  101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
  248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
  140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
  17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
  175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
  50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
  202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
  87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
  233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
  116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53

	};


}; //class DS2482

#endif

