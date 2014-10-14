// Marcel Marki & Kyle Nolan
// GPS Parsing Module Library File

#include "parse.h"
#include "display.h"
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

//Parse the NMEA nmea_string and populate the gps_data fields
int parse_nmea(char *in_sent, char *gps_data[13]){

	int i = 0, j=0;
	char* token;

	//copy over the first token
	token = strtok_single((char *)in_sent, ",");
	strcpy(gps_data[0], token);

	//null out the rest of the gps_data field
	for (j=strlen(token); j<16; j++)
	{
		gps_data[0][j] = '\0';
	}

	//copy over the rest of the tokens
	while(token) 
	{
		i++;
		token = strtok_single(NULL, ",");
		strcpy(gps_data[i], token);
		//null out the rest of the gps_data field
		for (j=strlen(token); j<16; j++)
		{
			gps_data[i][j] = '\0';
		}
	}

	return 0;
}

//Make sure the NMEA sentence leads with $GPRMC
int tag_check(char *in_sent){
	char token[6] = "$12345";
	int x = 0;

	for (x = 0; x <6; x ++){
	        token[x] = in_sent[x];
	}
	if (strcmp(token,"$GPRMC") ==0) 
		return 1;
	
	return 0;
}


//Tokenize the nmea_string (one token per function call)
char *strtok_single (char * in_str, char const * delims)
{
  static char  * src = NULL;

  char  *  p,  * ret = 0;

  if (in_str != NULL)
    src = in_str;

  if (src == NULL)
    return NULL;

  if ((p = strpbrk (src, delims)) != NULL) {
    *p  = 0;
    ret = src;
    src = ++p;
  }

  return ret;
}