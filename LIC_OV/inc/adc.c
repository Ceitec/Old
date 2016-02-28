#include "defines.h"
#include "common_defs.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "adc.h"

word adc_data;
word adc_meas[8];

//----------------------------------------------------------
void adc_init(void)
{
  ADCSRA = BV(ADEN) | 5; // clk / 128
  ADMUX  = 0x40;
  adc_data = 0;
}

word adc_read (byte channel)
{
  ADMUX = 0x40 | (channel & 7);
  _delay_us(50);
  ADCSRA |= BV(ADSC);

  while ((ADCSRA & BV(ADSC)) > 0 );

  return (ADC);
}

//----------------------------------------------------------
// get value from periodical scan
word adc_get(byte channel)
{
  if (channel < 8) {
    return adc_meas[channel];
   } else {
    return 0;
  }
}

//----------------------------------------------------------
// periodicaly measure all channels
void adc_process(void)
{
  static byte meas_state = 0; // state machine
  
  switch (meas_state) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
      adc_meas[meas_state] = adc_read(meas_state);
      meas_state++;
      break;
    case 30:
      meas_state = 0;
    default:
      meas_state++;
      break;
  }
  
}

