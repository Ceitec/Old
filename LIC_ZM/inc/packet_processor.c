/*
 * packet_processor.c
 *
 * Created: 5.7.2014 17:55:31
 *  Author: Michal
 */

#include "common_defs.h"
#include "defines.h"
#include "Tribus_types.h"
#include "uart_tri_0.h"
#include "uart_tri_1.h"
#include "extBus.h"

#include "Tribus.h"
#include "module_ZM.h"
#include "packet_processor.h"

Ttripac pp_buf_pc;  // buffer
Ttripac pp_buf_dev; // buffer
byte pp_f_frompc;   // flag
byte pp_f_fromdev;  // flag


// DEBUG !!!
byte Dcount = 0;

//----------------------------------------------------------
// process requests
void pp_processRequests(void)
{
//  byte dataindex;
  byte i;
  byte addr;

  if (pp_f_frompc) { 
    // some data in main packet buffer
    addr = pp_buf_pc.n.addr;
    if (addr == 0) {
      // data for us
      for (i=0; i<9; i++) {
        TB_bufIn[i] = pp_buf_pc.b[i];
      }
      modzm_parse();
    }
    else if (addr <= 64) {
      // data to internal
      uart1_put_data((byte *) &pp_buf_pc);
    }
    else {
      // data to external
      uart2_put_data((byte *) &pp_buf_pc);
    }
    pp_f_frompc = FALSE;
  }
}

//----------------------------------------------------------
// send responses
void pp_SendResponses(void)
{
//  byte i;

  // local to PC
  if (pp_f_fromdev) {
    uart0_put_data((byte *) &pp_buf_dev);
    pp_f_fromdev = false;
  }
}

//----------------------------------------------------------
void pp_receive_PC(void)
{
  byte * data_ptr;
  byte i;
  
  // receive error
  if (uart0_flags.data_receive_error) {
    uart0_flags.data_receive_error = FALSE;
    
    //pp_buf_pc.n.addr = 0;
    //pp_buf_pc.n.cmd = 255;
    //pp_f_frompc = TRUE;
  }
  
  // succesful receive
  // copy to pp Rx buffer
  if (uart0_flags.data_received) {
    data_ptr = uart0_get_data_begin();
    for(i=0; i<TRI_LEN; i++) {
      pp_buf_pc.b[i] = (*data_ptr);
      data_ptr++;      
    }
    uart0_get_data_end();
    pp_f_frompc = TRUE;
  }
}

//----------------------------------------------------------
void pp_receive_own(void)
{
  byte * data_ptr;
  byte i;
  
  if (!pp_f_fromdev) {
    if (TB_send_flag) {
      TB_send_flag = FALSE;
      data_ptr = TB_bufOut;
      for(i=0; i<TRI_LEN; i++) {
        pp_buf_dev.b[i] = (*data_ptr);
        data_ptr++;      
      }
      pp_f_fromdev = TRUE;
    }
  }
}

//----------------------------------------------------------
void pp_receive_int(void)
{
  byte * data_ptr;
  byte i;
  
  // receive error
  if (uart1_flags.data_receive_error) {
    uart1_flags.data_receive_error = FALSE;
    Dcount += 16;
  }
  
  // succesful receive
  // copy to pp Rx buffer
  if (!pp_f_fromdev) {
    if (uart1_flags.data_received) {
      Dcount++;
      data_ptr = uart1_get_data_begin();
      for(i=0; i<TRI_LEN; i++) {
        pp_buf_dev.b[i] = (*data_ptr);
        data_ptr++;      
      }
      uart1_get_data_end();
      pp_f_fromdev = TRUE;
    }
  }
}

//----------------------------------------------------------
void pp_receive_ext(void)
{
  byte * data_ptr;
  byte i;
  
  // receive error
  if (uart2_flags.data_receive_error) {
    uart2_flags.data_receive_error = FALSE;
  }
  
  // succesful receive
  // copy to pp Rx buffer
  if (!pp_f_fromdev) {
    if (uart2_flags.data_received) {
      data_ptr = uart2_get_data();
      for(i=0; i<TRI_LEN; i++) {
        pp_buf_dev.b[i] = (*data_ptr);
        data_ptr++;      
      }
      uart2_flags.data_received = FALSE;
      pp_f_fromdev = TRUE;
    }
  }
}

/**********************************************************/

//----------------------------------------------------------
void pp_init(void)
{
  pp_f_fromdev = FALSE;
  pp_f_frompc = FALSE;
}

//----------------------------------------------------------
void pp_loop(void)
{
  // from PC to devs
  pp_receive_PC();
  pp_processRequests();
  
  // from devs to PC
  pp_receive_own();  // ZM
  pp_receive_int();  // rack
  pp_receive_ext();  // external
  pp_SendResponses();

}