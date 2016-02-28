pause
avrdude -P COM4 -c STK500v2 -p m32 -U lfuse:w:0x3f:m -U hfuse:w:0xc1:m 