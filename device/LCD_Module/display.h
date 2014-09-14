//Marcel Marki and Kyle Nolan
//Header File for LDC Module

#ifndef DISPLAY_H
#define DISPLAY_H

//Display On, Cursor On, Blink On.
int lcd_cursor();

//Write Data to DB4 to DB7
void letterwrite(int a, int b, int c, int d);

//  Write Data to other bits
void datarw();

//Write A String to the LCD
void string_write(char *mystring);

//Retrun Current Address
int get_current_address();

//Read Data From Busy Flag
int instruction_read();

//Write A Character to the LCD
void char_write(char mychar);

// Write Data to DDRAM
void data_write(uint8_t val);

//Write Data to Pins DB4 to DB7
void write_db74(int DB7_val, int DB6_val, int DB5_val, int DB4_val);

#endif