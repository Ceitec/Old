#include "common_defs.h"
#include "defines.h"
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart_tri_0.h"

// linear char buffers
volatile byte uart0_buf_rx[UART0_BUFFER_LINEAR_SIZE];
volatile byte uart0_buf_rx_ptr_b = 0;
volatile byte uart0_buf_rx_ptr_e = 0;
volatile byte uart0_buf_tx[9];
volatile byte uart0_buf_tx_ptr = 0; // fixed 0 - 8

// packet buffers
Ttripac uart0_buf_pac_tx[UART0_BUFFER_PACKET_SIZE];
byte uart0_buf_pac_ptr_b = 0;
byte uart0_buf_pac_ptr_e = 0;
Ttripac uart0_buf_pac_rx;


//volatile byte uart0_vysilani = false;
//volatile byte uart0_prijato = false;
volatile byte uart0_rx_timeout = 0; // kontrola návaznosti pøíchozích dat
volatile byte uart0_tx_timeout = 0; // mezera mezi rámci
volatile byte uart0_status;
volatile Tuartflags uart0_flags;
// buf_rx - linear (cyclic) receive buffer
// empty    -> b = e
// length   ->   = (e - b) & UART_BUFFER_SIZE
// free spc ->   = (b - e) & UART_BUFFER_SIZE
// full     -> b = (e + 1) & UART_BUFFER_SIZE
// write    -> e++, write *e
// read     -> b++, read *b

/******************************************************/
// buffer logic
/******************************************************/

//----------------------------------------------------------
// get single char from RX linear buffer
char uart_get_char()
{
  // read     -> b++, read *b
  // interrupt safe -> read *(b+1); b++
  byte ptr;
  byte res;

  ptr = (uart0_buf_rx_ptr_b+1) & UART0_BUFFER_LINEAR_SIZE_MAX;
  res = uart0_buf_rx[ptr];
  uart0_buf_rx_ptr_b = ptr;
  return res;
}

//----------------------------------------------------------
// put single char in RX linear buffer
void uart_receive_char(char dat)
{
  // write    -> e++, write *e
  // interrupt safe -> write *(e+1); e++
  byte ptr;

  ptr = (uart0_buf_rx_ptr_e+1) & UART0_BUFFER_LINEAR_SIZE_MAX;
  uart0_buf_rx[ptr] = dat;
  uart0_buf_rx_ptr_e = ptr;
}

//----------------------------------------------------------
// get single char from TX buffer (frame)
char uart_send_char(void)
{
  byte ptr;
  byte res;

  ptr = uart0_buf_tx_ptr;
  res = uart0_buf_tx[ptr];
  uart0_buf_tx_ptr = ptr+1;
  return res;
}

//----------------------------------------------------------



//----------------------------------------------------------

//----------------------------------------------------------


//----------------------------------------------------------
/******************************************************/
// start sending TX buffer
/******************************************************/
void uart_send(void)
{
  if (uart0_flags.txing == false) {
    // is some data in buffer ?
    if (!(uart0_flags.txing)) {
      uart0_flags.txing = true;
      uart0_tx_timeout = UART0_TX_TIMEOUT;
      UART0_TX_ENA;  // tx mode
      uart0_buf_tx_ptr = 0; // send first byte from buffer
      UART0_PROC_UDR = uart_send_char();
    }
  }
}

/******************************************************/
// Useful functions
/******************************************************/
inline byte uart_rx_size(void)
{
  return (uart0_buf_rx_ptr_e - uart0_buf_rx_ptr_b);
}

inline byte uart_tx_size(void)
{
  return 9;
}

inline bool uart_rx_empty(void)
{
  return (uart0_buf_rx_ptr_e == uart0_buf_rx_ptr_b);
}

inline void uart_tx_flush(void)
{
  uart0_buf_tx_ptr = 0;
}

inline void uart_rx_flush(void)
{
  uart0_buf_rx_ptr_b = uart0_buf_rx_ptr_e;
}

inline bool uart_pac_empty(void)
{
  return (uart0_buf_pac_ptr_e == uart0_buf_pac_ptr_b);
}


/******************************************************/
// Enable/disable interrupts
/******************************************************/
void uart_interrupt_rx(byte enable)
{
  if (enable)
    UART0_PROC_UCSRB |= BV(RXCIE);
   else
    UART0_PROC_UCSRB &= ~BV(RXCIE);
}

void uart_interrupt_tx(byte enable)
{
  if (enable)
    UART0_PROC_UCSRB |= BV(TXCIE);
   else
    UART0_PROC_UCSRB &= ~BV(TXCIE);
}


/******************************
**         Interrupt         **
******************************/

/******************************************************/
//
/******************************************************/
ISR(USART_RXC_vect)
{
  byte tmpDat;
  byte tmpStatus;

  uart0_rx_timeout = UART0_TIMEOUT;
  tmpStatus = UART0_PROC_UCSRA;
  uart0_status |= tmpStatus;
  tmpDat = UART0_PROC_UDR;
  uart_receive_char(tmpDat);
  //buf0_rx_ptr_e++;
  //buf0_rx[buf0_rx_ptr_e] = tmpDat;
}

/******************************************************/
//
/******************************************************/
ISR(USART_TXC_vect)
{
  byte tmpDat;

  // ???
  if (uart0_flags.txing == false) return;

  // is next data in buffer?
  if (uart0_buf_tx_ptr > 8) {
    // whole buffer was sended
    uart0_flags.txing = FALSE;
    // if whole packed was send, wait for response
    uart0_flags.wait_tx = TRUE;
    UART0_TX_DIS;   // rx mode
    return;
  }

  // send next byte
  tmpDat = uart_send_char();
  UART0_PROC_UDR = tmpDat;
}


/******************************************************/
// External Function
/******************************************************/

//----------------------------------------------------------
// Initialization
void uart0_init(void)
{
  // UART port

  #if F_CPU < 2000000UL && defined(U2X)
  UART0_PROC_UCSRA = _BV(U2X);             /* improve baud rate error by using 2x clk */
  UART0_PROC_UBRRL = (F_CPU / (8UL * UART0_DEFAULT_BAUD)) - 1;
  #else
  UART0_PROC_UBRRL = (F_CPU / (16UL * UART0_DEFAULT_BAUD)) - 1;

  #endif
  UART0_PROC_UCSRB = BV(TXEN) | BV(RXEN); /* tx/rx enable */

  uart_interrupt_rx(true);
  uart_interrupt_tx(true);
  uart0_status = 0;
}

//----------------------------------------------------------
// process internal logic
void uart0_process(void)
{
  byte i;
  byte sum;
  byte *ptr;

  // pøedává zpravy na odvysílání z paketového do lineárního bufferu
  if (!uart_pac_empty()) {
    // jsou data k odesláni ?
    if ((!uart0_flags.txing) && (!uart0_flags.wait_tx)) {
      // nevysíláme ani neèekáme na odpoveï ?
      // zaèneme vysílat další zprávu
      i = ++uart0_buf_pac_ptr_b;
      ptr = (byte *) &(uart0_buf_pac_tx[uart0_buf_pac_ptr_b]);
      for (i=0; i<9; i++) {
        uart0_buf_tx[i] = *ptr;
        ptr++;
      }
      uart0_buf_tx_ptr = 0;
      uart_send();
    }
  }

  // kontroluje pøijatá data
  if ((!uart0_flags.data_received) && (uart_tx_size() > 8)) {
    // máme alespoò 9 bytù dat a nejsou nezpracovaná data?

    // pøedáme do paketového pøijímacího bufferu
    for(i=0; i<9; i++) {
      uart0_buf_pac_rx.b[i] = uart_get_char();
    }

    // odpovídá kontrolní souèet?
    sum = 0;
    for(i=0; i<8; i++) {
      sum += uart0_buf_pac_rx.b[i];
    }
    if (sum == uart0_buf_pac_rx.b[8]) {
      // souèet v poøádku
      uart0_flags.data_received = TRUE;
      uart0_flags.wait_tx = FALSE; // odpoveï pøišla
     } else {
      uart0_flags.data_receive_error = TRUE;
    }
  }

}

//----------------------------------------------------------
// timer function
void uart0_ISR_timer(void)
{
  // pauza za odeslanými daty (nepøišla odpovìï)
  if (uart0_flags.wait_tx) {
    uart0_tx_timeout--;
    if (uart0_tx_timeout == 0) {
      uart0_flags.wait_tx = FALSE;
    }
  }

  // smazání náhodnì pøijatých dat
  if (uart0_rx_timeout > 0) {
    uart0_rx_timeout--;
    } else {
    uart0_buf_rx_ptr_b = uart0_buf_rx_ptr_e;
  }
}
