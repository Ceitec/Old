/*
 * Tribus_types.h
 *
 * Created: 9.7.2014 13:28:05
 *  Author: Michal
 */


#ifndef TRIBUS_TYPES_H_
#define TRIBUS_TYPES_H_

// data format
#define TB_BUF_ADDRESS  0
#define TB_BUF_COMMAND  1
#define TB_BUF_TYPE     2
#define TB_BUF_MOTOR    3
#define TB_BUF_SUM      8

// instructions
#define TB_CMD_DEBUG    254
#define TB_CMD_ROR      1
#define TB_CMD_ROL      2
#define TB_CMD_MST      3
#define TB_CMD_MVP      4
#define TB_CMD_SAP      5
#define TB_CMD_GAP      6
#define TB_CMD_STAP     7
#define TB_CMD_RSAP     8
#define TB_CMD_SGP      9
#define TB_CMD_GGP     10
#define TB_CMD_STGP    11
#define TB_CMD_RSGP    12
#define TB_CMD_RFS     13
#define TB_CMD_SIO     14
#define TB_CMD_GIO     15
#define TB_CMD_WAIT    27
#define TB_CMD_SCO     30
#define TB_CMD_GCO     31
#define TB_CMD_CCO     32

// error codes
#define TB_ERR_OK           100
#define TB_ERR_EEPROM_OK    101
#define TB_ERR_SUM            1
#define TB_ERR_COMMAND        2
#define TB_ERR_TYPE           3
#define TB_ERR_VALUE          4
#define TB_ERR_EEPROM_LOCK    5
#define TB_ERR_NA             6

// axis parameters
#define TB_PARAM_TARGET_POSITION 0
#define TB_PARAM_ACTUAL_POSITION 1
#define TB_PARAM_SPEED           4
#define TB_PARAM_ACCELERATION    5
#define TB_PARAM_CURRENT_RUN     6
#define TB_PARAM_CURRENT_HOLD    7
#define TB_PARAM_RESOLUTION    140
#define TB_PARAM_RFS_DISTANCE  196

#define TB_GBPARAM_EEMAGIC      64
#define TB_GBPARAM_BAUD         65
#define TB_GBPARAM_ADDRESS      66
#define TB_GBPARAM_TEL_PAU_TIME 75
#define TB_GBPARAM_HOST_ADDR    76

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


#endif /* TRIBUS_TYPES_H_ */