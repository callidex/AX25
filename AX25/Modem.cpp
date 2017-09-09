#include <Arduino.h>
#include "Modem.h"

volatile unsigned long lastTx = 0;
volatile unsigned long lastTxEnd = 0;
volatile unsigned long lastRx = 0;

#define BV(s) (1<<(s))

void Modem::start() {
    TCCR1A = 0;
    TCCR1B = BV(CS11) | BV(WGM13) | BV(WGM12);
    ICR1 = ((16000000 / 8) / 9600) - 1;
    ADMUX = BV(REFS0) | BV(ADLAR) | 0; // Channel 0, shift result left (ADCH used)
    DDRC &= ~BV(0);
    PORTC &= ~BV(0);
    DIDR0 |= BV(0);
    ADCSRB = BV(ADTS2) | BV(ADTS1) | BV(ADTS0);
    ADCSRA = BV(ADEN) | BV(ADSC) | BV(ADATE) | BV(ADIE) | BV(ADPS2); // | BV(ADPS0);
}

void Modem::timer() {
  TIFR1 = _BV(ICF1);
  
  if(encoder.isSending())
  {
    encoder.process(); // We have output to do
  }

}
