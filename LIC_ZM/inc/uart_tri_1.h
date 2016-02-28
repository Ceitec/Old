#ifndef UART_TRI_1_H
#define UART_TRI_1_H

#include "uart_types.h"

#define UART1_BUFFER_LINEAR_SIZE (0x20)
#define UART1_BUFFER_PACKET_SIZE (0x10)
#define UART1_TIMEOUT (5)
#define UART1_DEFAULT_BAUD 38400
#define UART1_TX_TIMEOUT (50)

#define UART1_PROC_UDR      UDR1
#define UART1_PROC_UCSRB    UCSR1B
#define UART1_PROC_UCSRA    UCSR1A
#define UART1_PROC_UBRRL    UBRR1L
#define UART1_PROC_RXCIE    RXCIE1
#define UART1_PROC_TXCIE    TXCIE1
#define UART1_PROC_RX_vect  USART1_RX_vect
#define UART1_PROC_TX_vect  USART1_TX_vect
#define UART1_PROC_TXEN     TXEN1
#define UART1_PROC_RXEN     RXEN1

#define UART1_TX_ENA  PORTD |= BV(PD4);
#define UART1_TX_DIS  PORTD &= ~(BV(PD4));

#define UART1_TX_DIR  DDRD |= BV(PD4);

extern volatile byte uart1_status;
extern volatile Tuartflags uart1_flags;

#define UART1_BUFFER_LINEAR_SIZE_MAX (UART1_BUFFER_LINEAR_SIZE-1)
#define UART1_BUFFER_PACKET_SIZE_MAX (UART1_BUFFER_PACKET_SIZE-1)

/*
 * Perform UART startup initialization.
 */
void uart1_init(void);
void uart1_process(void);
void uart1_set_baud(byte baud);
void uart1_ISR_timer(void);
byte * uart1_get_data_begin(void);
byte uart1_get_data_end(void);
void uart1_put_data(byte * dataptr);

#endif
