/*
 *
 * timer initialization and operation
 *
 */

#include <avr/io.h>
#include "common_defs.h"

//----------------------------------------------------
void timer_init(void)
{
  // Xtal = 14 745 600 Hz

  // * Timer 0 - system timer ....
  // 14745600 / 1024 / 144 = 100 Hz
  //   Xtal  /must/presca/ TOP
  OCR0A = 144; // = TOP
  OCR0B = 100; // dummy
  TCCR0A = 2; // CTC mode, hardware outputs off
  TCCR0B = 5; // CTC mode, presc = 1024
  TIMSK0 |= BV(OCIE0A); // compare A int enabled

  // * Timer 1 - unused
  // * Timer 2 - unused
}

//----------------------------------------------------

//----------------------------------------------------
