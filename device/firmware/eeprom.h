//Marcel Marki and Kyle Nolan
//Header File for EEPROM Module

#ifndef EEPROM_H
#define EEPROM_H

//EEPROM Operations
int my_eeprom_read_int(int address);
char my_eeprom_read_char(int address);
float my_eeprom_read_float(int address);
void my_eeprom_read_string(char *dest, int address, int num_chars);

#endif