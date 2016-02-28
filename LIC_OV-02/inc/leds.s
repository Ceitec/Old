                        
#define _SFR_ASM_COMPAT 1
#define __SFR_OFFSET 0
#include <avr/io.h>
#include "leds.h"

.global led_send; The assembly function must be declared as global

; 18-27 - volné registry
; 30-31 - volné registry

led_send:
            cli
            
            ldi     r30, lo8(leds)
            ldi     r31, hi8(leds)
            ld      r20, Z
            
            ; prepare data to PORTB (CLK bit)
            in      r18, PORTB
            mov     r19, r16
            andi    r18, 0xFE
            ori     r19, 0x001
            
            ; prepare data to send
            in      r21, PORTD
            mov     r22, r19
            andi    r21, 0x7F
            ori     r22, 0x80
            
            ldi     r23, 8
            
            ; send first 7 bits
bits:       out     PORTB, clkR
            rol     dat
            out     PORTD, datR
            brcc    bit0
            out     PORTD, datS
bit0:       
            out     PORTB, clkS
            dec     counter
            brne    bits
            
            sei
            ldi     r20, 0x90
            out     PORTD, datR

loop1:
            nop
            dec     dat
            brne    loop1

            out     PORTB, clkR
            nop

ahead:
            ret

