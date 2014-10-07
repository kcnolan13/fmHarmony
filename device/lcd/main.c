// Marcel Marki & Kyle Nolan
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sfr_defs.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "display.h"




int main (int argc, char *argv[])
{
	//Intitialize LCD. Set Blinking cursor.
	lcd_init();

    
	//Write ECE to screen
    string_write("abcdefghijklmnopqrstuv");

    _delay_ms(1000);

    lcd_init();

    string_write("dose\nballs do...");
	
	while(1){

	}
	return 0; //should never get here.
}