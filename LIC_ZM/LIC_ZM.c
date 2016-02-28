/*
 * LIC_ZM.c
 *
 * Created: 2.7.2014 12:50:07
 *  Author: Michal
 *
 * Main control board
 *
 * USART0 - USB (PC)
 * USART1 - RS485
 * I2C - ATmega8
 *
 */

#include "inc/common_defs.h"
#include "inc/defines.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "inc/uart_types.h"
#include "inc/uart_tri_1.h"
#include "inc/uart_tri_0.h"
#include "inc/extBus.h"

#include "inc/adc.h"
#include "inc/timer.h"
#include "inc/Tribus_types.h"
#include "inc/Tribus.h"
#include "inc/packet_processor.h"

volatile byte timer0_flag = 0; // T = 10ms

byte Dtime = 0;

//----------------------------------------------------------
ISR(TIMER0_COMPA_vect) {
  // T = 10ms
  timer0_flag = true;
}

ISR(TIMER0_COMPB_vect) {
  // T = 10ms
  timer0_flag = true;
}

ISR(TIMER0_OVF_vect) {
  // T = 10ms
  timer0_flag = true;
}

//----------------------------------------------------------
// ERR led operation
inline void led_on(void)
{
  PORTB |= BV(PB0);
}

inline void led_off(void)
{
  PORTB &= ~BV(PB0);
}

inline void led_toggle(void)
{
  PORTB ^= BV(PB0);
}

//----------------------------------------------------------
void process_timer_100Hz(void)
{
  if (timer0_flag) { // T = 10ms
    timer0_flag = false;
    uart0_ISR_timer();
    uart1_ISR_timer();
    //uart2_ISR_timer();
  }
}

//----------------------------------------------------------
void init(void)
{

  DDRB = BV(PB0);
  DDRD = BV(PD1) | BV(PD3) | BV(PD4);

  uart0_init(); // PC
  uart1_init(); // internal
  uart2_init(); // external (via extBus)
  adc_init();
  timer_init();
  TB_Callback_setBaud = uart1_set_baud;
  TB_Init((void*) 0x10); // addr in eeprom with settings
  pp_init();
  sei();
}

//----------------------------------------------------------
int main(void)
{
  init();
  led_off();
  
  //Ddebug();

  while(1) { // mail loop
  
    pp_loop();
    process_timer_100Hz();
    uart0_process();
    uart1_process();
    //uart2_process();

    if (TB_out.b0) {
      led_on();
     } else {
      led_off();
    }

  }
}