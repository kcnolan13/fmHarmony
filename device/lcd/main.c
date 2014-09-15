// Marcel Marki & Kyle Nolan
#include <avr/io.h>
#include <avr/delay.h>
#include <avr/sfr_defs.h>
#include <stdio.h>
#include <string.h>

#include "display.h"




int main (int argc, char *argv[])
{
	//Intitialize LCD. Set Blinking cursor.
	lcd_cursor();

    
	//Write ECE to screen
    string_write("Library Victory");
	
	while(1){

	}
	return 0; //should never get here.
}