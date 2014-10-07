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

#include "arduino-serial-lib.h"

#define BUF_MAX 256

int open_port(void);
int send_string(char* in_string);
int parse_line(char* in_line);
char *strtok_single(char * in_str, char const * delims);
char * line_prep(char* in_line);
void strip_newline( char *str, int size );
int send_float(char* in_string);
int send_byte (uint8_t in_byte);

int fd = -1;
char serialport[BUF_MAX];
char *optarg =  "/dev/serial/by-id/usb-Dean_Camera_LUFA_USB-RS232_Adapter_A48303630363513112E1-if00";
int baudrate = 9600;

union float2bytes { 
    float f; 
    char b[sizeof(float)];
};

int main (int argc, char *argv[])
{

    FILE *file_hndl;
    char buf[1000];
    char *str_array[140]={"\0"};
    int i;

    //Open log file
    file_hndl =fopen("log.txt","r");
    if (!file_hndl){
        printf("ERROR: Could not open file for reading.");
        return -1;
    }

    open_port();
    send_string("$$$");

    //Fetch each line into a string
    while (fgets(buf,1000, file_hndl)!=NULL){
        str_array[i] = buf;
        //printf("%s",str_array[i]);

        strip_newline( str_array[i], 210);

        //Parse and send lines
        parse_line(line_prep(str_array[i]));
        i++;
    }

    //Close File
    fclose(file_hndl);

    usleep(3000000);
    send_string("^^^");

    serialport_close(fd);
    return 0;
}

int open_port(void){
    if( fd!=-1 ) {
        serialport_close(fd);
        printf("closed port %s\n",serialport);
    }
    strcpy(serialport,optarg);
    fd = serialport_init(optarg, baudrate);
    if( fd==-1 ){
        printf("couldn't open port");
        return -1;
    } 
    printf("opened port %s\n",serialport);
    serialport_flush(fd);
    
    return 0;
}

int send_string(char* in_string){

    //printf("in send_string\n");

    int rc = -1;

    if( fd == -1 ){ 
        printf("serial port not opened");
        return -1;
    }
    //sprintf(in_string, "%s", optarg);
    printf("send string:%s\n", in_string);
    rc = serialport_write(fd, in_string);
    if(rc==-1){
        printf("error writing");
        return -2;
    }

    return 0;
}

int send_float(char* in_string){
    int rc = -1;
    int i = 0;
    union float2bytes f2b;

    if( fd == -1 ){ 
        printf("serial port not opened");
        return -1;
    }

    printf("float value :%f\n" ,atof(in_string));
    f2b.f = atof(in_string);

    for (i = 0; i < 4; i++ ) {
        printf("byte %d = %c, ", i, f2b.b[i]);
        
        //functionalize!
        rc = send_byte((uint8_t)f2b.b[i]); //If this doesn't work, try a loop of 1 char stringwrites.
        if(rc==-1){
            printf("error writing");
            return -2;
        }
    }

    return 0;
}

int send_byte (uint8_t in_byte){
    int rc = -1;

    if( fd == -1 ){ 
        printf("serial port not opened");
        return -1;
    }
    rc = serialport_writebyte(fd, in_byte);
    if(rc==-1){
        printf("error writing");
        return -2;
    }

    return 0;
}

int parse_line(char * in_line){
    int i = 0;
    int initial_length = strlen(in_line);
    printf("initial_length = %d\n", initial_length);
    char * token = strtok_single(in_line, " ");
    /* walk through other tokens */
    //The first call you use a char array which has the elements you want parsed.
    //The second time you call it you pass it NULL as the first parameter to tell function 
    //to resume from the last spot in the string. Once the first call is made your char 
    //array receives the parsed string. If you don't put NULL you would lose your place 
    //and effectivly the last part of your string.
    if (initial_length > 2){
        if (initial_length >= 200){
            //grid population
            send_byte((uint8_t)*(token));
            printf("%s ", token);
            while(token){
                i++;
                token = strtok_single(NULL, " ");
                if (i < 100) send_byte((uint8_t)*(token));
                if (i < 100)  printf("%s ",token);
                usleep(50000);
            }
        }
        else{
            //Station line
            printf("callsign:%s ", token);
            send_string(token);
            if (strlen(token) < 8)
            {
                int k=0;
                for (k=0; k< 8-strlen(token); k++)
                {
                    send_string("+");
                }
            }
            while(token){
                i++;
                token = strtok_single(NULL, " ");
                if (i < 6) send_float(token); 
                //if (i < 6) printf("float:%s ",token); 
                usleep(50000);
            }       
        }  
    }
    else{
        send_byte((uint8_t)*(token));
        printf("byte:%s ",token);
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
            //printf("We are here: %d\n", i);
            str[i] = '\0';

            // we're done, so just exit the function by returning
            return;   
        }
    }
    // if we get all the way to here, there must not have been a newline
}

    