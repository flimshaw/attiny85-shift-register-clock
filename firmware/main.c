/* Name: main.c
 * Author: <insert your name here>
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// pin definitions for shift registers
#define SDA 2
#define SCL 1
#define LAT 0

// fine tuning time keeping functions, based around F_CPU being set to xtal freq
#define PRESCALE 0xff       // how many ticks until an overflow occurs
#define CLOCK_TUNING 180    // how many overflows fast or slow is the crystal
#define FIRST_OVERFLOW 227  // how much to offset the initial overflow
#define OVF_PER_SECOND (F_CPU / PRESCALE) - CLOCK_TUNING  // how many overlows per second

// constants for multiplexing pairs of digits
#define S1 0
#define M1 1
#define H1 2

// counter for overflows
unsigned int counter = 0;

// counters for secs/mins/hours
unsigned long seconds = 30;
unsigned long minutes = 20;
unsigned long hours = 22;

// var to hold full string for time readout
unsigned long timeVar = 0;

// the current digits being displayed (for multiplexing)
unsigned char currentDigit = 0;

// shifts an unsigned long value out to the shift registers
void shiftOut( unsigned long value ) {
  char i;
  for( i = 23; i >= 0; i-- ) {
    char b = (value >> i) & 1;  // read the least significant bit
    PORTB &= ~(1<<SCL);         // set the clock pin low
    if( b == 1 ) {              // if this bit is set
      PORTB |= (1<<SDA);        // set the data line high
    } else {                    // otherwise
      PORTB &= ~(1<<SDA);       // clear it
    }
    PORTB |= (1<<SCL);          // set the clock high
  }
}

// generates a string for the display based on current time and multiplexing state
void updateTime() {

  // build out the correct variable
  timeVar = 0;

  // ratchet the current digit into place
  switch( currentDigit ) {
    case S1:
      timeVar |= (seconds % 10) | (seconds / 10) << 4 | 0xffff00;
      break;
    case M1:
      timeVar |= (minutes % 10) << 8 | (minutes / 10) << 12 | 0xff00ff;
      break;
    case H1:
      timeVar |= (hours % 10) << 16 | (hours / 10) << 20 | 0x00ffff;
      break;
  }

  // step forward, displaying a different pair of digits each refresh
  currentDigit++;
  if( currentDigit > H1 ) {
    currentDigit = S1;
  }

  // close the output latch
  PORTB &= ~(1<<LAT);

  // shift out the variable
  shiftOut( timeVar );

  // open the output latch
  PORTB |= (1<<LAT);

}

// increment the second and increment values appropriately
void tick() {
  seconds++;

  // update the time
  if( seconds >= 60 ) {
    seconds = 0;
    minutes++;
    if( minutes >= 60 ) {
      minutes = 0;
      hours++;
      if( hours >= 24 ) {
        hours = 0;
      }
    }
  }

}

// initialize the timer and overflow interrrupts
void setup() {
  DDRB = 0xff;
  CLKPR |= (1<<CLKPCE); // just make sure the damn CLKDIV8 flag isn't set
  CLKPR = 0;            // and clear all system clock prescaling
  TCCR0A |= (1<<WGM01); // enable clear on compare
  TCCR0B |= (1<<CS00);  // enable the timer with no prescaling
  TIMSK |= (1<<OCIE0A); // enable timer0 output OCRA interrupt
  OCR0A = PRESCALE;
  sei();
}

// interrupt vector, fires after PRESCALE clock ticks
ISR(TIMER0_COMPA_vect) {

  counter++;

  // how long has passed since the compare matched?
  if( counter >= OVF_PER_SECOND ) {
    counter = 0;
    TCNT0 = FIRST_OVERFLOW; // very granular tuning, offset the initial clock cycle by a little bit
    tick();
  }
}

// the main loop
int main(void)
{
    setup();

    // and forever thus, we remain in this forloop
    for(;;){
      updateTime();
    }

    return 0;   /* never reached :( */
}
