@echo off
echo.
echo.
echo *** Linking ***
avr-gcc -mmcu=atmega8 obj/uart_tri_0.o obj/i2c.o LIC_ZM_ext.o -o LIC_ZM_ext.elf -Wl,--start-group -Wl,--end-group -Wl,--gc-sections
echo Finish

echo on