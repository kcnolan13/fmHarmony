// Marcel Marki & Kyle Nolan
#include <avr/io.h>
#include <avr/delay.h>
#include <avr/sfr_defs.h>
#include <stdio.h>
#include <string.h>
#include "display.h"

//Map LED pins to AVR GPIO ports
#define RS	    PA0
#define RW	    PA1
#define E 	    PA2
#define DB4 	PA3
#define DB5 	PA4
#define DB6 	PA5
#define DB7 	PA6


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