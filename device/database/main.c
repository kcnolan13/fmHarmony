// Marcel Marki & Kyle Nolan
#include <avr/io.h>
#include <avr/delay.h>
#include <avr/sfr_defs.h>
#include <stdio.h>
#include <string.h>
#include <avr/eeprom.h>

#include "display.h"


int main (int argc, char *argv[])
{
	//Intitialize LCD. Set Blinking cursor.
	int readbyte;

	lcd_cursor();
	eeprom_write_byte(0,7);
	readbyte = eeprom_read_byte((int*)0);

	char readchar = (char)readbyte;
    
	//Write ECE to screen
    //string_write(readchar);
	char_write(readchar);

	while(1){

	}
	return 0; //should never get here.
}