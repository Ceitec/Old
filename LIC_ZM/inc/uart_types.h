#ifndef UART_TRI_TYPES_H
#define UART_TRI_TYPES_H

typedef struct {
  byte txing : 1; // data transmitting
  byte rxing : 1; // data receiving
  byte wait_tx : 1; // waiting for response
  byte data_received : 1; // valid data received
  byte data_receive_error : 1; // some received data was not valid

} Tuartflags;

typedef struct {
  byte buf_tx_lin_over : 1; // buf tx linear overrun
  byte buf_tx_lin_under : 1; // buf_tx_linear underrun 
  byte buf_rx_lin_over : 1; // buf tx linear overrun
  byte buf_rx_lin_under : 1; // buf_tx_linear underrun
  byte buf_tx_pac_over : 1; // buf tx packet overrun
  byte buf_tx_pac_under : 1; // buf_tx_packet underrun 
  byte buf_rx_pac_over : 1; // buf tx packet overrun
  byte buf_rx_pac_under : 1; // buf_tx_packet underrun

} Tuartflags_buferr;

#endif