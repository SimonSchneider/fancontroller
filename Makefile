BIN=PWM
OBJS=PWM.o lcd.o
MCU=atmega328p#48p#atmega328
MCP=m328p#48p#m328
CLK=4000000

CC=avr-gcc
OBJCOPY=avr-objcopy
ALL_CFLAGS=-Os -Wall -DF_CPU=$(CLK)UL -mmcu=$(MCU)
CFLAGS=$(ALL_CFLAGS)
LDFLAGS=$(ALL_CFLAGS)

${BIN}.hex: ${BIN}.elf
	    ${OBJCOPY} -j .text -j .data -O ihex $< $@

${BIN}.elf: ${OBJS}
	    ${CC} ${LDFLAGS} -o $@ $^

install: ${BIN}.hex
	    avrdude -B 5 -c usbasp -p $(MCP) -U flash:w:$<

clean:
	    rm -f ${BIN}.elf ${BIN}.hex ${OBJS}
