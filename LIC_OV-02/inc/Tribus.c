/***********************************************
*         Trinamic  Bus  libraly               *
***********************************************/

#include "common_defs.h"
#include "defines.h"
#include <avr/eeprom.h>
//#include <stdint.h>
#include "Tribus_types.h"
#include "Tribus.h"
#include "Tribus_conf.h"


/******************************************************/
//variables
struct TB_PARAM TB_param;
struct TB_GBPARAM TB_gbparam;
struct TB_IO TB_inp;
struct TB_IO TB_out;

byte TB_bufOut[9];
byte TB_bufIn[9];
byte TB_AddrReply;
byte TB_AddrModule;
void * addr_setting_in_eeprom;
int32_t TB_Value;
byte TB_send_flag = 0;
void (*TB_Callback_TX)(void) = NULL;
void (*TB_Callback_setBaud)(byte) = NULL;

/******************************************************/
// private functions
/******************************************************/
void TB_Send(void)
{
  if (TB_Callback_TX != NULL) TB_Callback_TX();
  //TB_send_flag = true;
}


/******************************************************/
void TB_calcSum(void)
{
  byte i, sum;
  sum = 0;
  for(i=0; i<8; i++) {
    sum += TB_bufOut[i];
  }
  TB_bufOut[TB_BUF_SUM] = sum;
}

/******************************************************/
// public functions
/******************************************************/
// initialize
void TB_Init(void * setting_in_eeprom)
{
  addr_setting_in_eeprom = setting_in_eeprom;
  //                 DST,   SRC, size
  eeprom_read_block((void *) &TB_gbparam, setting_in_eeprom, sizeof(struct TB_GBPARAM));
  if (TB_gbparam.eemagic != 66) {
    // not valid data in eeprom
    TB_gbparam.eemagic = 66;
    TB_gbparam.baud = 4;
    TB_gbparam.address = 3;
    TB_gbparam.telegram_pause_time = 0;
    TB_gbparam.host_address = 2;
    // save default setting to eeprom
    eeprom_write_block((void *) &TB_gbparam, setting_in_eeprom, sizeof(struct TB_GBPARAM));
  }
  // ted mame funkèni konfiguraci naètenou

  // zvolíme správnou komunikaèní rychlost:
  if (TB_Callback_setBaud != NULL) TB_Callback_setBaud(TB_gbparam.baud);

  // poznaèíme si adresy
  TB_AddrReply = TB_gbparam.host_address;
  TB_AddrModule= TB_gbparam.address;
}

/******************************************************/
// try to read incoming data
// return 3 - invalid data;
// return 2 - another address
// return 1 - reserved (backward compatibility)
// return 0 - valid command
byte TB_Read(void)
{
  byte i;
  byte sum;

  // check address
  if (TB_bufIn[TB_BUF_ADDRESS] != TB_AddrModule) return 2;
  
  // check SUM byte
  sum = 0;
  for( i=0; i<8; i++) {
    sum += TB_bufIn[i];
  }
  if (sum != TB_bufIn[TB_BUF_SUM]) {
    TB_SendAck(1, 0); // wrong checksum
    return 3; // bad checksum
  }

  // we have valid data in TB_bufIn
  return 0;
}


/******************************************************/
// decode incoming command
// return = unhandled command number, 0=handled or unknown
byte TB_Decode(void)
{
  volatile byte b;
  TB_Value = (((int32_t) TB_bufIn[4]) << 24) |
             (((int32_t) TB_bufIn[5]) << 16) |
             (((int32_t) TB_bufIn[6]) <<  8) |
             (((int32_t) TB_bufIn[7])      ) ;

  switch (TB_bufIn[TB_BUF_COMMAND]) {
    case TB_CMD_DEBUG:
      return TB_CMD_DEBUG;
      break;
    case TB_CMD_ROR:
      TB_SendAck(TB_ERR_OK, 0);
      return TB_CMD_ROR;
      break;
    case TB_CMD_ROL:
      TB_SendAck(TB_ERR_OK, 0);
      return TB_CMD_ROL;
      break;
    case TB_CMD_MVP:
      return TB_CMD_MVP;
      break;
    case TB_CMD_SAP: // set axis parameter
      if (TB_bufIn[TB_BUF_MOTOR] != 0) {
        TB_SendAck(TB_ERR_VALUE, 0); // invalid value
      } else {
        switch (TB_bufIn[TB_BUF_TYPE]) {
          case TB_PARAM_SPEED:
            TB_param.speed = TB_Value;
            break;
          case TB_PARAM_ACCELERATION:
            TB_param.acceleration = TB_Value;
            break;
          case TB_PARAM_CURRENT_RUN:
            TB_param.current = TB_Value;
            break;
          case TB_PARAM_CURRENT_HOLD:
            TB_param.current_hold = TB_Value;
            break;
          case TB_PARAM_RESOLUTION:
            TB_param.resolution = TB_Value;
            break;

// doplnit nastavitelné parametry
        }
        TB_SendAckOK();
      }
      break;
    case TB_CMD_GAP: // get axis parameter
      if (TB_bufIn[TB_BUF_MOTOR] != 0) {
        TB_SendAck(TB_ERR_VALUE, 0); // invalid value
      } else {
        switch (TB_bufIn[TB_BUF_TYPE]) {
          case TB_PARAM_ACTUAL_POSITION:
            TB_SendAck(TB_ERR_OK, TB_param.actual_position);
            break;
          case TB_PARAM_ACCELERATION:
            TB_SendAck(TB_ERR_OK, TB_param.acceleration);
            break;
          case TB_PARAM_CURRENT_RUN:
            TB_SendAck(TB_ERR_OK, TB_param.current);
            break;
          case TB_PARAM_CURRENT_HOLD:
            TB_SendAck(TB_ERR_OK, TB_param.current_hold);
            break;
          case TB_PARAM_SPEED:
            TB_SendAck(TB_ERR_OK, TB_param.speed);
            break;
          case TB_PARAM_RESOLUTION:
            TB_SendAck(TB_ERR_OK, TB_param.resolution);
            break;
          case TB_PARAM_RFS_DISTANCE:
            TB_SendAck(TB_ERR_OK, TB_param.rfs_distance);
            break;
// doplnit nastavitelné parametry
          default:
            TB_SendAck(TB_ERR_VALUE, 0);
            break;
        }
      }
      break;
    case TB_CMD_SIO:
      switch (TB_bufIn[TB_BUF_MOTOR]) {
        case 0:
          #ifdef TB_SIO_BANK_0_IMPLEMENTED
            return TB_CMD_SIO;
          #else
            TB_SendAck(TB_ERR_TYPE, 0); // invalid value
          #endif
          break;
        case 1:
          #ifdef TB_SIO_BANK_1_IMPLEMENTED
            return TB_CMD_SIO;
          #else
            TB_SendAck(TB_ERR_TYPE, 0); // invalid value
          #endif
          break;
        case 2:
          switch (TB_bufIn[TB_BUF_TYPE]) {
            case 0:
              TB_out.b0 = (TB_Value != 0);
              TB_SendAckOK();
              break;
            case 1:
              TB_out.b1 = (TB_Value != 0);
              TB_SendAckOK();
              break;
            default:
              TB_SendAck(TB_ERR_TYPE, 0); // invalid value
              break;
          }
          break;
        default:
          TB_SendAck(TB_ERR_TYPE, 0); // invalid value
      }
      return TB_CMD_SIO;
      break;
    case TB_CMD_GIO:
      switch (TB_bufIn[TB_BUF_MOTOR]) {
        case 0: // inputs (4)
          switch (TB_bufIn[TB_BUF_TYPE]) {
            case 0:
              TB_SendAck(TB_ERR_OK, TB_inp.b0);
              break;
            case 1:
              TB_SendAck(TB_ERR_OK, TB_inp.b1);
              break;
            case 2:
              TB_SendAck(TB_ERR_OK, TB_inp.b2);
              break;
            case 3:
              TB_SendAck(TB_ERR_OK, TB_inp.b3);
              break;
            default:
              TB_SendAck(TB_ERR_VALUE, 0); // invalid value
              break;
          }
          break;
        case 1: // analog inputs (2)
          //TB_SendAck(TB_ERR_VALUE, 0); // invalid value
          return TB_CMD_GIO;
          break;
        case 2: // outputs (2);
          switch (TB_bufIn[TB_BUF_TYPE]) {
            case 0:
              TB_SendAck(TB_ERR_OK, TB_out.b0);
              break;
            case 1:
              TB_SendAck(TB_ERR_OK, TB_out.b1);
              break;
            default:
              TB_SendAck(TB_ERR_VALUE, 0); // invalid value
              break;
          }
          break;
      }
      break;
    case TB_CMD_RFS:
      return TB_CMD_RFS;
      break;
    case TB_CMD_SGP:
      if (TB_bufIn[TB_BUF_MOTOR] != 0) {
        TB_SendAck(TB_ERR_VALUE, 0); // invalid value
      } else {
        switch (TB_bufIn[TB_BUF_TYPE]) {
          case TB_GBPARAM_EEMAGIC:
            if (TB_Value != TB_gbparam.eemagic) {
              TB_gbparam.eemagic = TB_Value;
              b = (void *) &(TB_gbparam.eemagic) - (void *) &(TB_gbparam);
              eeprom_update_byte(b+addr_setting_in_eeprom, TB_gbparam.eemagic);
            }
            TB_SendAck(TB_ERR_OK, 0);
            break;
          case TB_GBPARAM_BAUD:
            if (TB_Value != TB_gbparam.baud) {
              TB_gbparam.baud = TB_Value;
              b = (void *) &(TB_gbparam.baud) - (void *) &(TB_gbparam);
              eeprom_update_byte(b+addr_setting_in_eeprom, TB_gbparam.baud);
            }
            TB_SendAck(TB_ERR_OK, 0);
            break;
          case TB_GBPARAM_ADDRESS:
            if (TB_Value != TB_gbparam.address) {
              TB_gbparam.address = TB_Value;
              b = (void *) &(TB_gbparam.address) - (void *) &(TB_gbparam);
              eeprom_update_byte(b+addr_setting_in_eeprom, TB_gbparam.address);
            }
            TB_SendAck(TB_ERR_OK, 0);
            break;
          case TB_GBPARAM_HOST_ADDR:
            if (TB_Value != TB_gbparam.host_address) {
              TB_gbparam.host_address = TB_Value;
              b = (void *) &(TB_gbparam.host_address) - (void *) &(TB_gbparam);
              eeprom_update_byte(b+addr_setting_in_eeprom, TB_gbparam.host_address);
            }
            TB_SendAck(TB_ERR_OK, 0);
            break;
          default:
            TB_SendAck(TB_ERR_VALUE, 0); // invalid value
            break;
        }
      }
      break;
    case TB_CMD_GGP:
      if (TB_bufIn[TB_BUF_MOTOR] != 0) {
        TB_SendAck(TB_ERR_VALUE, 0); // invalid value
      } else {
        switch (TB_bufIn[TB_BUF_TYPE]) {
          case TB_GBPARAM_BAUD:
            TB_SendAck(TB_ERR_OK, TB_gbparam.baud);
            break;
          case TB_GBPARAM_ADDRESS:
            TB_SendAck(TB_ERR_OK, TB_gbparam.address);
            break;
          case TB_GBPARAM_HOST_ADDR:
            TB_SendAck(TB_ERR_OK, TB_gbparam.host_address);
            break;
          case TB_GBPARAM_EEMAGIC:
            TB_SendAck(TB_ERR_OK, TB_gbparam.eemagic);
            break;
          default:
            TB_SendAck(TB_ERR_VALUE, 0); // invalid value
            break;
        }
      }
      break;
    case 136: // get module version
      if (TB_bufIn[TB_BUF_TYPE] == 0) {
        // text mode
        TB_bufOut[0] = TB_AddrReply;
        TB_bufOut[1] = '1';
        TB_bufOut[2] = '0';
        TB_bufOut[3] = '2';
        TB_bufOut[4] = '1';
        TB_bufOut[5] = 'V';
        TB_bufOut[6] = '1';
        TB_bufOut[7] = '2';
        TB_bufOut[8] = '0';
        TB_Send();
       } else {
        // binary mode
        TB_SendAck(TB_ERR_OK, (0x10203040));
      };
      break;
    default:
      TB_SendAck(TB_ERR_COMMAND, 0); // invalid command
      return 0;
  }
  return 0;
}

/******************************************************/
// set command to module
void TB_SetParam(byte addr, byte command, byte type, byte motor, long int value)
{
  TB_bufOut[0] = addr;
  TB_bufOut[1] = command;
  TB_bufOut[2] = type;
  TB_bufOut[3] = motor;
  TB_bufOut[4] = value >> 24;
  TB_bufOut[5] = value >> 16;
  TB_bufOut[6] = value >> 8;
  TB_bufOut[7] = value >> 0;
  TB_calcSum();
  TB_Send();
}

/******************************************************/
// send response from module
void TB_SendAck(byte status, long int value)
{
  TB_bufOut[0] = TB_AddrReply;
  TB_bufOut[1] = TB_AddrModule;
  TB_bufOut[2] = status;
  TB_bufOut[3] = TB_bufIn[TB_BUF_COMMAND]; //command;
  TB_bufOut[4] = value >> 24;
  TB_bufOut[5] = value >> 16;
  TB_bufOut[6] = value >> 8;
  TB_bufOut[7] = value >> 0;
  TB_calcSum();
  TB_Send();
}

/******************************************************/
// send OK response from module
inline void TB_SendAckOK(void)
{
  TB_SendAck(100, 0);
}

