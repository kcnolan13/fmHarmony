//Marcel Marki and Kyle Nolan
//Header File for Constants and Structs Used By Multiple fmHarmony Libraries

#ifndef TYPES_H
#define TYPES_H

//modes of operation
#define NUM_MODES 6
#define MD_NORMAL 0
#define MD_GPS 1
#define MD_DATABASE 2
#define MD_GPS_LONG 3
#define MD_DEBUG 4
#define MD_DEBUG2 5
#define MD_UPDATE_REQUIRED 6
#define MD_UPDATE 7

//FM station offsets in memory
#define STATION_BLOCKSIZE 28
#define FIRST_STATION_OFFSET 1

//serial update parameters
#define RX_BUFFER_SIZE  128
#define SERIAL_TIMEOUT 50000
#define FL_SUCCESS 0
#define FL_FAIL 1

//gps constants
#define NUM_GPS_FIELDS 13
#define GPS_FIELD_LEN 16
#define GPS_RX_BUFFER_SIZE 80
#define NUM_NEAREST 3

//Device State
typedef struct device_state {
    double serial_timer;
    int updating;
    int eeprom_index;
    int op_mode;
    int op_mode_prior;
    char serialStartChar;
    char serialEndChar;
    char rxBuffer[RX_BUFFER_SIZE];
    uint8_t rxReadPos;
    uint8_t rxWritePos;
    char serial_history[3];
    int gps_rxCount;
    char gps_rxBuffer[GPS_RX_BUFFER_SIZE];
    char *raw_gps_data[NUM_GPS_FIELDS];
    int gps_update_trigger;
    int button_pressable;
    int blinker1;
    int blinker2;
    int blinker3;
    int blinking;
} DEV_STATE;

//FM Station Structure
typedef struct station {
    char callsign[8];
    float freq;
    float lat;
    float lon;
    float erp;
    float haat;
} STATION;

//Houses Entire FM Station Database
typedef struct station_database {
	STATION *all_stations;
	int num_stations;
    int nearest_station;
	int nearest_stations[NUM_NEAREST][2]; //1st dim = station index, 2nd dim = distance (km)
	int corrupted;
} DATABASE;

//User GPS Data Structure
typedef struct gps_data {
    char msg_type[8];
    char utc_time[8];
    char nrw;
    float lat;
    float lon;
    float speed;
    float course;
    char date[8];
    char mag_var[8];
    char mode;
    char checksum[3];
    float abs_bearing_nearest[NUM_NEAREST];
    float rel_bearing_nearest[NUM_NEAREST];
    char str_abs_bearing_nearest[NUM_NEAREST][3]; //1st dim matches station index, 2nd dim = chars
    char str_course[3];
} GPS_DATA;

#endif