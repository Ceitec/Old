/*
 * LIC_ZM.c
 *
 * Created: 23.10.2014 19:25:00
 *  Author: Michal
 *
 * Light control board
 *
 * behave as TMCM-1021
 * 
 *   
 */

#include "inc/common_defs.h"
#include "inc/defines.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "inc/uart_types.h"
#include "inc/uart_tri_0.h"
#include "inc/timer.h"
#include "inc/Tribus_types.h"
#include "inc/Tribus.h"
#include "inc/adc.h"

volatile byte timer0_flag = 0; // T = 10ms

byte led_timer = 0;

word osv1;
word osv2;
byte osv1_on;
byte osv2_on;

//----------------------------------------------------------
ISR(TIMER1_CAPT_vect) {
  static byte timer1_postscaler = 0;
  
  if (timer1_postscaler > 4) {
    // T = 10ms
    timer0_flag = true;
    timer1_postscaler = 0;
  } else {
    timer1_postscaler++;
  }
}

//----------------------------------------------------------
// ERR led operation
inline void led_on(void)     { PORTD |=  BV(PD6); }
inline void led_off(void)    { PORTD &= ~BV(PD6); }
inline void led_toggle(void) { PORTD ^=  BV(PD6); }

void led_blik(void)
{
  led_timer = 10;
  led_off();
}

inline void outA_dis(void) { TCCR1A &= ~(BV(COM1A1)); }
inline void outB_dis(void) { TCCR1A &= ~(BV(COM1B1)); }
inline void outA_ena(void) { TCCR1A |=  (BV(COM1A1)); }
inline void outB_ena(void) { TCCR1A |=  (BV(COM1B1)); }

//----------------------------------------------------------
void send_data(void)
{
  uart0_put_data((byte *) &TB_bufOut);
}

//----------------------------------------------------------
void try_receive_data(void)
{
  byte i;
  byte *ptr;
  
  if (uart0_flags.data_received) {
    ptr = uart0_get_data_begin();
    for (i=0; i<9; i++) {
      TB_bufIn[i] = *ptr;
      ptr++;
    }
    uart0_get_data_end();
    if (TB_Read() == 0) {
      switch (TB_Decode()) {
        case TB_CMD_SIO:
          if (TB_bufIn[TB_BUF_MOTOR] == 1) { // analog output (mimo dokumentaci)
            switch (TB_bufIn[TB_BUF_TYPE]) {
              case 0:
                //if (TB_Value & 1) led_on(); else led_off();
                OCR1A = (word) TB_Value;
                led_blik();
                TB_SendAckOK();
                break;
              case 1:
                OCR1B = (word) TB_Value;
                led_blik();
                TB_SendAckOK();
                break;
              default:
                TB_SendAck(TB_ERR_TYPE, 0); // invalid value
                break;
            }
          };
          if (TB_bufIn[TB_BUF_MOTOR] == 2) { // digital output
            switch (TB_bufIn[TB_BUF_TYPE]) {
              case 0:
                if (TB_out.b0 != 0) {
                  // ch 0 zapnuto
                  outA_ena();
                 } else {
                  // ch 0 vypnuto
                  outA_dis();
                }
                return;
              case 1:
                if (TB_out.b1 != 0) {
                  // ch 1 zapnuto
                  outB_ena();
                 } else {
                  // ch 2 vypnuto
                  outB_dis();
                }
                return;
              default:
                return;
            };
          };
          break;
        case TB_CMD_GIO:
          // analog inputs (measure)
          if (TB_bufIn[TB_BUF_TYPE] < 8) {
            // return ADC data
            TB_SendAck(TB_ERR_OK, adc_get(TB_bufIn[TB_BUF_TYPE]));
            } else if (TB_bufIn[TB_BUF_TYPE] == 8) {
              // return actual channel 0
              TB_SendAck(TB_ERR_OK, OCR1A);
            } else if ((TB_bufIn[TB_BUF_TYPE] == 9)) {
              // return actual channel 1
              TB_SendAck(TB_ERR_OK, OCR1B);
          } else {
           // requested channel not exist
           TB_SendAck(TB_ERR_TYPE, 0);
          }      
          break;
        default:   
          break;
      } // switch
    } // if TB_Read
  } // if data_received
}

//----------------------------------------------------------
void process_timer_100Hz(void)
{
  if (timer0_flag) { // T = 10ms
    timer0_flag = false;
    uart0_ISR_timer();
    
    if (led_timer > 0) {
      led_timer--;
      if (led_timer == 0) {
        led_on();
      }
    }
    
    adc_process();
  }
}

//----------------------------------------------------------
void init(void)
{

  DDRB = BV(PB1) | BV(PB2); // PWM outputs
  DDRD = BV(PD1) | BV(PD2) | BV(PD6); // com + LED

  uart0_init();
  timer_init();
  adc_init();
  
  osv1 = 0;
  osv1_on = 0;
  osv2 = 0;
  osv2_on = 0;
  
  TB_Callback_setBaud = &uart0_set_baud;
  TB_Callback_TX = &send_data;
  TB_Init((void*) 0x10); // addr in eeprom with settings
  OCR1A = 3;
  OCR1B = 3;
  sei();
}

//----------------------------------------------------------
int main(void)
{
  init();
  led_on();
  
  //Ddebug();

  while(1) { // mail loop
  
    process_timer_100Hz();
    uart0_process();
    try_receive_data();

  }
}