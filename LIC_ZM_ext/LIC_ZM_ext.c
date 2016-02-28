/*
 * LIC_ZM_ext.c
 *
 * Created: 3.7.2014 8:54:15
 *  Author: Michal
 */

#include <avr/pgmspace.h>
#include "inc/common_defs.h"
#include "inc/defines.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
//#include <avr/eeprom.h>
#include "inc/uart_tri_0.h"
#include "inc/i2c.h"

#include <util/delay.h>

//#include "adc.h"


// System variables
byte address;

// User variables
byte txing = false;
byte status;
byte CMD = CMD_NULL;
byte rcv_len = 0;		// number of bytes to send to master

void uart_sendbuf(void);
void send_info(void);

void i2c_rcv  (u08 receiveDataLength, u08* receiveData);
u08  i2c_send (u08 transmitDataLengthMax, u08* transmitData);

//byte EEMEM adr = 2;


/******************************
**         Interrupt         **
******************************/


/******************************************************/
ISR(TIMER0_OVF_vect)
{

}

/******************************************************/
ISR(TIMER1_COMPA_vect)
{

}


/******************************
**           Main            **
******************************/

int main(void)
{
  //MCUCSR = 0;
  
  //  wdt_reset();
  //  wdt_enable(WDTO_60MS);
  //  TCCR0 = 2;
  //  TIMSK |= BV(TOIE0) | BV(OCIE1A);

  // Timer1 = CTC
  //  TCCR1A = 0;
  //  TCCR1B = BV(WGM12) | 1;
  //  OCR1A = 737; // 7.37MHz / 737 ~ 10kHz

  // Timer2 = Fast PWM
  //TCCR2 = 0x61;
  //OCR2=0; // f = fio = 31,250 kHz

  //  uart_init();
  UCSRA = 0;
  //  uart_interrupt_rx(true);
  //  uart_interrupt_tx(true);


  //  PB
  DDRB  = 0x00; // prog pins
  DDRC  = 0x00; // IPC, need change
  DDRD  = 0x06; // TXD & TE

  PORTB = 0;
  PORTC = 0;
  PORTD = 0;

  // load settings from eeprom
  address = 1;

  // initialize i2c function library
  i2cInit();
  i2cSetLocalDeviceAddr(address << 1, false);
  i2cSetSlaveReceiveHandler(i2c_rcv);
  i2cSetSlaveTransmitHandler(i2c_send);

  // initialize UART interface
  uart0_init();
  UCSRA = 0;
  uart_interrupt_rx(true);
  uart_interrupt_tx(true);

  sei();

  // main loop
  for (;;) {
    //    wdt_reset();
    if (uart_rx_size()) { status |= 1; } else { status &= ~1; }
    if (uart_tx_size()) { status |= 2; } else { status &= ~2; }
  }
}


/******************************
**          Rutiny           **
******************************/

void i2c_rcv (u08 receiveDataLength, u08* recieveData)
{
  byte i;
  if (receiveDataLength < 1) return;

    //uart_put_char('x');
    //uart_put_char(recieveData[0]);
    //uart_send();


  switch (recieveData[0]) {

    case CMD_BAUD_RATE:
      // set baud rate (Baud_Rate / 4 !)
      if (receiveDataLength != 3) break;
      //uart_set_rate((recieveData[1] << 9) | (recieveData[2] << 1));
      CMD = CMD_NULL;
      break;

    case CMD_SEND:
      // send serial data

      // copy data form I2C to UART buffer
      for(i=0; i< (receiveDataLength-2); i++) {
        /*
        if (!uart_put_char(recieveData[i+2])) {
          status |= 8;
        }
        */
        //uart_put_char(recieveData[i+2]);
      }

      // transmit start
      uart_send();
      CMD = CMD_NULL;
      break;

    case CMD_RTS:
      // set RTS line state
      if (recieveData[1]) {
        cbi(RTS_PORT, RTS_PIN);
       } else {
        sbi(RTS_PORT, RTS_PIN);
      }
      CMD = CMD_NULL;
      break;

    case CMD_STATUS_C:
      status &= ~0x0C;
      break;

    // commands which generate response
    case CMD_RCV:
      rcv_len = recieveData[1];
    case CMD_IDENT:
    case CMD_STATUS:
    case CMD_RCV_SIZE:
    case CMD_CTS:
      CMD = recieveData[0];
    break;
  }

}

/******************************************************/
u08 i2c_send (u08 transmitDataLengthMax, u08* transmitData)
{
  byte i, len;

  switch (CMD) {
    case CMD_NULL:
      return 0;
      break;
    case CMD_IDENT:
      // Send identification string
      transmitData[0] = CMD_IDENT;
      transmitData[1] = 'E';
      transmitData[2] = 'C';
      transmitData[3] = 'O';
      transmitData[4] = 'M';
      transmitData[5] = '0' + (address);
      return 6;
      break;
    case CMD_RCV:
      transmitData[0] = CMD_RCV;
      len = 0;
      for(i=1; i<=rcv_len; i++) {
        transmitData[i] = uart_get_char();
        len++;
      }
      return len+1;
      break;
    case CMD_STATUS:
      transmitData[0] = CMD_STATUS;
      transmitData[1] = status;
      return 2;
      break;
    case CMD_RCV_SIZE:
      transmitData[0] = CMD_RCV_SIZE;
      transmitData[1] = uart_rx_size();
      return 2;
      break;
    case CMD_CTS:
      transmitData[0] = CMD_CTS;
      transmitData[1] = (CTS_PORT & BV(CTS_PIN)) ? 0 : 1;
      return 2;
      break;
    default:
      CMD = CMD_NULL;
      return 0;
  }
}

/******************************************************/