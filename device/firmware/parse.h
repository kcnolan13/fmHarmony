// Marcel Marki & Kyle Nolan
// GPS Parsing Module Library File

#ifndef PARSE_H
#define PARSE_H

//Parse the NMEA nmea_string and populate the gps_data fields
int parse_nmea(char *in_sent,  char *gps_data[13]);

//Make sure the NMEA sentence leads with $GPRMC
int tag_check(char *in_sent);

//Tokenize the String (one token per function call)
char *strtok_single (char * in_str, char const * delims);

#endif