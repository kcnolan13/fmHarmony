// M. Marki K. Nolan
// Program for sending log file serially to fmHarmony device
// arduino-serial library courtesy of Tod E. Kurt
// The open_port() and send_string() functions of this program were
// largely inspired by Tod E. Kurt's arduino-serial.c program.

#include <stdio.h>    // Standard input/output definitions
#include <stdlib.h>
#include <string.h>   // String function definitions
#include <unistd.h>   // for usleep()
#include <getopt.h>
#include <dirent.h>     //for finding serial transmitter

#include "arduino-serial-lib.h"

#define BUF_MAX 256
#define UPLOAD_SPEED 5

int open_port(void);
int send_string(char* in_string);
int parse_line(char* in_line);
char *strtok_single(char * in_str, char const * delims);
char * line_prep(char* in_line);
void strip_newline( char *str, int size );
int send_float(char* in_string);
int send_byte (uint8_t in_byte);
void find_serial_transmitter(void);

int fd = -1;
char serialport[BUF_MAX];
char *optarg;
int baudrate = 9600;
int num_stations = 0;
int stations_uploaded = 0;

union float2bytes { 
    float f; 
    char b[sizeof(float)];
};

int main (int argc, char *argv[])
{
    optarg = (char *)malloc(100*sizeof(char));

    //fill optarg with the path to the serial transmitter
    find_serial_transmitter();

    FILE *log_file;
    char buf[1000];
    char *str_array[140]={"\0"};
    int i=0;

    //Open the FM stations log file that will become the device database
    log_file = fopen("log.txt","r");
    if (!log_file){
        printf("ERROR: Could not open FM Stations log file for reading.\n");
        fflush(stdout);
        return -1;
    }

    //create a connection over the virtual serial port
    open_port();

    //send the start sequence
    printf("START SEQUENCE:\n"); fflush(stdout);
    send_string("$$$");

    printf("\n\n");

    //wait a while to make sure the device has detected the start sequence and is ready for data
    printf("giving device plenty of time...\n\n"); fflush(stdout);
    usleep(5000000);

    //parse the log file line by line
    while (fgets(buf,1000, log_file)!=NULL) 
    {
        //keep track of each line
        str_array[i] = buf;
        strip_newline( str_array[i], 210);

        //prepare, parse, and transmit the data within each line
        parse_line(line_prep(str_array[i]));
        i++;
    }

    //close the log file
    fclose(log_file);

    //make sure the device has had enough time to write all the data before sending end sequence
    printf("giving device plenty of time...\n\n"); fflush(stdout);
    usleep(4000000);

    //send the end sequence
    printf("END SEQUENCE:\n"); fflush(stdout);
    send_string("^^^");
    printf("\n\n");

    //terminate the virtual serial port connection
    serialport_close(fd);
    return 0;
}

int open_port(void){
    if( fd!=-1 ) {
        serialport_close(fd);
        printf("closed port %s\n",serialport); fflush(stdout);
    }
    strcpy(serialport,optarg);
    fd = serialport_init(optarg, baudrate);
    if( fd==-1 ){
        printf("couldn't open port: %s\n", serialport); fflush(stdout);
        return -1;
    }
    printf("opened port! %s\n\n",serialport); fflush(stdout);
    serialport_flush(fd);
    
    return 0;
}

int send_string(char* in_string){

    //printf("in send_string\n");

    int rc = -1;

    if( fd == -1 ){ 
        printf("serial port not opened\n"); fflush(stdout);
        return -1;
    }

    //print strings that aren't spacers
    if (in_string[0] != ' ')
        printf("string:%s\n", in_string);

    rc = serialport_write(fd, in_string);
    if(rc==-1){
        printf("error writing\n"); fflush(stdout);
        return -2;
    }

    usleep(10000/UPLOAD_SPEED);

    return 0;
}

int send_float(char* in_string){
    int rc = -1;
    int i = 0;
    union float2bytes f2b;

    if( fd == -1 ){ 
        printf("serial port not opened\n"); fflush(stdout);
        return -1;
    }

    printf("float: %f -----> " ,atof(in_string));
    f2b.f = atof(in_string);

    for (i = 0; i < 4; i++ ) {
        printf("byte %d = %c, ", i, f2b.b[i]); fflush(stdout);
        
        //functionalize!
        rc = send_byte((uint8_t)f2b.b[i]); //If this doesn't work, try a loop of 1 char stringwrites.
        if(rc==-1){
            printf("error writing\n"); fflush(stdout);
            return -2;
        }
    } printf("\n");

    usleep(10000/UPLOAD_SPEED);

    return 0;
}

int send_byte (uint8_t in_byte){
    int rc = -1;

    if( fd == -1 ){ 
        printf("serial port not opened"); fflush(stdout);
        return -1;
    }
    rc = serialport_writebyte(fd, in_byte);
    if(rc==-1){
        printf("error writing"); fflush(stdout);
        return -2;
    }

    usleep(20000/UPLOAD_SPEED);

    return 0;
}

int parse_line(char * in_line){
    int i = 0;
    int initial_length = strlen(in_line);
    printf("chars parsed: %d\n", initial_length); fflush(stdout);
    char * token = strtok_single(in_line, " ");
    /* walk through other tokens */
    //The first call you use a char array which has the elements you want parsed.
    //The second time you call it you pass it NULL as the first parameter to tell function 
    //to resume from the last spot in the string. Once the first call is made your char 
    //array receives the parsed string. If you don't put NULL you would lose your place 
    //and effectivly the last part of your string.
    if (initial_length > 10){
        if (initial_length >= 200){
            //grid population
            send_byte((uint8_t)*(token));
            printf("\n\nMaine Station Grid Distribution\n");
            printf("\n---------------------------------------------------------------------------------\n");
            int col_counter = 1;
            printf("|       |       |       |       |       |       |       |       |       |       |\n");
            printf("|   %s   ", token); fflush(stdout);
            
            while(token){

                col_counter++;
                i++;
                token = strtok_single(NULL, " ");

                if ((col_counter==1)&&(strlen(token) > 0))
                    printf("|       |       |       |       |       |       |       |       |       |       |\n");

                if (i < 100) {
                    send_byte((uint8_t)*(token));
                    if (strlen(token) < 2)
                        printf("|   %s   ",token);
                    else
                        printf("|   %s  ",token);
                }

                if (col_counter > 9)
                {
                    col_counter = 0;
                    printf("|\n|       |       |       |       |       |       |       |       |       |       |\n");
                    printf("---------------------------------------------------------------------------------\n");

                     fflush(stdout);
                }

                usleep(500000/UPLOAD_SPEED);
            }
            printf("\n"); fflush(stdout);
        }
        else{
            //Station line
            stations_uploaded++;
            printf("\n---------------------------------\nUploading Station (%d of %d)\n---------------------------------\n\n",stations_uploaded, num_stations);

            //send the station callsign --> always send only 8 chars; truncate or pad if needed
            char *token2 = (char *)malloc(8*sizeof(char));

            if (strlen(token) > 8)
                strncpy(token2,token,8);
            else
                strcpy(token2,token);

            send_string(token2);
            if (strlen(token2) < 8)
            {
                int k=0;
                for (k=0; k< 8-strlen(token2); k++)
                {
                    send_string(" ");
                }
            }

            free(token2);

            //send the other station parameters
            while(token){
                i++;
                token = strtok_single(NULL, " ");
                if (i < 6) send_float(token); 
                //if (i < 6) printf("float:%s ",token); 
                usleep(200000/UPLOAD_SPEED);
            }       
        }  
    }
    else{
        num_stations = (uint8_t)atoi(token);
        send_byte((uint8_t)num_stations);
        printf("\n%d stations to upload\n", num_stations); fflush(stdout);
    }
    printf("\n");
    return 0;
}

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

char * line_prep(char* in_line){
    //concatenate
    in_line = strcat(in_line, " ");

    return in_line;
}

void strip_newline( char *str, int size )
{
    int i;
    // remove the newline character
    for (  i = 0; i < size; i++ )
    {
        if ( str[i] == '\n' )
        {
            str[i] = '\0';

            // we're done, so just exit the function by returning
            return;   
        }
    }
    // if we get all the way to here, there must not have been a newline
}

void find_serial_transmitter(void)
{
    char *common_chars = "usb-Dean_Camera_LUFA_USB-RS232_Adapter_";
    char *base_directory = "/dev/serial/by-id/";
    int num_common_chars = strlen(common_chars);

    DIR *dp;
    struct dirent *dirp;

    if ((dp = opendir(base_directory)) == NULL)
    {
        printf("ERROR: cannot open /dev/serial/by-id/\n\n"); fflush(stdout);
        exit(1);
    }

    while ((dirp = readdir(dp)) != NULL)
    {
        if (!strncmp(dirp->d_name, common_chars, num_common_chars))
        {
            printf("\nFOUND SERIAL TRANSMITTER!\n%s\n\n", dirp->d_name); fflush(stdout);
            sprintf(optarg, "%s%s",base_directory,dirp->d_name);
            closedir(dp);
            return;
        }
    }

    printf("\nERROR: cannot find serial transmitter\n\n"); fflush(stdout);
    closedir(dp);
}