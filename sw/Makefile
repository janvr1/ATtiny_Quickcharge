compile: main.c
	avr-gcc -Os -Wall -mmcu=attiny84a -DF_CPU=1000000UL main.c -o main.out

hex: main.out
	avr-objcopy -O ihex -j.text -j.data main.out main.hex

program: main.hex
	avrdude -v -p t84 -c stk500v1 -b 19200 -U flash:w:main.hex:i -P /dev/ttyUSB0

read_fuses:
	avrdude -p t84 -c stk500v1 -b 19200 -P /dev/ttyUSB0
	
all: compile hex program

clean:
	rm main.out main.hex
