/*
 * module_ZM.c
 *
 * Created: 15.7.2014 8:50:34
 *  Author: Michal
 */

#include "common_defs.h"
#include "defines.h"
#include <stdbool.h>
#include <avr/io.h> // debug
#include "Tribus.h"
#include "Tribus_types.h"
#include "adc.h"

// debug variables from:
#include "debug_var.h"
#include "uart_tri_0.h"
#include "uart_tri_1.h"
#include "extBus.h"
#include "packet_processor.h"
#include <util/delay.h>

Ttripac debug_buf;


void Ddebug(void)
{
  debug_buf.n.addr = 20;
  debug_buf.n.cmd = 5;
  debug_buf.n.val.b.b0 = uart1_buf_pac_rx.b[0];
  debug_buf.n.val.b.b1 = uart1_buf_pac_rx.b[1];
  debug_buf.n.val.b.b2 = uart1_buf_pac_rx.b[2];
  debug_buf.n.val.b.b3 = Dcount;
  uart0_put_data((byte*) &debug_buf);
}

void modzm_parse(void)
{
    // data is in TB_BufIn

  switch (TB_Decode()) {
    case TB_CMD_GIO:
      // analog inputs
      if (TB_bufIn[TB_BUF_TYPE] < 8) {
        TB_SendAck(TB_ERR_OK, adc_read(TB_bufIn[TB_BUF_TYPE]));
        } else {
        TB_SendAck(TB_ERR_TYPE, 0);
      }
    break;
    case TB_CMD_DEBUG:
      //TB_SendAck(TB_ERR_OK, 12345);
      switch (TB_bufIn[TB_BUF_TYPE]) {
        case 0:
          TB_SendAck(TB_ERR_OK, ((dword) 0 << 24) | ((dword) 0 << 16) | ((dword) dbg << 8));
          //TB_SendAckOK();
          break;
        case 1: // get uart status
          TB_SendAck(TB_ERR_OK, ((dword) uart2_status << 16) | ((dword) uart1_status << 8) | ((dword) uart0_status));
          break;
        case 2: // clear statuses
          uart2_status = 0;
          uart1_status = 0;
          uart0_status = 0;
          TB_SendAckOK();
          break;
        case 3: // test uart1
          Ddebug();
          //TB_SendAckOK();
          break;
      }
      break;
  }
}