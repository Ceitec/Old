@echo off
echo.
echo *** File: LIC_ZM/extBus ***
avr-gcc -c -x c -funsigned-char -funsigned-bitfields -DDEBUG  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atmega644p -Iinc -DF_CPU=14745600UL -Wall -Wstrict-prototypes -std=gnu99 -MD -MP inc\extBus.c -o obj\extBus.o
echo Finish
echo on