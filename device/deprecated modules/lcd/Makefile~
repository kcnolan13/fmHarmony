CC=avr-gcc
CFLAGS=-g -Os -Wall -mmcu=atmega1284p
#CFLAGS=-g -Os -Wall -mcall-prologues -mmcu=atmega88
OBJ2HEX=avr-objcopy
#UISP=/usr/local/bin/uisp
TARGET=main
#ADFLAGS=-p m88 -c avrispmkII -P usb 
ADFLAGS=-p m1284p -c avrispmkII -P usb -F

.PHONY: fuses prog erase # .PHONY means that these are not a target


prog : $(TARGET).hex $(TARGET).eeprom  #this programs the chip, depends on Target.hex and Target.eeprom
	avrdude $(ADFLAGS) -V -U flash:w:$(TARGET).hex:i
#       avrdude $(ADFLAGS) -U eeprom:w:$(TARGET).eeprom:i

%.obj : %.o   # make .obj (any obj) if corresponding .o is newer
	 $(CC) $(CFLAGS) $< -o $@  # $@ means target of rule, $< means prerequisite of rule

%.hex : %.obj  # similarly, make any .hex if corresponding .obj is newer
	$(OBJ2HEX) -R .eeprom -O ihex $< $@  # remove  eeprom output ihex translate .obj into hex file

%.eeprom : %.obj  # any eeprom depends on same .obj
	$(OBJ2HEX) -j .eeprop -O ihex $< $@ 

erase :   # this target will just erase avr
	avrdude $(ADFLAGS) -E noreset -e
clean :  # this target will remove all non-source files (will cause a complete re-build on next make)
	rm -f *.hex *.obj *.o

fuses: # program the fuses (NOT done by regular make)
	avrdude $(ADFLAGS) -U lfuse:w:0x62:m -U hfuse:w:0xdf:m -U efuse:w:0xf9:m #www.engbedded.com/fusecalc

