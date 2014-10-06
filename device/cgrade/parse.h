// Marcel Marki & Kyle Nolan
// GPS Parsing Module Library File

#ifndef PARSE_H
#define PARSE_H

int merge_nmea(char in_char);
int parse_nmea(char *in_sent);
int reallocate(char *in_string, int bytenum);
int tag_check(char *in_sent);
char * strtok_single (char * str, char const * delims);
char * sent_prep(char* in_sent);

//Smartly manipulate the NMEA string based on input value.
int merge_nmea(char in_char);

//Parse the NMEA String
int parse_nmea(char *in_sent);

//function to check if $GPRMC tag is present
int tag_check(char *in_sent);

//Free and reallocate string memory to clear string.
int reallocate(char *in_string, int byte_num);

//Tokenize the String (one token per function call)
char *strtok_single (char * in_str, char const * delims);

//Prepare the Sentence for Parsing
char * sent_prep(char* in_sent);

#endif