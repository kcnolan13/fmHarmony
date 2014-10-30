// Marcel Marki & Kyle Nolan
// Geolocation Module Library File

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
int parse_nmea(volatile DEV_STATE *device, volatile char *in_sent, char * volatile *raw_gps_data){

	int l = 0, m =0, k=0;
	char* token;

	//null out the first raw_gps_data field
	for (m=0; m<GPS_FIELD_LEN; m++)
	{
		raw_gps_data[0][m] = '\0';
	}

	//ditch the first token
	token = strtok_single((char *)in_sent, ",");

	//auto-insert $GPRMC (it passed the tag check already)
	char token2[6] = "$GPRMC";
	strncpy(raw_gps_data[0],token2,6);

	//copy over the rest of the tokens
	while((token) && (l<NUM_GPS_FIELDS-1)) 
	{
		l++;

		//null out the next raw_gps_data field
		for (m=0; m<GPS_FIELD_LEN; m++)
		{
			raw_gps_data[l][m] = '\0';
		}

		token = strtok_single(NULL, ",");
		strcpy(raw_gps_data[l], token);
	}

	//clear the rxBuffer
    for (k=0; k<GPS_RX_BUFFER_SIZE; k++)
        device->gps_rxBuffer[k]='\0';
    device->gps_rxCount = 0;

	return 0;
}

//Make sure the NMEA sentence leads with $GPRMC
int tag_check(volatile DEV_STATE *device){
	char token[6] = "$GPRMC";
	int i;
	for (i=0; i<6; i++)
	{
		if (device->gps_rxBuffer[i] != token[i])
			return 0;
	}
	return 1;
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

//null out part of a string
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
	double slice;

	//16-point compass 
	char *str_bearings[] = {"N  ", "NNE", "NE ", "ENE", "E  ", "ESE", "SE ", "SSE", "S  ", "SSW", "SW ", "WSW", "W  ", "WNW", "NW ", "NNW"};

	wipe_chars(gps_data->msg_type,8);
	strcpy(gps_data->msg_type,raw_gps_data[0]);

	//format time
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

	//format latitude
	gps_data->lat = 0;
	temp = lat2dec(raw_gps_data[3], raw_gps_data[4][0]);

	//handle invalid latitudes
	if ((temp >= -90)&&(temp <= 90))
		gps_data->lat = temp;

	//format longitude
	gps_data->lon = 0;
	temp = lon2dec(raw_gps_data[5], raw_gps_data[6][0]);

	//handle invalid longitudes
	if ((temp >= -180)&&(temp <= 180))
		gps_data->lon = temp;

	gps_data->speed = 0;
	gps_data->speed = (float)strtod(raw_gps_data[7],NULL);

	gps_data->course = 0;
	gps_data->course = (float)strtod(raw_gps_data[8],NULL);

	//format date
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

	//compute the course string (16-point compass)
	slice = gps_data->course/360*16;

	if ((slice<=0.5)||(slice>=15.5))
	{
		//course is NORTH
		strncpy(gps_data->str_course, str_bearings[0], 3);
	} else {
		//course index complies with (int)(slice+0.5) convention
		strncpy(gps_data->str_course, str_bearings[(int)(slice+0.5)], 3);
	}

}

//find the closest station (index) to a lat/lon coordinate pair
int get_nearest_station(STATION *all_stations, int num_stations, float lat, float lon)
{
    float min_dist = -1;
    int station_index = -1, i;

    //compute earth distance to all stations --> remember min distance
    for (i=0; i<num_stations; i++)
    {
        float temp = earth_distance(lat, lon, all_stations[i].lat, all_stations[i].lon);
        if ((temp < min_dist)||(min_dist==-1))
        {
            //new closest station
            station_index = i;
            min_dist = temp;
        }
    }
    //return closest station index
    return station_index;
}

//find the distance from the user to a particular station
float my_distance_to_station(GPS_DATA * gps_data, STATION *all_stations, int station_index)
{
    return earth_distance(gps_data->lat, gps_data->lon, all_stations[station_index].lat, all_stations[station_index].lon);
}

//calculate the absolute and relative bearings to the nearest station 
int calculate_bearings(GPS_DATA *gps_data, DATABASE *fm_stations)
{
	double y, x, bearing, lat1, lat2, lon1, lon2;
	float slice;

	//16-point compass 
	char *str_bearings[] = {"N  ", "NNE", "NE ", "ENE", "E  ", "ESE", "SE ", "SSE", "S  ", "SSW", "SW ", "WSW", "W  ", "WNW", "NW ", "NNW"};

	//all angles must be in radians
	lat1 = to_radians((double)gps_data->lat);
	lon1 = to_radians((double)gps_data->lon);
	lat2 = to_radians((double)fm_stations->all_stations[fm_stations->nearest_station].lat);
	lon2 = to_radians((double)fm_stations->all_stations[fm_stations->nearest_station].lon);

	//use the Forward Azimuth Formula
	y = sin(lon2 - lon1)*cos(lat2);
	x = cos(lat1)*sin(lat2) - sin(lat1)*cos(lat2)*cos(lon2 - lon1);

	//finish computing the absolute bearing to the nearest station
	bearing = to_degrees(atan2(y, x));

	//keep absolute bearing between 0-360
	if (bearing < 0)
		bearing += 360;

	//save bearing into the DEV_STATE struct
	gps_data->abs_bearing_nearest = (float)bearing;

	//compute the relative bearing to nearest station
	gps_data->rel_bearing_nearest = gps_data->abs_bearing_nearest - gps_data->course;

	//keep relative bearing within 0-360
	if (gps_data->rel_bearing_nearest < 0)
		gps_data->rel_bearing_nearest += 360;

	//compute the absolute bearing string (16-point compass)
	slice = gps_data->abs_bearing_nearest/360*16;

	if ((slice<=0.5)||(slice>=15.5))
	{
		//bearing is NORTH
		strncpy(gps_data->str_abs_bearing_nearest, str_bearings[0], 3);
	} else {
		//bearing index complies with (int)(slice+0.5) convention
		strncpy(gps_data->str_abs_bearing_nearest, str_bearings[(int)(slice+0.5)], 3);
	}

	return 1;
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

    //apply haversine formula
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

//convert an angle from radians to degrees
double to_degrees(double radian_angle)
{
	return 180*radian_angle/(M_PI);
}

//make sure there is valid GPS data to work with
int gps_locked(GPS_DATA *gps_data)
{
    int i;

    //message param should not contain nulls
    for (i=0; i<4; i++)
    {
        if (gps_data->msg_type[i]=='\0')
            return 0;
    }

    //time param should not contain nulls
    for (i=0; i<4; i++)
    {
        if (gps_data->utc_time[i]=='\0')
            return 0;
    }

    //lat and lon should not be 0,0
    if ((gps_data->lat==0)||(gps_data->lon==0))
        return 0;

    //checksum should exist
    if (gps_data->checksum[0] != '*')
        return 0;

    //course should be between 0 and 360
    if ((gps_data->course < 0)||(gps_data->course>=360))
    	return 0;

    return 1;

}