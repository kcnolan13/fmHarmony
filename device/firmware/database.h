//Marcel Marki and Kyle Nolan
//Header File for Database Module

#ifndef DATABASE_H
#define DATABASE_H

#include "types.h"

//EEPROM Operations
int my_eeprom_read_int(int address);
char my_eeprom_read_char(int address);
float my_eeprom_read_float(int address);
void my_eeprom_read_string(char *dest, int address, int num_chars);

int database_load(DATABASE *fm_stations);
int database_free(DATABASE *fm_stations);

#endif