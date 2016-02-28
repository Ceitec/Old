/*
 * LIC_ZM.c
 *
 * Created: 23.10.2014 19:25:00
 *  Author: Michal
 *
 * Light control board
 *
 * behave as TMCM-1021
 * -02 - new indication LED system
 *     - indegrated shutter driver
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
#include "inc/Tribus2.h"
#include "inc/leds.h"
#include "inc/adc.h"

#include <util/delay.h>

volatile byte timer0_flag = 0; // T = 10ms

byte led_timer = 0;

byte zaverka; // stav zaverky

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

void led_blik(void)
{
  led_timer = 2;
  led_on(led_comm);
}

inline void outA_dis(void) { osv1_on = 0; OCR1A = 0;}
inline void outB_dis(void) { osv2_on = 0; OCR1B = 0;}
inline void outA_ena(void) { osv1_on = 1;}
inline void outB_ena(void) { osv2_on = 1;}

//----------------------------------------------------
inline void pulse_open(void)
{
  PORTD |= BV(PD5);
}

//----------------------------------------------------
inline void pulse_close(void)
{
  PORTD |= BV(PD6);
}

//----------------------------------------------------
inline void pulse_off(void)
{
  PORTD &= ~(BV(PD5) | BV(PD6));
}

//----------------------------------------------------
void zaverka_close(void)
{
  pulse_close();
  _delay_ms(20);
  pulse_off();
}

//----------------------------------------------------
void zaverka_open(void)
{
  pulse_open();
  _delay_ms(20);
  pulse_off();
}

//----------------------------------------------------------
void send_data(void)
{
  uart0_put_data((byte *) &TB_bufOut);
}

//----------------------------------------------------------
void send_data2(void)
{
  uart0_put_data((byte *) &TB2_bufOut);
}

//----------------------------------------------------------
void try_receive_data(void)
{
  byte i;
  byte *ptr;
  
  if (!uart_pac_rx_empty()) {
    ptr = uart0_get_data_begin();
    for (i=0; i<9; i++) {
      TB_bufIn[i]  = *ptr;
      TB2_bufIn[i] = *ptr;
      ptr++;
    }
    uart0_get_data_end();
    if (TB_Read() == 0) {
      led_blik();
      switch (TB_Decode()) {
        case TB_CMD_SIO:
          if (TB_bufIn[TB_BUF_MOTOR] == 1) { // analog output (mimo dokumentaci)
            switch (TB_bufIn[TB_BUF_TYPE]) {
              case 0:
                // hard limit max. value
                if (TB_Value > 120) {
                  TB_Value = 120;
                }
                osv1 = (word) TB_Value;
                TB_SendAck(TB_ERR_OK, TB_Value);
                break;
              case 1:
                if (TB_Value > 60) {
                  TB_Value = 60;
                }
                osv2 = (word) TB_Value;
                TB_SendAck(TB_ERR_OK, TB_Value);
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
     else
    {
    if (TB2_Read() == 0) {
      led_blik();
        switch (TB2_Decode()) {
          case TB_CMD_SIO:
            if (TB2_out.b0) {
              // close
              if (zaverka) {
                // closed
                //
               } else {
                // do close
                zaverka_close();
                zaverka = 1;
                led_on (led_zav_b);
                led_off(led_zav_a);
        // DEBUG !!!
                uart0_flags.data_receive_error = false;
        // DEBUG !!!
              }
             } else { 
              // open
              if (zaverka) {
                // do open
                zaverka_open();
                zaverka = 0;
                led_on (led_zav_a);
                led_off(led_zav_b);
               } else {
                // opened
              }
            }
            break;
          //case
        }
      } // if TB2_Read
      } // else if TB_Read
  } // if data_received
}

//----------------------------------------------------------
void process_leds(void)
{
  // master OK led
  if (uart0_flags.data_receive_error) {
    led_off(led_ok);
   } else {
    led_on(led_ok);
  }
  
  // osv leds & on/off outputs
  if (osv1_on) {
    OCR1A = osv1;
    led_on(led_ov1_ok);
   } else {
    OCR1A = 0;
    led_off(led_ov1_ok);
  }
  if (osv2_on) {
    OCR1B = osv2;
    led_on(led_ov2_ok);
   } else {
    OCR1B = 0;
    led_off(led_ov2_ok);
  }
  
  // osv err leds
  if ((adc_get(ADC_U0) > 800) || (adc_get(ADC_U0) < 3)) {
    led_on(led_ov1_err);
   } else {
    led_off(led_ov1_err);
  }
  if ((adc_get(ADC_U1) > 800) || (adc_get(ADC_U1) < 3)) {
    led_on(led_ov2_err);
   } else {
    led_off(led_ov2_err);
  }
  
  // comm led timeout
  if (led_timer > 0) {
    led_timer--;
    if (led_timer == 0) {
      led_off(led_comm);
    }
  }
}



//----------------------------------------------------------
void process_timer_100Hz(void)
{
  if (timer0_flag) { // T = 10ms
    timer0_flag = false;
    uart0_ISR_timer();
    adc_process();
   
    // urèí stav led diod
    process_leds();
    // odešle stav všech led
    led_send();
      
  } // if timer0_flag
}

//----------------------------------------------------------
void init(void)
{

  DDRB = BV(PB0) | BV(PB1) | BV(PB2); // LEDs + PWM outputs
  PORTB = 0;
  DDRD = BV(PD1) | BV(PD2) | BV(PD5) | BV(PD6) | BV(PD7); // com + driver + LEDs

  leds=0;
  led_send();
  zaverka = 0;
  uart0_init();
  timer_init();
  adc_init();
  
  osv1 = 0;
  osv1_on = 0;
  osv2 = 0;
  osv2_on = 0;
  
  TB_Callback_setBaud = &uart0_set_baud;
  TB_Callback_TX = &send_data;
  TB2_Callback_TX = &send_data2;
  TB_Init ((void*) 0x10); // addr in eeprom with settings
  TB2_Init((void*) 0x20); // addr in eeprom with settings
  OCR1A = 3;
  OCR1B = 3;
  sei();
}

//----------------------------------------------------------
int main(void)
{
  init();
  
  led_on(led_ok);
  led_on (led_zav_a);
  led_off(led_zav_b);
    
  while(1) { // mail loop
  
    process_timer_100Hz();
    uart0_process();
    try_receive_data();
  }
}