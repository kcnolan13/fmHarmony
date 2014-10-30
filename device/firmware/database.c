// Marcel Marki & Kyle Nolan
// Database Operations Library File

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

#include "database.h"
#include "lcd.h"

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

//load the FM Stations database from EEPROM into program memory
int database_load(DATABASE *fm_stations)
{
    int i;
    //figure out how many stations there are by reading the first number written to EEPROM
    fm_stations->num_stations = my_eeprom_read_int(0);

    //EEPROM defaults to 255 -- this means no station data in EEPROM
    if (fm_stations->num_stations==255)
        fm_stations->num_stations = 0;

    _delay_ms(500);
    lcd_init();
    string_write("importing ");
    string_write_int(fm_stations->num_stations,3);
    string_write("\nstations...");

    //allocate memory for all the station structures
    fm_stations->all_stations = (STATION *)malloc(fm_stations->num_stations*sizeof(STATION));
    if (fm_stations->all_stations == NULL)
    {
            lcd_init();
            string_write("bad malloc");
            //database failed to load
            return 0;
    }

    //load in the stations one by one into the all_stations array of Station structs
    for (i=0; i<fm_stations->num_stations; i++)
    {
        int start = FIRST_STATION_OFFSET+i*STATION_BLOCKSIZE;
        my_eeprom_read_string(fm_stations->all_stations[i].callsign,start,8); start += 8;
        fm_stations->all_stations[i].freq = my_eeprom_read_float(start); start += 4;
        fm_stations->all_stations[i].lat = my_eeprom_read_float(start); start += 4;
        fm_stations->all_stations[i].lon = my_eeprom_read_float(start); start += 4;
        fm_stations->all_stations[i].erp = my_eeprom_read_float(start); start += 4;
        fm_stations->all_stations[i].haat = my_eeprom_read_float(start); start += 4;
    }

    //database loaded successfully
    return 1;
}

//free the FM Stations database from program memory
int database_free(DATABASE *fm_stations)
{

    free(fm_stations->all_stations);
    fm_stations->all_stations = NULL;

    return 1;
}