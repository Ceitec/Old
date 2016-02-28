pause
avrdude -P COM4 -c STK500v2 -p m8 -U lfuse:w:0x20:m -U hfuse:w:0xd1:m 