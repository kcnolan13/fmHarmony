//Marcel Marki and Kyle Nolan
//Header File for Device Configuration and Common Operations Module

#ifndef OPMODES_H
#define OPMODES_H

#include "types.h"

//synchronize op mode LEDs
void sync_leds(volatile DEV_STATE *device);

//Initialize USART
void InitUSART(void);

//Set Up External Interrupt 2
void InitINT(void);

//set up GPIO, initialize interrupts, serial comm, and LCD
int prepare_device(volatile DEV_STATE *device);

//turn off GPS intterupts, receiver, and transmitter
void disable_gps(void);

//turn on GPS interrupts, receiver, and transmitter
void enable_gps(void);

//parse available GPS data and pull formatted params into the GPS_DATA struct
void sync_gps_data(volatile DEV_STATE *device, GPS_DATA *gps_data);

//catch the serial update start sequence
int detectSerialStart(volatile DEV_STATE *device);

//catch the serial update end sequence
int detectSerialEnd(volatile DEV_STATE *device);

//read a char from the serial update buffer
char getChar(volatile DEV_STATE *device);

//peek at the next char in the serial update buffer
char peekChar(volatile DEV_STATE *device);

//terminate the serial update with a certain status
int terminate_serial(volatile DEV_STATE *device, DATABASE *fm_stations, int flag);

//modes of operation
void wipe_eeprom(volatile DEV_STATE *device);

//compute the nearest station and display pertinent, formatted info
void show_nearest_station(volatile DEV_STATE *device, DATABASE *fm_stations, GPS_DATA *gps_data);

//hold device state and wait for database update
void wait_for_update(volatile DEV_STATE *device);

//print the EEPROM contents for all stations
void print_eeprom_station_contents(volatile DEV_STATE *device, DATABASE *fm_stations);

//print the EEPROM contents for an address range
void print_eeprom_contents(volatile DEV_STATE *device, DATABASE *fm_stations, int start_addr, int end_addr);

//print the information held for all stations to the screen
void print_all_known_stations(volatile DEV_STATE *device, DATABASE *fm_stations);

//quickly print all known callsigns to the screen
void print_all_callsigns(volatile DEV_STATE *device, DATABASE *fm_stations);

//print the callsign for a given station index to the LCD
void print_callsign(DATABASE *fm_stations, int station_index);

//print the informatoin held for a single station to the LCD
void print_station(volatile DEV_STATE *device, DATABASE *fm_stations, int index);

//print the formatted data stored in the GPS_DATA struct to the screen
void print_gps_data(volatile DEV_STATE *device, GPS_DATA *gps_data);

//print a concise version of the formatted GPS data to the screen
void print_gps_data_concise(volatile DEV_STATE *device, GPS_DATA *gps_data);

//print the raw gps data in the gps_data string array to the screen
void print_raw_gps_data(volatile DEV_STATE *device);

//crosscheck the earth-distance formula with a few known distances
void test_earth_distance(void);

//check for database corruption
int check_database_integrity(DATABASE *fm_stations);

#endif