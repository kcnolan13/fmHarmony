//Marcel Marki and Kyle Nolan
//Header File for Database Module

#ifndef DATABASE_H
#define DATABASE_H

#include "types.h"

//read a single-byte integer from an EEPROM address
int my_eeprom_read_int(int address);

//read a char from an EEPROM address
char my_eeprom_read_char(int address);

//read a 4-byte float from an EEPROM address
float my_eeprom_read_float(int address);

//read a string from an EEPROM address
void my_eeprom_read_string(char *dest, int address, int num_chars);

//load the FM Stations database from EEPROM into program memory
int database_load(DATABASE *fm_stations);

//free the FM Stations database from program memory
int database_free(DATABASE *fm_stations);

#endif