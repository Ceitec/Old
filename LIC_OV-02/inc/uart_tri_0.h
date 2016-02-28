#ifndef __UART_TRI_0_H
#define __UART_TRI_0_H

//#include <stdbool.h>
#include "uart_types.h"

#define UART0_BUFFER_LINEAR_SIZE (0x10)
#define UART0_BUFFER_PACKET_SIZE (0x10)
#define UART0_TIMEOUT (5)
#define UART0_DEFAULT_BAUD 115200
#define UART0_TX_TIMEOUT (200)

#define UART0_PROC_UDR      UDR
#define UART0_PROC_UCSRB    UCSRB
#define UART0_PROC_UCSRA    UCSRA
#define UART0_PROC_UBRRL    UBRRL
#define UART0_PROC_RXCIE    RXCIE
#define UART0_PROC_TXCIE    TXCIE
#define UART0_PROC_RX_vect  USART_RXC_vect
#define UART0_PROC_TX_vect  USART_TXC_vect
#define UART0_PROC_TXEN     TXEN
#define UART0_PROC_RXEN     RXEN

#define UART0_TX_ENA  PORTD |= BV(PD2);
#define UART0_TX_DIS  PORTD &= ~(BV(PD2));


extern volatile byte uart0_status;
extern volatile Tuartflags uart0_flags;

#define UART0_BUFFER_LINEAR_SIZE_MAX (UART0_BUFFER_LINEAR_SIZE-1)
#define UART0_BUFFER_PACKET_SIZE_MAX (UART0_BUFFER_PACKET_SIZE-1)

/*
 * Perform UART startup initialization.
 */
void uart0_init(void);
void uart0_process(void);
void uart0_set_baud(byte baud);
void uart0_ISR_timer(void);
byte * uart0_get_data_begin(void);
byte uart0_get_data_end(void);
void uart0_put_data(byte * dataptr);
byte uart_pac_rx_empty(void);
byte uart_pac_rx_size(void);

#endif
