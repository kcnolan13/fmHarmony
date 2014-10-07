// M. Marki K. Nolan
// Program for sending log file serially to fmHarmony device
// arduino-serial library courtesy of Tod E. Kurt
// The open_port and send_string functions of this program were
// largely inspired by Tod E. Kurt's arduino-serial.c program

#include <stdio.h>    // Standard input/output definitions
#include <stdlib.h>
#include <string.h>   // String function definitions
#include <unistd.h>   // for usleep()
#include <getopt.h>

#include "arduino-serial-lib.h"

#define BUF_MAX 256

int open_port(void);
int send_string(char* in_string);

int fd = -1;
char serialport[BUF_MAX];
char *optarg =  "/dev/serial/by-id/usb-Dean_Camera_LUFA_USB-RS232_Adapter_A48303630363513112E1-if00";
int baudrate = 9600;



int main (int argc, char *argv[])
{
    open_port();

    send_string("@try #this.");

    serialport_close(fd);
    return 1;
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
    
    return 1;
}

int send_string(char* in_string){

    printf("in send_string\n");

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

    return 1;
}