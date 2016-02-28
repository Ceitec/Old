@echo off
echo * Cleaning *
rm obj/*.o
rm obj/*.d
rm *.hex
rm *.elf
for /R ".\inc" %%i IN (*.c) DO (
echo.
echo.
echo *** File: %%~nxi ***
avr-gcc -c -x c -funsigned-char -funsigned-bitfields -DDEBUG  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atmega8 -Iinc -DF_CPU=14745600UL -Wall -Wstrict-prototypes -std=gnu99 -MD -MP inc/%%~ni.c -o obj/%%~ni.o
)
echo.
echo.
echo *** File: LIC_ZM_ext ***
avr-gcc -c -x c -funsigned-char -funsigned-bitfields -DDEBUG  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atmega8 -Iinc -DF_CPU=14745600UL -Wall -Wstrict-prototypes -std=gnu99 -MD -MP LIC_ZM_ext.c -o LIC_ZM_ext.o
echo Finish
echo on