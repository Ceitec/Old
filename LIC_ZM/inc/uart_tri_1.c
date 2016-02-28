#include "common_defs.h"
#include "defines.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Tribus_types.h"
#include "uart_types.h"
#include "uart_tri_1.h"

// linear char buffers
volatile byte uart1_buf_rx[UART1_BUFFER_LINEAR_SIZE];
volatile byte uart1_buf_rx_ptr_b = 0;
volatile byte uart1_buf_rx_ptr_e = 0;
volatile byte uart1_buf_tx[9];
volatile byte uart1_buf_tx_ptr = 0; // fixed 0 - 8

// packet buffers
Ttripac uart1_buf_pac_tx[UART1_BUFFER_PACKET_SIZE];
byte uart1_buf_pac_tx_ptr_b = 0;
byte uart1_buf_pac_tx_ptr_e = 0;
Ttripac uart1_buf_pac_rx[UART1_BUFFER_PACKET_SIZE];
byte uart1_buf_pac_rx_ptr_b = 0;
byte uart1_buf_pac_rx_ptr_e = 0;

volatile byte uart1_rx_timeout = 0; // kontrola návaznosti pøíchozích dat
volatile byte uart1_tx_timeout = 0; // mezera mezi rámci
volatile byte uart1_status = 0; // raw hardware UART status latch
volatile Tuartflags uart1_flags = {0}; // status flags
volatile Tuartflags_buferr uart1_flags_buferr = {0}; // buffers error flags
// buf_rx - linear (cyclic) receive buffer
// empty    -> b = e
// length   ->   = (e - b) & UART_BUFFER_SIZE
// free spc ->   = (b - e) & UART_BUFFER_SIZE
// full     -> b = (e + 1) & UART_BUFFER_SIZE
// write    -> e++, write *e
// read     -> b++, read *b

/******************************************************/
// Useful functions
/******************************************************/
inline byte uart1_rx_size(void)
{
  return ((uart1_buf_rx_ptr_e - uart1_buf_rx_ptr_b) & UART1_BUFFER_LINEAR_SIZE_MAX);
}

inline byte uart1_buf_tx_size(void)
{
  return 9;
}
/*
inline byte uart1_tx_empty(void)
{
  return (uart1_buf_tx_ptr_e == uart1_buf_tx_ptr_b);
}
*/
inline byte uart1_rx_empty(void)
{
  return (uart1_buf_rx_ptr_e == uart1_buf_rx_ptr_b);
}
/*
inline byte uart1_tx_full(void)
{
  return ((uart1_buf_tx_ptr_e + 1) & UART1_BUFFER_LINEAR_SIZE_MAX) == uart1_buf_tx_ptr_b);
}
*/
inline byte uart1_rx_full(void)
{
  return (((uart1_buf_rx_ptr_e + 1) & UART1_BUFFER_LINEAR_SIZE_MAX) == uart1_buf_rx_ptr_b);
}

inline void uart1_tx_flush(void)
{
  uart1_buf_tx_ptr = 0;
}

inline void uart1_rx_flush(void)
{
  uart1_buf_rx_ptr_b = uart1_buf_rx_ptr_e;
}

inline byte uart1_pac_tx_empty(void)
{
  return (uart1_buf_pac_tx_ptr_e == uart1_buf_pac_tx_ptr_b);
}

inline byte uart1_pac_rx_empty(void)
{
  return (uart1_buf_pac_rx_ptr_e == uart1_buf_pac_rx_ptr_b);
}

inline byte uart1_pac_tx_size(void)
{
  return (uart1_buf_pac_tx_ptr_e - uart1_buf_pac_tx_ptr_b) & UART1_BUFFER_PACKET_SIZE_MAX;
}

inline byte uart1_pac_rx_size(void)
{
  return (uart1_buf_pac_rx_ptr_e - uart1_buf_pac_rx_ptr_b) & UART1_BUFFER_PACKET_SIZE_MAX;
}

inline byte uart1_pac_tx_full(void)
{
  return (((uart1_buf_pac_tx_ptr_e + 1) & UART1_BUFFER_PACKET_SIZE_MAX) ==  uart1_buf_pac_tx_ptr_b);
}

inline byte uart1_pac_rx_full(void)
{
  return (((uart1_buf_pac_rx_ptr_e + 1) & UART1_BUFFER_PACKET_SIZE_MAX) ==  uart1_buf_pac_rx_ptr_b);
}

/******************************************************/
// buffer logic
/******************************************************/

//----------------------------------------------------------
// get single char from RX linear buffer
char uart1_get_char(void)
{
  // interrupt safe -> read *(b+1); b++
  byte ptr;
  byte res;
  
  if (uart1_rx_empty()) {
    // v bufferu nic není
    uart1_flags_buferr.buf_rx_lin_under = true;
    return 0; 
   } else {
    ptr = (uart1_buf_rx_ptr_b+1) & UART1_BUFFER_LINEAR_SIZE_MAX;
    res = uart1_buf_rx[ptr];
    uart1_buf_rx_ptr_b = ptr;
    return res;
  }
}

//----------------------------------------------------------
// put single char in RX linear buffer
void uart1_receive_char(char dat)
{
  // interrupt safe -> write *(e+1); e++
  byte ptr;
  
  if (uart1_rx_full()) {
    // není kam pøíjímat !
    uart1_flags_buferr.buf_rx_lin_over = true;
   } else {
    ptr = (uart1_buf_rx_ptr_e+1) & UART1_BUFFER_LINEAR_SIZE_MAX;
    uart1_buf_rx[ptr] = dat;
    uart1_buf_rx_ptr_e = ptr;
  }
}

//----------------------------------------------------------
// get single char from TX buffer (frame)
char uart1_send_char(void)
{
  byte ptr;
  byte res;
  
  if (uart1_buf_tx_ptr > 8) {
    // není co vysílat !
    uart1_flags_buferr.buf_tx_lin_under = true;
    return 0;
   } else {
    // vrátí byte k odeslání
    ptr = uart1_buf_tx_ptr;
    res = uart1_buf_tx[ptr];
    uart1_buf_tx_ptr++;
    return res; 
  }
}

//----------------------------------------------------------



//----------------------------------------------------------

//----------------------------------------------------------


//----------------------------------------------------------
/******************************************************/
// start sending TX buffer
/******************************************************/
void uart1_send(void)
{
  if (uart1_flags.txing == false) {
    // is some data in buffer ?
    uart1_flags.txing = true;
    UART1_PROC_UCSRB &= ~BV(UART1_PROC_RXEN);
    UART1_TX_ENA;  // tx mode
    uart1_buf_tx_ptr = 0; // send first byte from buffer
    UART1_PROC_UDR = uart1_send_char();
  }
}

/******************************************************/
// Enable/disable interrupts
/******************************************************/
void uart1_interrupt_rx(byte enable)
{
  if (enable)
    UART1_PROC_UCSRB |= BV(UART1_PROC_RXCIE);
   else
    UART1_PROC_UCSRB &= ~BV(UART1_PROC_RXCIE);
}

void uart1_interrupt_tx(byte enable)
{
  if (enable)
    UART1_PROC_UCSRB |= BV(UART1_PROC_TXCIE);
   else
    UART1_PROC_UCSRB &= ~BV(UART1_PROC_TXCIE);
}


/******************************
**         Interrupt         **
******************************/

/******************************************************/
//
/******************************************************/
ISR(UART1_PROC_RX_vect)
{
  byte tmpDat;
  byte tmpStatus;

  uart1_rx_timeout = UART1_TIMEOUT;
  tmpStatus = UART1_PROC_UCSRA;
  uart1_status |= tmpStatus;
  tmpDat = UART1_PROC_UDR;
  uart1_receive_char(tmpDat);
}

/******************************************************/
//
/******************************************************/
ISR(UART1_PROC_TX_vect)
{
  byte tmpDat;

  // ???
  if (uart1_flags.txing == false) return;

  // is next data in buffer?
  if (uart1_buf_tx_ptr > 8) {
    // whole buffer was sended
    uart1_flags.txing = FALSE;
    // if whole packed was send, wait for response
    uart1_tx_timeout = UART1_TX_TIMEOUT;
    uart1_flags.wait_tx = TRUE;
    UART1_TX_DIS;   // rx mode
    UART1_PROC_UCSRB |= BV(UART1_PROC_RXEN);
    return;
  }

  // send next byte
  tmpDat = uart1_send_char();
  UART1_PROC_UDR = tmpDat;
}


/******************************************************/
// External Function
/******************************************************/

//----------------------------------------------------------
// Initialization
void uart1_init(void)
{
  // UART port
  //UART1_TX_DIR;
  //DDRD |= BV(PD3);

  UART1_PROC_UBRRL = (F_CPU / (16UL * UART1_DEFAULT_BAUD)) - 1;

  UART1_PROC_UCSRB |= BV(UART1_PROC_TXEN) | BV(UART1_PROC_RXEN); /* tx/rx enable */

  uart1_interrupt_rx(true);
  uart1_interrupt_tx(true);
  uart1_status = 0;
}

//----------------------------------------------------------
// process internal logic
void uart1_process(void)
{
  byte i;
  byte sum;
  byte *ptr;
  byte iptr;

  // pøedává zpravy na odvysílání z paketového do lineárního bufferu
  if (!uart1_pac_tx_empty()) {
    // jsou data k odesláni ?
    if ((!uart1_flags.txing) && (!uart1_flags.wait_tx)) {
      // nevysíláme ani neèekáme na odpoveï ?
      // zaèneme vysílat další zprávu
      i = (uart1_buf_pac_tx_ptr_b+1) & UART1_BUFFER_PACKET_SIZE_MAX;
      ptr = (byte *) &(uart1_buf_pac_tx[i]);
      uart1_buf_pac_tx_ptr_b = i;
      for (i=0; i<9; i++) {
        uart1_buf_tx[i] = *ptr;
        ptr++;
      }
      uart1_buf_tx_ptr = 0;
      uart1_send();
    }
  }

  // kontroluje pøijatá data
  if ((!uart1_flags.data_received) && (uart1_rx_size() > 8)) {
    // máme alespoò 9 bytù dat a nejsou nezpracovaná data?

    // pøedáme do paketového pøijímacího bufferu
    iptr = (uart1_buf_pac_rx_ptr_e + 1) & UART1_BUFFER_PACKET_SIZE_MAX;
    for(i=0; i<9; i++) {
      uart1_buf_pac_rx[iptr].b[i] = uart1_get_char();
    }
    uart1_buf_pac_rx_ptr_e = iptr;

    // odpovídá kontrolní souèet?
    sum = 0;
    for(i=0; i<8; i++) {
      sum += uart1_buf_pac_rx[iptr].b[i];
    }
    if (sum == uart1_buf_pac_rx[iptr].b[8]) {
      // souèet v poøádku
      uart1_flags.data_received = TRUE;
      uart1_flags.wait_tx = FALSE; // odpoveï pøišla
      uart1_buf_pac_rx_ptr_e = iptr;
     } else {
      uart1_flags.data_receive_error = TRUE;
    }
    /*
    // DEBUG !!!
    uart1_flags.data_received = TRUE;
    uart1_flags.wait_tx = FALSE; // odpoveï pøišla
    */
  }

}

//----------------------------------------------------------
// timer function
void uart1_ISR_timer(void)
{
  // pauza za odeslanými daty (nepøišla odpovìï)
  if (uart1_flags.wait_tx) {
    uart1_tx_timeout--;
    if (uart1_tx_timeout == 0) {
      uart1_flags.wait_tx = FALSE;
    }
  }

  // smazání náhodnì pøijatých dat
  if (uart1_rx_timeout > 0) {
    uart1_rx_timeout--;
    } else {
    uart1_buf_rx_ptr_b = uart1_buf_rx_ptr_e;
  }
}

//----------------------------------------------------------
// get pointer to received data
// must be called uart0_get_data_end() at end of handling data
byte * uart1_get_data_begin(void)
{
  byte iptr;
  
  if (uart1_pac_rx_empty()) {
    // není co pøedat !
    uart1_flags_buferr.buf_rx_pac_under = true;
    return 0;
   } else {
    iptr = (uart1_buf_pac_rx_ptr_b+1) & UART1_BUFFER_PACKET_SIZE_MAX;
    return (byte *) &uart1_buf_pac_rx[iptr].b[0];
  }
}

//----------------------------------------------------------
// clear data_received flag and return number of pending packets
// must be called after uart0_get_data_begin()
byte uart1_get_data_end(void)
{
  byte i;
  
  i = (uart1_buf_pac_rx_ptr_b+1) & UART1_BUFFER_PACKET_SIZE_MAX;
  uart1_buf_pac_rx_ptr_b = i;
  
  i = uart1_pac_rx_size();
  if (i == 0) {
    uart1_flags.data_received = FALSE;
  }
  return i;
}

//----------------------------------------------------------
// send packet pointed by dataptr
void uart1_put_data(byte * dataptr)
{
  byte i;
  byte j;
  byte sum;
  
  if (uart1_pac_tx_full()) {
    // není kam zapsat !
    uart1_flags_buferr.buf_tx_pac_over = true;
    return;
  }
  
  j = (uart1_buf_pac_tx_ptr_e+1) & UART1_BUFFER_PACKET_SIZE_MAX;

  sum = 0;
  // copy data without sum
  for(i=0; i<8; i++) {
    uart1_buf_pac_tx[j].b[i] = *dataptr;
    sum += *dataptr;
    dataptr++;
  }
  uart1_buf_pac_tx[j].n.sum = sum; // save calculated sum

  uart1_buf_pac_tx_ptr_e = j;
}

#define MACRO_BAUDRATE(BAUDRATE) (UART1_PROC_UBRRL = (F_CPU / (16UL * BAUDRATE)) - 1)

//----------------------------------------------------------
// Set from default baud rates
void uart1_set_baud(byte baud)
{
  // UART port
  switch (baud) {
    case 0:  
      MACRO_BAUDRATE(9600);
      break;
    case 1:  
      MACRO_BAUDRATE(14400);
      break;
    case 2:  
      MACRO_BAUDRATE(19200);
      break;
    case 3:  
      MACRO_BAUDRATE(28800);
      break;
    case 4:  
      MACRO_BAUDRATE(38400);
      break;
    case 5:  
      MACRO_BAUDRATE(57600);
      break;
    case 6:  
      MACRO_BAUDRATE(76800);
      break;
    case 7:  
      MACRO_BAUDRATE(115200);
      break;
    case 8:  
      MACRO_BAUDRATE(230400);
      break;
    case 9:  
      MACRO_BAUDRATE(250000);
      break;
    case 10:  
      MACRO_BAUDRATE(500000);
      break;
    default:  
      MACRO_BAUDRATE(19200);
      break;
  }
  return;
}
