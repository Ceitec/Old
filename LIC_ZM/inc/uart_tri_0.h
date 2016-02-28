#ifndef UART_TRI_0_H
#define UART_TRI_0_H

//#include <stdbool.h>
#include "uart_types.h"

#define UART0_BUFFER_LINEAR_SIZE (0x80)
#define UART0_BUFFER_PACKET_SIZE (0x20)
#define UART0_TIMEOUT (5)
#define UART0_DEFAULT_BAUD 115200
#define UART0_TX_TIMEOUT (200)

#define UART0_PROC_UDR      UDR0
#define UART0_PROC_UCSRB    UCSR0B
#define UART0_PROC_UCSRA    UCSR0A
#define UART0_PROC_UBRRL    UBRR0L
#define UART0_PROC_RXCIE    RXCIE0
#define UART0_PROC_TXCIE    TXCIE0
#define UART0_PROC_RX_vect  USART0_RX_vect
#define UART0_PROC_TX_vect  USART0_TX_vect
#define UART0_PROC_TXEN     TXEN0
#define UART0_PROC_RXEN     RXEN0

#define UART0_TX_ENA  ;
#define UART0_TX_DIS  ;

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

#endif
