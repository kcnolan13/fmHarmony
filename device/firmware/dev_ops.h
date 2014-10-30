//Marcel Marki and Kyle Nolan
//Header File for Device Configuration and Operations Module

#ifndef OPMODES_H
#define OPMODES_H

#include "types.h"

//synchronize op mode LEDs
void sync_leds(volatile DEV_STATE *device);

//device config
void InitUSART(void);
void InitINT(void);
int prepare_device(volatile DEV_STATE *device);
void disable_gps(void);
void enable_gps(void);
void sync_gps_data(volatile DEV_STATE *device, GPS_DATA *gps_data);

//update utilities
int detectSerialStart(volatile DEV_STATE *device);
int detectSerialEnd(volatile DEV_STATE *device);
char getChar(volatile DEV_STATE *device);
char peekChar(volatile DEV_STATE *device);
int terminate_serial(volatile DEV_STATE *device, DATABASE *fm_stations, int flag);

//modes of operation
void wipe_eeprom(volatile DEV_STATE *device);
void show_nearest_station(volatile DEV_STATE *device, DATABASE *fm_stations, GPS_DATA *gps_data);
void wait_for_update(volatile DEV_STATE *device);
void print_eeprom_station_contents(volatile DEV_STATE *device, DATABASE *fm_stations);
void print_eeprom_contents(volatile DEV_STATE *device, DATABASE *fm_stations, int start_addr, int end_addr);
void print_all_known_stations(volatile DEV_STATE *device, DATABASE *fm_stations);
void print_all_callsigns(volatile DEV_STATE *device, DATABASE *fm_stations);
void print_callsign(DATABASE *fm_stations, int station_index);
void print_station(volatile DEV_STATE *device, DATABASE *fm_stations, int index);
void print_gps_data_concise(volatile DEV_STATE *device, GPS_DATA *gps_data);
void print_gps_data(volatile DEV_STATE *device, GPS_DATA *gps_data);
void print_raw_gps_data(volatile DEV_STATE *device);
void test_earth_distance(void);
void wait_for_gps_lock(volatile DEV_STATE *device, GPS_DATA *gps_data);
int check_database_integrity(DATABASE *fm_stations);

#endif