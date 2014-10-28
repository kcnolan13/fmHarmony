// Marcel Marki & Kyle Nolan
// EEPROM Operations Library File

#include <avr/io.h>
#include <util/delay.h>
#include <avr/sfr_defs.h>
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <inttypes.h>
#include <avr/eeprom.h>
#include <math.h>

//---- EEPROM Operations ----//

//read a single-byte integer from an EEPROM address
int my_eeprom_read_int(int address)
{
    int temp_num = ((int)eeprom_read_byte((uint8_t *)address));
    return (temp_num);
}

//read a char from an EEPROM address
char my_eeprom_read_char(int address)
{
    return (char)eeprom_read_byte((uint8_t *)address);
}

//read a 4-byte float from an EEPROM address
float my_eeprom_read_float(int address)
{
    return (float)(eeprom_read_float((const float *)address));
}

//read a string from an EEPROM address
void my_eeprom_read_string(char *dest, int address, int num_chars)
{
    eeprom_read_block((void *)dest,(const void *)address,num_chars);
}