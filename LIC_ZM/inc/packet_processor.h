#ifndef PACKET_PROCESSOR_H_
#define PACKET_PROCESSOR_H_
/*
 * packet_processor.h
 *
 * Created: 5.7.2014 17:56:28
 *  Author: Michal
 */

// must be power of 2
#define PP_QUEUE_LENGTH (16)
/*
union Tpacket {
  byte raw[9];
  struct {
    byte addr;
    byte inst;
    byte type;
    byte moto;
    dword data;
    byte ssum;
  } named;
};

union TppData {
  byte raw[9*PP_QUEUE_LENGTH];
  union Tpacket packet[PP_QUEUE_LENGTH];
};

union TppData pp_dataIn;
union TppData pp_dataOut;
byte pp_Inlast;
byte pp_Infirst;
byte pp_Outlast;
byte pp_Outfirst;
*/

void pp_init(void);
void pp_loop(void);

#endif /* PACKET_PROCESSOR_H_ */