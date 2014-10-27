// Marcel Marki & Kyle Nolan
// GPS Module Library File

#include <avr/io.h>
#include <util/delay.h>
#include <avr/sfr_defs.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <inttypes.h>
#include <avr/eeprom.h>

#include "geolocation.h"
#include "lcd.h"

//Parse the NMEA nmea_string and populate the raw_gps_data fields
int parse_nmea(volatile char *in_sent, char * volatile *raw_gps_data){

	int l = 0, m =0;
	char* token;

	//null out the raw_gps_data field
	for (m=0; m<GPS_FIELD_LEN; m++)
	{
		raw_gps_data[0][m] = '\0';
	}

	//copy over the first token
	token = strtok_single((char *)in_sent, ",");
	strcpy(raw_gps_data[0], token);

	//copy over the rest of the tokens
	while((token) && (l<NUM_GPS_FIELDS-1)) 
	{
		l++;

		//null out the raw_gps_data field
		for (m=0; m<GPS_FIELD_LEN; m++)
		{
			raw_gps_data[l][m] = '\0';
		}

		token = strtok_single(NULL, ",");
		strcpy(raw_gps_data[l], token);
	}

	return 0;
}

//Make sure the NMEA sentence leads with $GPRMC
int tag_check(volatile char *in_sent){
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

void wipe_chars(char *str, int num)
{
	int i;
	for (i=0; i<num; i++)
	{
		str[i] = '\0';
	}
}

//convert GPRMC-style latitude to decimal degrees
float lat2dec(char lat[9], char N_indicator)
{
	//GPRMC latitude = ddmm.mmmm

	//obtain degrees
	float degrees = (float)(((int)(lat[0]-'0'))*10+(int)(lat[1]-'0'));

	//obtain minutes
	char temp[7];
	strncpy(temp,(char *)&lat[2],7);

	float minutes = strtod(temp,NULL);

	//do the math
	float result = degrees + minutes*1/60;

	//adjust for N/S indicator
	if (N_indicator == 'S')
		result *= -1;

	return result;

}

//convert GPRMC-style longitude to decimal degrees
float lon2dec(char lon[10], char E_indicator)
{
	//GPRMC longitude = dddmm.mmmm

	//obtain degrees
	float degrees = (float)((int)(lon[0]-'0')*100+(int)(lon[1]-'0')*10+(int)(lon[2]-'0'));

	//obtain minutes
	char temp[7];
	strncpy(temp,(char *)&lon[3],7);

	float minutes = strtod(temp,NULL);

	//do the math
	float result = degrees + minutes*1/60;

	//adjust for N/S indicator
	if (E_indicator == 'W')
		result *= -1;

	return result;
}

//use the raw gps_data fields to populate the GPS_DATA struct
void update_user_gps_data(char * volatile *raw_gps_data, GPS_DATA *gps_data)
{
	float temp;
	wipe_chars(gps_data->msg_type,8);
	strcpy(gps_data->msg_type,raw_gps_data[0]);

	wipe_chars(gps_data->utc_time,8);
	gps_data->utc_time[0] = raw_gps_data[1][0];
	gps_data->utc_time[1] = raw_gps_data[1][1];
	gps_data->utc_time[2] = ':';
	gps_data->utc_time[3] = raw_gps_data[1][2];
	gps_data->utc_time[4] = raw_gps_data[1][3];
	gps_data->utc_time[5] = ':';
	gps_data->utc_time[6] = raw_gps_data[1][4];
	gps_data->utc_time[7] = raw_gps_data[1][5];

	gps_data->nrw = raw_gps_data[2][0];

	gps_data->lat = 0;
	temp = lat2dec(raw_gps_data[3], raw_gps_data[4][0]);

	//handle invalid latitudes
	if ((temp >= -90)&&(temp <= 90))
		gps_data->lat = temp;

	gps_data->lon = 0;
	temp = lon2dec(raw_gps_data[5], raw_gps_data[6][0]);

	//handle invalid longitudes
	if ((temp >= -180)&&(temp <= 180))
		gps_data->lon = temp;

	gps_data->speed = 0;
	gps_data->speed = (float)strtod(raw_gps_data[7],NULL);

	gps_data->course = 0;
	gps_data->course = (float)strtod(raw_gps_data[8],NULL);

	wipe_chars(gps_data->date,8);
	gps_data->date[0] = raw_gps_data[9][2];
	gps_data->date[1] = raw_gps_data[9][3];
	gps_data->date[2] = '/';
	gps_data->date[3] = raw_gps_data[9][0];
	gps_data->date[4] = raw_gps_data[9][1];
	gps_data->date[5] = '/';
	gps_data->date[6] = raw_gps_data[9][4];
	gps_data->date[7] = raw_gps_data[9][5];

	wipe_chars(gps_data->mag_var,8);
	strcpy(gps_data->mag_var,raw_gps_data[10]);

	gps_data->mode = raw_gps_data[12][0];

	wipe_chars(gps_data->checksum,3);
	gps_data->checksum[0] = raw_gps_data[12][1];
	gps_data->checksum[1] = raw_gps_data[12][2];
	gps_data->checksum[2] = raw_gps_data[12][3];
}

//---- GEO-POSITIONAL ALGORITHMS ----//

//find the closest station to a lat/lon coordinate pair
int get_nearest_station(STATION *all_stations, int num_stations, float lat, float lon)
{
    float min_dist = -1;
    int station_index = -1, i;

    //compute earth distance to all stations --> track min distance
    for (i=0; i<num_stations; i++)
    {
        float temp = earth_distance(lat, lon, all_stations[i].lat, all_stations[i].lon);
        if ((temp < min_dist)||(min_dist==-1))
        {
            //this is the closest station at the moment
            station_index = i;
            min_dist = temp;
        }
    }

    return station_index;
}

//find the distance from the user to a particular station
float my_distance_to_station(GPS_DATA * gps_data, STATION *all_stations, int station_index)
{
    return earth_distance(gps_data->lat, gps_data->lon, all_stations[station_index].lat, all_stations[station_index].lon);
}

//use the haversine fomula to calculate the great-circle distance between two coordinate pairs
float earth_distance(float lat1, float lon1, float lat2, float lon2)
{
    //radius of earth in km
    double R = 6371;

    double theta1 = to_radians((double)lat1);
    double theta2 = to_radians((double)lat2);

    double dtheta = to_radians((double)lat2 - (double)lat1);
    double dlambda = to_radians((double)lon2 - (double)lon1);

    double a = sin(dtheta/2)*sin(dtheta/2) + cos(theta1)*cos(theta2)*sin(dlambda/2)*sin(dlambda/2);
    double c = 2*atan2(sqrt(a), sqrt(1-a));
    double distance = R*c;

    return (float)distance;
}

//convert an angle from degrees to radians
double to_radians(double decimal_angle)
{
    return (M_PI)*decimal_angle/180;
}

//make sure there is valid GPS data to work with
int gps_locked(GPS_DATA *gps_data)
{
    int i;
    for (i=0; i<4; i++)
    {
        if (gps_data->msg_type[i]=='\0')
            return 0;
    }

    for (i=0; i<4; i++)
    {
        if (gps_data->utc_time[i]=='\0')
            return 0;
    }

    if ((gps_data->lat==0)||(gps_data->lon==0))
        return 0;

    if (gps_data->checksum[0] != '*')
        return 0;

    return 1;

}