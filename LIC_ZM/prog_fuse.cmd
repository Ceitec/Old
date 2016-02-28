pause
avrdude -P COM4 -c STK500v2 -p m644p -U lfuse:w:0xff:m -U hfuse:w:0xd1:m -U efuse:w:0xfd:m