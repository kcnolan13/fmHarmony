// Marcel Marki & Kyle Nolan
// GPS Parsing Module Library File

#ifndef GPS_H
#define GPS_H

//User GPS Data Structure
typedef struct user_data {
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
} UserData;

//Parse the NMEA nmea_string and populate the gps_data fields
int parse_nmea(char *in_sent,  char *gps_data[13]);

//Make sure the NMEA sentence leads with $GPRMC
int tag_check(volatile char *in_sent);

//Tokenize the String (one token per function call)
char *strtok_single (char * in_str, char const * delims);

//use the raw gps_data fields to populate the UserData struct
void update_user_gps_data(char *gps_data[13], UserData *user);

//used to null out strings
void wipe_chars(char *str, int num);

//convert GPRMC-style latitude to decimal degrees
float lat2dec(char lat[9], char N_indicator);

//convert GPRMC-style longitude to decimal degrees
float lon2dec(char lon[10], char E_indicator);

#endif