@echo off
echo.
echo.
echo *** Linking ***
avr-gcc -mmcu=atmega8 obj\uart_tri_0.o obj\Tribus.o obj\Tribus2.o obj\timer.o obj\leds.o obj\adc.o LIC_OV.o -o LIC_OV.elf -Wl,--start-group -Wl,--end-group -Wl,--gc-sections
echo Finish

echo on