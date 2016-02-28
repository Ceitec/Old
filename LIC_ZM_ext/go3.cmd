@echo off
"avr-objcopy" -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "LIC_ZM_ext.elf" "LIC_ZM_ext.hex"
"avr-objcopy" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "LIC_ZM_ext.elf" "LIC_ZM_ext.eep"
avr-size --format=avr LIC_ZM_ext.elf --mcu=atmega8