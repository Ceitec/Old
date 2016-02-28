#include "defines.h"
#include "common_defs.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

word adc_data;

void adc_init(void)
{
  ADCSRA = BV(ADEN) | 7; // clk / 128
  ADCSRB = 0;
  DIDR0 = 0xFF; // all pin C is analog
  ADMUX  = 0;
  adc_data = 0;
}

word adc_read (byte channel)
{

  ADMUX = (channel & 7);
  _delay_us(50);
  ADCSRA |= BV(ADSC);

  while ((ADCSRA & BV(ADSC)) > 0 );

  return (ADC);
}

void adc_on(void)
{

}

void adc_off(void)
{

}

