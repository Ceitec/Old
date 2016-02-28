/* ========================================================================== */
/*                                                                            */
/*   leds.h                                                                   */
/*   (c) 2015 Michal Petrilak                                                 */
/*                                                                            */
/*   head file for leds.s                                                     */
/*                                                                            */
/*   libraly for sending led states to 74HC595 without SPI module             */
/*   using only 2 pins on MCU                                                 */
/*                                                                            */
/*                                                                            */
/* ========================================================================== */
#ifndef __LEDS_H
#define __LEDS_H

#ifdef __ASSEMBLER__

#define clkR      r18   ; reset CLK
#define clkS      r19   ; set CLK
#define dat       r20   ; data to send
#define datR      r21   ; reset data
#define datS      r22   ; set data
#define counter   r23   ; bit counter

#else

byte leds;
void led_send(void);

#define led_ok       0
#define led_comm     1
#define led_ov1_ok   2
#define led_ov1_err  3
#define led_ov2_ok   4
#define led_ov2_err  5
#define led_zav_a    6
#define led_zav_b    7

#define led_on(x)    leds |=  BV(x);
#define led_off(x)   leds &= ~BV(x);


#endif
#endif