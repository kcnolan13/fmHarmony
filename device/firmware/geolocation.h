// Marcel Marki & Kyle Nolan
// GPS Module Library File

#ifndef GPS_H
#define GPS_H

//Generic FM Station Structure
typedef struct station {
    char callsign[8];
    float freq;
    float lat;
    float lon;
    float erp;
    float haat;
} Station;

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
int parse_nmea(volatile char *in_sent,  char *gps_data[13]);

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

//find the closest station to a given lat/lon pair
int get_nearest_station(Station *all_stations, int num_stations, float lon, float lat);

//find the closest station to the user
float my_distance_to_station(UserData *user, Station *all_stations, int station_index);

//use the haversine fomula to calculate the great-circle distance between two coordinate pairs
float earth_distance(float lat1, float lon1, float lat2, float lon2);

//convert an angle from degrees to radians (needed for the haversine formula)
double to_radians(double decimal_angle);

//has the UserData struct been populated with enough GPS data?
int gps_locked(UserData *user);

#endif