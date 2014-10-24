// Marcel Marki & Kyle Nolan
// GPS Module Library File

#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include <math.h>
#include "geolocation.h"
#include "lcd.h"

//Parse the NMEA nmea_string and populate the gps_data fields
int parse_nmea(volatile char *in_sent, char *gps_data[13]){

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

//use the raw gps_data fields to populate the UserData struct
void update_user_gps_data(char *gps_data[13], UserData *user)
{
	float temp;
	wipe_chars(user->msg_type,8);
	strcpy(user->msg_type,gps_data[0]);

	wipe_chars(user->utc_time,8);
	user->utc_time[0] = gps_data[1][0];
	user->utc_time[1] = gps_data[1][1];
	user->utc_time[2] = ':';
	user->utc_time[3] = gps_data[1][2];
	user->utc_time[4] = gps_data[1][3];
	user->utc_time[5] = ':';
	user->utc_time[6] = gps_data[1][4];
	user->utc_time[7] = gps_data[1][5];

	user->nrw = gps_data[2][0];

	user->lat = 0;
	temp = lat2dec(gps_data[3], gps_data[4][0]);

	//handle invalid latitudes
	if ((temp >= -90)&&(temp <= 90))
		user->lat = temp;

	user->lon = 0;
	temp = lon2dec(gps_data[5], gps_data[6][0]);

	//handle invalid longitudes
	if ((temp >= -180)&&(temp <= 180))
		user->lon = temp;

	user->speed = 0;
	user->speed = (float)strtod(gps_data[7],NULL);

	user->course = 0;
	user->course = (float)strtod(gps_data[8],NULL);

	wipe_chars(user->date,8);
	user->date[0] = gps_data[9][2];
	user->date[1] = gps_data[9][3];
	user->date[2] = '/';
	user->date[3] = gps_data[9][0];
	user->date[4] = gps_data[9][1];
	user->date[5] = '/';
	user->date[6] = gps_data[9][4];
	user->date[7] = gps_data[9][5];

	wipe_chars(user->mag_var,8);
	strcpy(user->mag_var,gps_data[10]);

	user->mode = gps_data[12][0];

	wipe_chars(user->checksum,3);
	user->checksum[0] = gps_data[12][1];
	user->checksum[1] = gps_data[12][2];
	user->checksum[2] = gps_data[12][3];
}

//---- GEO-POSITIONAL ALGORITHMS ----//

//find the closest station to a lat/lon coordinate pair
int get_nearest_station(Station *all_stations, int num_stations, float lat, float lon)
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
float my_distance_to_station(UserData * user, Station *all_stations, int station_index)
{
    return earth_distance(user->lat, user->lon, all_stations[station_index].lat, all_stations[station_index].lon);
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
int gps_locked(UserData *user)
{
    int i;
    for (i=0; i<4; i++)
    {
        if (user->msg_type[i]=='\0')
            return 0;
    }

    for (i=0; i<4; i++)
    {
        if (user->utc_time[i]=='\0')
            return 0;
    }

    if ((user->lat==0)||(user->lon==0))
        return 0;

    if (user->checksum[0] != '*')
        return 0;

    return 1;

}