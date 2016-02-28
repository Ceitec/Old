#ifndef UART_TRI_H
#define UART_TRI_H

#include <stdbool.h>

#define UART0_BUFFER_LINEAR_SIZE (0x20)
#define UART0_BUFFER_PACKET_SIZE (0x10)
#define UART0_TIMEOUT (5)
#define UART0_DEFAULT_BAUD 9600
#define UART0_TX_TIMEOUT (200)

#define UART0_PROC_UDR UDR
#define UART0_PROC_UCSRB UCSRB
#define UART0_PROC_UCSRA UCSRA
#define UART0_PROC_UBRRL UBRRL

#define UART0_TX_ENA  PORTD |= BV(PD2);
#define UART0_TX_DIS  PORTD &= ~(BV(PD2));


typedef union {
    struct {
      byte addr;
      byte cmd;
      byte type;
      byte motor;
      union {
        dword dw;
        struct {
          byte b0;
          byte b1;
          byte b2;
          byte b3;
        } b;
      } val;
      byte sum;
    } n;
    byte b[9];
} Ttripac;

typedef struct {
  byte txing : 1; // data transmitting
  byte rxing : 1; // data receiving
  byte wait_tx : 1; // waiting for response
  byte data_received : 1; // valid data received
  byte data_receive_error : 1; // some received data was not valid

} Tuartflags;

/*
    d1addr: byte;
    d2command: byte;
    d3typ: byte;
    d4motor: byte;
    d5value: Cardinal;
*/
/*
extern volatile byte uart0_buf_rx[UART0_BUFFER_LINEAR_SIZE];
extern volatile byte uart0_buf_tx[UART0_BUFFER_LINEAR_SIZE];
extern volatile byte uart0_buf_rx_ptr_b;
extern volatile byte uart0_buf_rx_ptr_e;
extern volatile byte uart0_buf_tx_ptr_b;
extern volatile byte uart0_buf_tx_ptr_e;
extern volatile byte vysilani0;
extern volatile byte prijato0;
extern volatile byte BusTimeOut0;
*/
extern volatile byte uart0_status;

#define UART0_BUFFER_LINEAR_SIZE_MAX (UART0_BUFFER_LINEAR_SIZE-1)
#define UART0_BUFFER_PACKET_SIZE_MAX (UART0_BUFFER_PACKET_SIZE-1)

/*
 * Perform UART startup initialization.
 */
void uart0_init(void);
void uart_interrupt_rx(unsigned char enable);
void uart_interrupt_tx(unsigned char enable);
void uart_send(void);
byte uart_tx(byte dat);
void uart_ISR_timer(void);

void uart_put_char(char dat);
char uart_get_char(void);

byte uart_rx_size(void);
byte uart_tx_size(void);
bool uart_rx_empty(void);
bool uart_tx_empty(void);
void uart_tx_flush(void);
void uart_rx_flush(void);


#endif
