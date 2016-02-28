@echo off
echo.
echo.
echo *** Linking ***
avr-gcc -mmcu=atmega644p obj/uart_tri_0.o obj/uart_tri_1.o obj/extBus.o obj/adc.o obj/i2c.o obj/packet_processor.o obj/Tribus.o LIC_ZM.o obj\module_ZM.o obj\timer.o -o LIC_ZM.elf -Wl,--start-group -Wl,--end-group -Wl,--gc-sections
echo Finish

echo on