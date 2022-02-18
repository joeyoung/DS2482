// printing numerical values in fixed-width fields
//
// Dec 17/21 - change printFixInt to printFixUint, add new printFixInt
//
char prStr[PRSTRSIZE];    // array to hold output characters

void printFixHex( uint32_t val, uint8_t width, char *out ) {
  for( uint8_t ix = width; ix > 0; ix-- ) {
    uint8_t jx = ix-1;
    out[jx] = (val & 0x0f) + '0';
    if( out[jx] > '9' ) out[jx] += 7;
    val >>= 4;
  }
  out[width] = NULL;
} // printFixHex( )

//unsigned integer to decimal output
void printFixUint( uint32_t val, uint8_t width, char *out ) {
  for( uint8_t ix=width; ix>0; ix-- ) {
    uint8_t jx = ix-1;
    if( val == 0 ) {
      out[jx] = '0';
    } else {
      uint32_t v10 = val/10;
      out[jx] = val - v10*10 + '0';
      val = v10;
    }
  }
  out[width] = NULL;
}// printFixUint( )

//signed integer printing in fixed-width columns
void printFixInt( int32_t val, uint8_t width, char *out ) {
  if( val < 0 ) {
    *out = '-';
    val = -val;
  } else {
    *out = ' ';  
  }
  out++;
  width--;
  printFixUint( (uint32_t)val, width, out );
}// printFixInt( )

