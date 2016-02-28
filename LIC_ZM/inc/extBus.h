#ifndef EXTBUS_H_
#define EXTBUS_H_
/*
 * extBus.h
 *
 * Created: 9.7.2014 17:44:38
 *  Author: Michal
 */

#include <stdbool.h>
#include "uart_types.h"

#define UART2_BUFFER_LINEAR_SIZE (0x20)
#define UART2_BUFFER_PACKET_SIZE (0x10)
#define UART2_TIMEOUT (5)
#define UART2_DEFAULT_BAUD 38400
#define UART2_TX_TIMEOUT (50)

extern volatile byte uart2_status;
extern volatile Tuartflags uart2_flags;

#define UART2_BUFFER_LINEAR_SIZE_MAX (UART2_BUFFER_LINEAR_SIZE-1)
#define UART2_BUFFER_PACKET_SIZE_MAX (UART2_BUFFER_PACKET_SIZE-1)
/*
extern volatile byte buf2_rx[UART2_BUFFER_SIZE];
extern volatile byte buf2_tx[UART2_BUFFER_SIZE];
extern volatile byte buf2_rx_ptr_b;
extern volatile byte buf2_rx_ptr_e;
extern volatile byte buf2_tx_ptr_b;
extern volatile byte buf2_tx_ptr_e;
extern volatile byte BusTimeOut2;
*/

extern byte dbg;

/*
 * Perform UART startup initialization.
 */
void uart2_init(void);
void uart2_process(void);
void uart2_set_baud(byte baud);
void uart2_ISR_timer(void);
byte * uart2_get_data(void);
void uart2_put_data(byte * dataptr);
/*
byte uart2_rx_size(void);
byte uart2_tx_size(void);
bool uart2_rx_empty(void);
bool uart2_tx_empty(void);
void uart2_tx_flush(void);
void uart2_rx_flush(void);

// debug
void uart2_receive_packet_query(void);
*/

/* ext module commands */
#define CMD_NULL	0	// nic
#define CMD_IDENT	1	// get identification string
#define CMD_SEND	2	// send serial data
#define CMD_RCV		3	// receive serial data
#define CMD_STATUS	4	// get status
#define CMD_RCV_SIZE	5	// get Rx buffer length
#define CMD_BAUD_RATE	6	// set baud rate
#define CMD_RTS		7	// set RTS signal state
#define CMD_CTS		8	// get CTS signal state
#define CMD_FLUSH	9 	// flush Rx buffer


#endif /* EXTBUS_H_ */