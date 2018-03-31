/* Hello World program */

#include<stdio.h>

unsigned long seconds = 15;
unsigned long minutes = 45;
unsigned long hours = 1;

unsigned long timeVar = 0;
unsigned long PORTB = 0;

#define SDA 2
#define SCL 1
#define LAT 0

#define S1 0
#define S2 1
#define M1 2
#define M2 3
#define H1 4
#define H2 5

unsigned char currentDigit = 0;

void shiftOut( unsigned long value ) {
  char i;
  for( i = 23; i >= 0; i-- ) {
    char b = (value >> i) & 1;  // read the least significant bit
    PORTB &= ~(1<<SCL);         // set the clock pin low
    if( b == 1 ) {
      PORTB |= (1<<SDA);          // set the clock high
    } else {
      PORTB &= ~(1<<SDA);
    }
    if( i % 4 == 3 ) {
      printf(" ");
    }
    printf("%u", b);

    PORTB |= (1<<SCL);          // set the clock high
  }
}


main()
{

  for( int i = 0; i < 6; i++ ) {

    // seconds++;
    printf("%02u -", seconds);

    // build out the correct variable
    timeVar = 0;

    // ratchet the current digit into place
    switch( currentDigit ) {
      case S1:
        timeVar |= (seconds % 10) | 0xfffff0;
        break;
      case S2:
        timeVar |= (seconds / 10) << 4 | 0xffff0f;
        break;
      case M1:
        timeVar |= (minutes % 10) << 8 | 0xfff0ff;
        break;
      case M2:
        timeVar |= (minutes / 10) << 12 | 0xff0fff;
        break;
      case H1:
        timeVar |= (hours % 10) << 16 | 0xf0ffff;
        break;
      case H2:
        timeVar |= (hours / 10) << 20 | 0x0fffff;
        break;
    }

    currentDigit++;
    if( currentDigit > H2 ) {
      currentDigit = S1;
    }

    shiftOut( timeVar );
    printf("\n");
  }

  printf("\n");
}
