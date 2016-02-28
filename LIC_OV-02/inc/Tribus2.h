#ifndef __TRIBUS2_H
#define __TRIBUS2_H

#define TRI_LEN (9) 


// shared variables
extern struct TB_PARAM TB2_param;
extern struct TB_GBPARAM TB2_gbparam;
extern struct TB_IO TB2_inp;
extern struct TB_IO TB2_out;

extern byte TB2_bufOut[9];
extern byte TB2_bufIn[9];
extern byte TB2_AddrReply;
extern byte TB2_AddrModule;
extern long int TB2_Value;
extern byte TB2_send_flag;

extern void (*TB2_Callback_TX)(void); // callback for sending data
extern void (*TB2_Callback_setBaud)(byte); // callback for switch uart speed

// init
void TB2_Init(void * setting_in_eeprom);

// main
byte TB2_Read(void);
byte TB2_Decode(void);

// auxiliary
void TB2_SendAck(byte status, long int value);
void TB2_SendAckOK(void);


#endif
