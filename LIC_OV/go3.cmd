@echo off
"avr-objcopy" -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "LIC_OV.elf" "LIC_OV.hex"
"avr-objcopy" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "LIC_OV.elf" "LIC_OV.eep"
avr-size --format=avr LIC_OV.elf --mcu=atmega8