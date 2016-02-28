@echo off
for /R ".\inc" %%i IN (*.c) DO (
echo.
echo.
echo *** File: %%~nxi ***
avr-gcc -pipe -c -x c -funsigned-char -funsigned-bitfields -DDEBUG  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atmega8 -Iinc -Wall -Wstrict-prototypes -std=gnu99 -MD -MP inc/%%~ni.c -o obj/%%~ni.o
)
for /R ".\inc" %%i IN (*.s) DO (
echo.
echo.
echo *** File: %%~nxi ***
avr-gcc -pipe -c -x assembler-with-cpp -funsigned-char -funsigned-bitfields -DDEBUG  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atmega8 -Iinc -Wall -Wstrict-prototypes -std=gnu99 -MD -MP inc/%%~ni.s -o obj/%%~ni.o
)
echo.
echo.
echo *** File: LIC_ZM ***
avr-gcc -pipe -c -x c -funsigned-char -funsigned-bitfields -DDEBUG  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atmega8 -Iinc -Wall -Wstrict-prototypes -std=gnu99 -MD -MP LIC_OV.c -o LIC_OV.o
echo Finish
echo on