/***********************************************
*         Trinamic  Bus  libraly               *
***********************************************/

#include "common_defs.h"
#include "defines.h"
#include <avr/eeprom.h>
//#include <stdint.h>
#include "Tribus_types.h"
#include "Tribus.h"
#include "Tribus2.h"
#include "Tribus2_conf.h"


/******************************************************/
//variables
struct TB_PARAM TB2_param;
struct TB_GBPARAM TB2_gbparam;
struct TB_IO TB2_inp;
struct TB_IO TB2_out;

byte TB2_bufOut[9];
byte TB2_bufIn[9];
byte TB2_AddrReply;
byte TB2_AddrModule;
void * addr_setting_in_eeprom2;
int32_t TB2_Value;
byte TB2_send_flag = 0;
void (*TB2_Callback_TX)(void) = NULL;
void (*TB2_Callback_setBaud)(byte) = NULL;

/******************************************************/
// private functions
/******************************************************/
void TB2_Send(void)
{
  if (TB2_Callback_TX != NULL) TB2_Callback_TX();
  //TB2_send_flag = true;
}


/******************************************************/
void TB2_calcSum(void)
{
  byte i, sum;
  sum = 0;
  for(i=0; i<8; i++) {
    sum += TB2_bufOut[i];
  }
  TB2_bufOut[TB_BUF_SUM] = sum;
}

/******************************************************/
// public functions
/******************************************************/
// initialize
void TB2_Init(void * setting_in_eeprom)
{
  addr_setting_in_eeprom2 = setting_in_eeprom;
  //                 DST,   SRC, size
  eeprom_read_block((void *) &TB2_gbparam, setting_in_eeprom, sizeof(struct TB_GBPARAM));
  if (TB2_gbparam.eemagic != 66) {
    // not valid data in eeprom
    TB2_gbparam.eemagic = 66;
    TB2_gbparam.baud = 4;
    TB2_gbparam.address = 4;
    TB2_gbparam.telegram_pause_time = 0;
    TB2_gbparam.host_address = 2;
    // save default setting to eeprom
    eeprom_write_block((void *) &TB2_gbparam, setting_in_eeprom, sizeof(struct TB_GBPARAM));
  }
  // ted mame funkèni konfiguraci naètenou

  // zvolíme správnou komunikaèní rychlost:
  if (TB2_Callback_setBaud != NULL) TB2_Callback_setBaud(TB_gbparam.baud);

  // poznaèíme si adresy
  TB2_AddrReply = TB2_gbparam.host_address;
  TB2_AddrModule= TB2_gbparam.address;
}

/******************************************************/
// try to read incoming data
// return 3 - invalid data;
// return 2 - another address
// return 1 - reserved (backward compatibility)
// return 0 - valid command
byte TB2_Read(void)
{
  byte i;
  byte sum;

  // check address
  if (TB2_bufIn[TB_BUF_ADDRESS] != TB2_AddrModule) return 2;
  
  // check SUM byte
  sum = 0;
  for( i=0; i<8; i++) {
    sum += TB2_bufIn[i];
  }
  if (sum != TB2_bufIn[TB_BUF_SUM]) {
    TB2_SendAck(1, 0); // wrong checksum
    return 3; // bad checksum
  }

  // we have valid data in TB2_bufIn
  return 0;
}


/******************************************************/
// decode incoming command
// return = unhandled command number, 0=handled or unknown
byte TB2_Decode(void)
{
  volatile byte b;
  TB2_Value = (((int32_t) TB2_bufIn[4]) << 24) |
             (((int32_t) TB2_bufIn[5]) << 16) |
             (((int32_t) TB2_bufIn[6]) <<  8) |
             (((int32_t) TB2_bufIn[7])      ) ;

  switch (TB2_bufIn[TB_BUF_COMMAND]) {
    case TB_CMD_DEBUG:
      return TB_CMD_DEBUG;
      break;
    case TB_CMD_ROR:
      TB2_SendAck(TB_ERR_OK, 0);
      return TB_CMD_ROR;
      break;
    case TB_CMD_ROL:
      TB2_SendAck(TB_ERR_OK, 0);
      return TB_CMD_ROL;
      break;
    case TB_CMD_MVP:
      return TB_CMD_MVP;
      break;
    case TB_CMD_SAP: // set axis parameter
      if (TB2_bufIn[TB_BUF_MOTOR] != 0) {
        TB2_SendAck(TB_ERR_VALUE, 0); // invalid value
      } else {
        switch (TB2_bufIn[TB_BUF_TYPE]) {
          case TB_PARAM_SPEED:
            TB2_param.speed = TB2_Value;
            break;
          case TB_PARAM_ACCELERATION:
            TB2_param.acceleration = TB2_Value;
            break;
          case TB_PARAM_CURRENT_RUN:
            TB2_param.current = TB2_Value;
            break;
          case TB_PARAM_CURRENT_HOLD:
            TB2_param.current_hold = TB2_Value;
            break;
          case TB_PARAM_RESOLUTION:
            TB2_param.resolution = TB2_Value;
            break;

// doplnit nastavitelné parametry
        }
        TB2_SendAckOK();
      }
      break;
    case TB_CMD_GAP: // get axis parameter
      if (TB2_bufIn[TB_BUF_MOTOR] != 0) {
        TB2_SendAck(TB_ERR_VALUE, 0); // invalid value
      } else {
        switch (TB2_bufIn[TB_BUF_TYPE]) {
          case TB_PARAM_ACTUAL_POSITION:
            TB2_SendAck(TB_ERR_OK, TB2_param.actual_position);
            break;
          case TB_PARAM_ACCELERATION:
            TB2_SendAck(TB_ERR_OK, TB2_param.acceleration);
            break;
          case TB_PARAM_CURRENT_RUN:
            TB2_SendAck(TB_ERR_OK, TB2_param.current);
            break;
          case TB_PARAM_CURRENT_HOLD:
            TB2_SendAck(TB_ERR_OK, TB2_param.current_hold);
            break;
          case TB_PARAM_SPEED:
            TB2_SendAck(TB_ERR_OK, TB2_param.speed);
            break;
          case TB_PARAM_RESOLUTION:
            TB2_SendAck(TB_ERR_OK, TB2_param.resolution);
            break;
          case TB_PARAM_RFS_DISTANCE:
            TB2_SendAck(TB_ERR_OK, TB2_param.rfs_distance);
            break;
// doplnit nastavitelné parametry
          default:
            TB2_SendAck(TB_ERR_VALUE, 0);
            break;
        }
      }
      break;
    case TB_CMD_SIO:
      switch (TB2_bufIn[TB_BUF_MOTOR]) {
        case 0:
          #ifdef TB2_SIO_BANK_0_IMPLEMENTED
            return TB_CMD_SIO;
          #else
            TB2_SendAck(TB_ERR_TYPE, 0); // invalid value
          #endif
          break;
        case 1:
          #ifdef TB2_SIO_BANK_1_IMPLEMENTED
            return TB_CMD_SIO;
          #else
            TB2_SendAck(TB_ERR_TYPE, 0); // invalid value
          #endif
          break;
        case 2:
          switch (TB2_bufIn[TB_BUF_TYPE]) {
            case 0:
              TB2_out.b0 = (TB2_Value != 0);
              TB2_SendAckOK();
              break;
            case 1:
              TB2_out.b1 = (TB2_Value != 0);
              TB2_SendAckOK();
              break;
            default:
              TB2_SendAck(TB_ERR_TYPE, 0); // invalid value
              break;
          }
          break;
        default:
          TB2_SendAck(TB_ERR_TYPE, 0); // invalid value
      }
      return TB_CMD_SIO;
      break;
    case TB_CMD_GIO:
      switch (TB2_bufIn[TB_BUF_MOTOR]) {
        case 0: // inputs (4)
          switch (TB2_bufIn[TB_BUF_TYPE]) {
            case 0:
              TB2_SendAck(TB_ERR_OK, TB2_inp.b0);
              break;
            case 1:
              TB2_SendAck(TB_ERR_OK, TB2_inp.b1);
              break;
            case 2:
              TB2_SendAck(TB_ERR_OK, TB2_inp.b2);
              break;
            case 3:
              TB2_SendAck(TB_ERR_OK, TB2_inp.b3);
              break;
            default:
              TB2_SendAck(TB_ERR_VALUE, 0); // invalid value
              break;
          }
          break;
        case 1: // analog inputs (2)
          //TB2_SendAck(TB_ERR_VALUE, 0); // invalid value
          return TB_CMD_GIO;
          break;
        case 2: // outputs (2);
          switch (TB2_bufIn[TB_BUF_TYPE]) {
            case 0:
              TB2_SendAck(TB_ERR_OK, TB2_out.b0);
              break;
            case 1:
              TB2_SendAck(TB_ERR_OK, TB2_out.b1);
              break;
            default:
              TB2_SendAck(TB_ERR_VALUE, 0); // invalid value
              break;
          }
          break;
      }
      break;
    case TB_CMD_RFS:
      return TB_CMD_RFS;
      break;
    case TB_CMD_SGP:
      if (TB2_bufIn[TB_BUF_MOTOR] != 0) {
        TB2_SendAck(TB_ERR_VALUE, 0); // invalid value
      } else {
        switch (TB2_bufIn[TB_BUF_TYPE]) {
          case TB_GBPARAM_EEMAGIC:
            if (TB2_Value != TB2_gbparam.eemagic) {
              TB2_gbparam.eemagic = TB2_Value;
              b = (void *) &(TB2_gbparam.eemagic) - (void *) &(TB2_gbparam);
              eeprom_update_byte(b+addr_setting_in_eeprom2, TB2_gbparam.eemagic);
            }
            TB2_SendAck(TB_ERR_OK, 0);
            break;
          case TB_GBPARAM_BAUD:
            if (TB2_Value != TB2_gbparam.baud) {
              TB2_gbparam.baud = TB2_Value;
              b = (void *) &(TB2_gbparam.baud) - (void *) &(TB2_gbparam);
              eeprom_update_byte(b+addr_setting_in_eeprom2, TB2_gbparam.baud);
            }
            TB2_SendAck(TB_ERR_OK, 0);
            break;
          case TB_GBPARAM_ADDRESS:
            if (TB2_Value != TB2_gbparam.address) {
              TB2_gbparam.address = TB2_Value;
              b = (void *) &(TB2_gbparam.address) - (void *) &(TB2_gbparam);
              eeprom_update_byte(b+addr_setting_in_eeprom2, TB2_gbparam.address);
            }
            TB2_SendAck(TB_ERR_OK, 0);
            break;
          case TB_GBPARAM_HOST_ADDR:
            if (TB2_Value != TB2_gbparam.host_address) {
              TB2_gbparam.host_address = TB2_Value;
              b = (void *) &(TB2_gbparam.host_address) - (void *) &(TB2_gbparam);
              eeprom_update_byte(b+addr_setting_in_eeprom2, TB2_gbparam.host_address);
            }
            TB2_SendAck(TB_ERR_OK, 0);
            break;
          default:
            TB2_SendAck(TB_ERR_VALUE, 0); // invalid value
            break;
        }
      }
      break;
    case TB_CMD_GGP:
      if (TB2_bufIn[TB_BUF_MOTOR] != 0) {
        TB2_SendAck(TB_ERR_VALUE, 0); // invalid value
      } else {
        switch (TB2_bufIn[TB_BUF_TYPE]) {
          case TB_GBPARAM_BAUD:
            TB2_SendAck(TB_ERR_OK, TB2_gbparam.baud);
            break;
          case TB_GBPARAM_ADDRESS:
            TB2_SendAck(TB_ERR_OK, TB2_gbparam.address);
            break;
          case TB_GBPARAM_HOST_ADDR:
            TB2_SendAck(TB_ERR_OK, TB2_gbparam.host_address);
            break;
          case TB_GBPARAM_EEMAGIC:
            TB2_SendAck(TB_ERR_OK, TB2_gbparam.eemagic);
            break;
          default:
            TB2_SendAck(TB_ERR_VALUE, 0); // invalid value
            break;
        }
      }
      break;
    case 136: // get module version
      if (TB2_bufIn[TB_BUF_TYPE] == 0) {
        // text mode
        TB2_bufOut[0] = TB2_AddrReply;
        TB2_bufOut[1] = '1';
        TB2_bufOut[2] = '0';
        TB2_bufOut[3] = '2';
        TB2_bufOut[4] = '1';
        TB2_bufOut[5] = 'V';
        TB2_bufOut[6] = '1';
        TB2_bufOut[7] = '2';
        TB2_bufOut[8] = '0';
        TB2_Send();
       } else {
        // binary mode
        TB2_SendAck(TB_ERR_OK, (0x10203040));
      };
      break;
    default:
      TB2_SendAck(TB_ERR_COMMAND, 0); // invalid command
      return 0;
  }
  return 0;
}

/******************************************************/
// set command to module
void TB2_SetParam(byte addr, byte command, byte type, byte motor, long int value)
{
  TB2_bufOut[0] = addr;
  TB2_bufOut[1] = command;
  TB2_bufOut[2] = type;
  TB2_bufOut[3] = motor;
  TB2_bufOut[4] = value >> 24;
  TB2_bufOut[5] = value >> 16;
  TB2_bufOut[6] = value >> 8;
  TB2_bufOut[7] = value >> 0;
  TB2_calcSum();
  TB2_Send();
}

/******************************************************/
// send response from module
void TB2_SendAck(byte status, long int value)
{
  TB2_bufOut[0] = TB2_AddrReply;
  TB2_bufOut[1] = TB2_AddrModule;
  TB2_bufOut[2] = status;
  TB2_bufOut[3] = TB2_bufIn[TB_BUF_COMMAND]; //command;
  TB2_bufOut[4] = value >> 24;
  TB2_bufOut[5] = value >> 16;
  TB2_bufOut[6] = value >> 8;
  TB2_bufOut[7] = value >> 0;
  TB2_calcSum();
  TB2_Send();
}

/******************************************************/
// send OK response from module
inline void TB2_SendAckOK(void)
{
  TB2_SendAck(100, 0);
}

