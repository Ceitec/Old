/*
 * extBus.c
 *
 * Created: 9.7.2014 17:44:25
 *  Author: Michal
 */


#include "common_defs.h"
#include "defines.h"
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "i2cconf.h"
#include "i2c.h"
#include "uart_types.h"
#include "Tribus_types.h"
#include "extBus.h"

byte dbg;

// UART variables

// linear char buffers
volatile byte uart2_buf_rx[UART2_BUFFER_LINEAR_SIZE];
volatile byte uart2_buf_rx_ptr_b = 0;
volatile byte uart2_buf_rx_ptr_e = 0;
volatile byte uart2_buf_tx[9];
volatile byte uart2_buf_tx_ptr = 0; // fixed 0 - 8

// packet buffers
Ttripac uart2_buf_pac_tx[UART2_BUFFER_PACKET_SIZE];
byte uart2_buf_pac_ptr_b = 0;
byte uart2_buf_pac_ptr_e = 0;
Ttripac uart2_buf_pac_rx;

volatile byte uart2_rx_timeout = 0; // kontrola návaznosti pøíchozích dat
volatile byte uart2_tx_timeout = 0; // mezera mezi rámci
volatile byte uart2_status = 0;
volatile Tuartflags uart2_flags = {0};
// buf_rx - linear (cyclic) receive buffer
// empty    -> b = e
// length   ->   = (e - b) & UART_BUFFER_SIZE
// free spc ->   = (b - e) & UART_BUFFER_SIZE
// full     -> b = (e + 1) & UART_BUFFER_SIZE
// write    -> e++, write *e
// read     -> b++, read *b

byte ext_pending_data = 0;

byte ext_buffer[10]; // 9 Trinamic + 1 ext command

/******************************************************/
// buffer logic
/******************************************************/

//----------------------------------------------------------
// get single char from RX linear buffer
char uart2_get_char(void)
{
  // read     -> b++, read *b
  // interrupt safe -> read *(b+1); b++
  byte ptr;
  byte res;

  ptr = (uart2_buf_rx_ptr_b+1) & UART2_BUFFER_LINEAR_SIZE_MAX;
  res = uart2_buf_rx[ptr];
  uart2_buf_rx_ptr_b = ptr;
  return res;
}

//----------------------------------------------------------
// put single char in RX linear buffer
void uart2_receive_char(char dat)
{
  // write    -> e++, write *e
  // interrupt safe -> write *(e+1); e++
  byte ptr;

  ptr = (uart2_buf_rx_ptr_e+1) & UART2_BUFFER_LINEAR_SIZE_MAX;
  uart2_buf_rx[ptr] = dat;
  uart2_buf_rx_ptr_e = ptr;
}

//----------------------------------------------------------
// get single char from TX buffer (frame)
char uart2_send_char(void)
{
  byte ptr;
  byte res;

  ptr = uart2_buf_tx_ptr;
  res = uart2_buf_tx[ptr];
  uart2_buf_tx_ptr = ptr+1;
  return res;
}

//----------------------------------------------------------



byte * uart2_get_data(void)
{
  return (byte *) &uart2_buf_pac_rx.b[0];
}

void uart2_put_data(byte * dataptr)
{
  byte i;
  byte j;
  byte sum;
  
  j = (uart2_buf_pac_ptr_e+1) & UART2_BUFFER_PACKET_SIZE_MAX;

  sum = 0;
  // copy data without sum
  for(i=0; i<8; i++) {
    uart2_buf_pac_tx[j].b[i] = *dataptr;
    sum += *dataptr;
    dataptr++;
  }
  uart2_buf_pac_tx[j].n.sum = sum; // save calculated sum

  uart2_buf_pac_ptr_e = j;
}

/******************************************************/
// Initialize the UART
/******************************************************/
void uart2_init(void)
{
  // ext UART port via I2C
  i2cInit();
  uart2_status = 0;
}


/******************************************************/
// put single char to TX buffer
/******************************************************/
/*
void uart2_put_char(char dat)
{
  // write    -> e++, write *e
  // interrupt safe -> write *(e+1); e++
  byte *ptr;

  asm volatile (
  "ldi %B0, hi8(buf2_tx)"  "\n\t"
  "lds %A0, buf2_tx_ptr_e" "\n\t" // combine base and offset address
  "inc %A0"                "\n\t" // increment address
  "st %a0, %1"             "\n\t" // load
  "sts buf2_tx_ptr_e, %A0" "\n\t" // store new address
  : "=e" (ptr)
  : "r" (dat)
  );
}
*/
/******************************************************/
// Useful functions
/******************************************************/
inline byte uart2_rx_size(void)
{
  return ((uart2_buf_rx_ptr_e - uart2_buf_rx_ptr_b) & UART2_BUFFER_LINEAR_SIZE_MAX);
}

inline byte uart2_tx_size(void)
{
  return 9;
}

inline byte uart2_rx_empty(void)
{
  return (uart2_buf_rx_ptr_e == uart2_buf_rx_ptr_b);
}

inline void uart2_tx_flush(void)
{
  uart2_buf_tx_ptr = 0;
}

inline void uart2_rx_flush(void)
{
  uart2_buf_rx_ptr_b = uart2_buf_rx_ptr_e;
}

inline byte uart2_pac_empty(void)
{
  return (uart2_buf_pac_ptr_e == uart2_buf_pac_ptr_b);
}


/******************************************************/
// start sending TX buffer to EXT module
/******************************************************/
void uart2_send_packet(void)
{
  byte i;

  while (uart2_tx_size() >= 9) {
    for(i=(0+1); i<(9+1); i++) {
      ext_buffer[i] = uart2_send_char();
    }
    ext_buffer[0] = CMD_SEND;
    i2cMasterSend(0x01, 10, ext_buffer);
  }
}

/******************************************************/
// test received data in EXT module
/******************************************************/
void uart2_receive_packet_query(void)
{
  ext_buffer[0] = CMD_RCV_SIZE;
  i2cMasterSend(1, 1, ext_buffer);
  _delay_us(400);
  if (i2cGetLastError() == I2C_NACK) {
    // module is not responding
    // ToDo: status change
    uart2_status |= BV(0);
   } else {
    // module ok
    i2cMasterReceive(1, 2, ext_buffer);
    if (ext_buffer[0] == CMD_RCV_SIZE) {
      ext_pending_data = ext_buffer[1];
     } else {
       // bad answer
       uart2_status |= BV(1);
      ext_pending_data = 0;
    }
  }
}

/******************************************************/
// receive from EXT module
/******************************************************/
byte uart2_receive_packet(void)
{
  byte i;

  // debug !!!!
  //ext_pending_data = 2;

  if (ext_pending_data>8) {
    ext_buffer[0] = CMD_RCV;
    ext_buffer[1] = 9; // length to send
    i2cMasterSend(1, 2, ext_buffer);
    _delay_us(400);
    if (i2cGetLastError() == I2C_NACK) {
      // module is not responding
      // ToDo: status change
      uart2_status |= BV(0);
     } else {
      // module ok
      i2cMasterReceive(1, 10, ext_buffer);
      if (ext_buffer[0] == CMD_RCV) {
        // answer ok
        for (i=(0+1); i<(9+1); i++) {
          uart2_receive_char(ext_buffer[i]);
        }
        ext_pending_data = 0;
       } else {
        // bad answer
        uart2_status |= BV(1);
        ext_pending_data = 0;
      }
    }
  }
  else if (ext_pending_data>0) {
    ext_buffer[0] = CMD_RCV;
    ext_buffer[1] = ext_pending_data; // length to send
    i2cMasterSend(1, 2, ext_buffer);
    _delay_us(400);
    if (i2cGetLastError() == I2C_NACK) {
      // module is not responding
      // ToDo: status change
      uart2_status |= BV(0);
      } else {
      // module ok
      i2cMasterReceive(1, ext_pending_data+1, ext_buffer); // data len + cmd byte
      if (ext_buffer[0] == CMD_RCV) {
        // answer ok
        for (i=(0+1); i<(ext_pending_data+1); i++) {
          uart2_receive_char(ext_buffer[i]);
        }
        ext_pending_data = 0;
        } else {
        // bad answer
        uart2_status |= BV(1);
        ext_pending_data = 0;
      }
    }
  }
  return 0;
}

//----------------------------------------------------------
// process internal logic
void uart2_process(void)
{
  byte i;
  byte sum;
  byte *ptr;

  // pøedává zpravy na odvysílání z paketového do lineárního bufferu
  if (!uart2_pac_empty()) {
    // jsou data k odesláni ?
    if ((!uart2_flags.txing) && (!uart2_flags.wait_tx)) {
      // nevysíláme ani neèekáme na odpoveï ?
      // zaèneme vysílat další zprávu
      i = (uart2_buf_pac_ptr_b+1) & UART2_BUFFER_PACKET_SIZE_MAX;
      ptr = (byte *) &(uart2_buf_pac_tx[i]);
      uart2_buf_pac_ptr_b = i;
      for (i=0; i<9; i++) {
        uart2_buf_tx[i] = *ptr;
        ptr++;
      }
      uart2_buf_tx_ptr = 0;
      //uart2_send(); // automaticky v timer funkci
    }
  }

  // kontroluje pøijatá data
  if ((!uart2_flags.data_received) && (uart2_rx_size() > 8)) {
    // máme alespoò 9 bytù dat a nejsou nezpracovaná data?

    // pøedáme do paketového pøijímacího bufferu
    for(i=0; i<9; i++) {
      uart2_buf_pac_rx.b[i] = uart2_get_char();
    }

    // odpovídá kontrolní souèet?
    sum = 0;
    for(i=0; i<8; i++) {
      sum += uart2_buf_pac_rx.b[i];
    }
    /*
    if (sum == uart2_buf_pac_rx.b[8]) {
      // souèet v poøádku
      uart2_flags.data_received = TRUE;
      uart2_flags.wait_tx = FALSE; // odpoveï pøišla
     } else {
      uart2_flags.data_receive_error = TRUE;
    }
    */
    // DEBUG !!!
    uart2_flags.data_received = TRUE;
    uart2_flags.wait_tx = FALSE; // odpoveï pøišla
  }
}

/******************************
**          Timer            **
******************************/

void uart2_ISR_timer(void)
{
  static byte ISR_prescaler = 0;

  if (uart2_rx_timeout > 0) {
    uart2_rx_timeout--;
    } else {
    uart2_buf_rx_ptr_b = uart2_buf_rx_ptr_b;
  }

  if (ISR_prescaler > 9) {
    ISR_prescaler = 0;
   } else {
    ISR_prescaler++;
  }

  if (ISR_prescaler == 1) {
    uart2_send_packet();
  }
  if (ISR_prescaler == 5) {
    uart2_receive_packet_query();
  }
  if (ISR_prescaler == 6) {
    uart2_receive_packet();
  }

}