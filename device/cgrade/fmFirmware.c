/*
#=================================================================
# APPLICATION NAME: fmFirmware
#
# DESCRIPTION: Microcontroller Firmware for the FM Harmony Device
#
# AUTHORS: Marcel Marki, Kyle Nolan
#
# DATE: 2013.10.09
#  
#================================================================
*/

//required libraries
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
#include "display.h"

//---- DEVICE CONSTANTS ----//

#define RX_BUFFER_SIZE  128
#define STATION_BLOCKSIZE 28
#define FIRST_STATION_OFFSET 101// 1 + 100
#define NUM_GRID_CELLS 100
#define SERIAL_TIMEOUT 50000

//serial termination flags
#define FL_SUCCESS 0
#define FL_FAIL 1

//modes of operation
#define MD_NORMAL 0
#define MD_UPDATE 1
#define MD_UPDATE_REQUIRED 2

//---- FUNCTION DECLARATIONS ----//

//device configuration
void InitUSART(void);
void prepare_device(void);
void database_load(void);
void database_free(void);

//EEPROM Operations
void wipe_eeprom(void);
int my_eeprom_read_int(int address);
char my_eeprom_read_char(int address);
float my_eeprom_read_float(int address);
void my_eeprom_read_string(char *dest, int address, int num_chars);

//LCD Routines
void string_write_int(int num, int num_digits);
void string_write_float(float num, int dec_digits);
void print_station(int index);
void print_callsign(int station_index);

//Update Utilities
int detectSerialStart(void);
int detectSerialEnd(void);
char getChar(void);
char peekChar(void);
void terminate_serial(int flag);
void check_database_integrity(void);


//modes of operation
void wait_for_update(void);
void print_eeprom_station_contents(void);
void print_eeprom_contents(int start_addr, int end_addr);
void print_all_known_stations(void);
void print_all_callsigns(void);


//---- GLOBAL VARS AND STRUCTURES ----//

//Generic FM Station Structure
typedef struct station {
    char callsign[8];
    float freq;
    float lat;
    float lon;
    float erp;
    float haat;
} Station;

//Station array held in program memory, populated from EEPROM
Station *all_stations;
int num_stations = 0;
//The number of stations in each Maine grid cell (#1 - #100)
int stations_in_cell[NUM_GRID_CELLS] = {0};
//The Byte offset to the start of each grid cell in EEPROM (relative to location of first station)
int cell_offsets[NUM_GRID_CELLS] = {0};

//Serial Variables
volatile char rxBuffer[RX_BUFFER_SIZE];
volatile uint8_t rxReadPos = 0;
volatile uint8_t rxWritePos = 0;
volatile char serial_history[3] = {'\0'};
char serialStartChar = '$';
char serialEndChar = '^';
double serial_timer = 0;

//State Variables
volatile int op_mode = MD_NORMAL;
volatile int eeprom_index = 0;
int updating = 0;
int database_corrupted = 0;

//used to reconstruct a float from four continguous bytes in EEPROM
union float2bytes { 
    float f; 
    char b[sizeof(float)];
};


//---- PRIMARY APPLICATION ----//

int main (int argc, char *argv[])
{
    char holder;
    
    //set up GPIO, initialize interrupts, serial comm, and LCD
    prepare_device();

    //load in the FM stations database from EEPROM
    string_write("reading\ndatabase...");
    database_load();
    _delay_ms(1000);
    
    //primary program loop
    while(1){

        switch (op_mode)
        {
            case MD_NORMAL:

                //ask for an update if no known stations
                if (num_stations < 1)
                {
                    print_eeprom_contents(0,32);
                    //don't set the mode if the update has already been triggered
                    if (op_mode != MD_UPDATE)
                        op_mode = MD_UPDATE_REQUIRED;

                    break;
                }

                //behave normally
                //print_eeprom_contents(0,32);
                print_all_callsigns();
                print_all_known_stations();
            break;

            case MD_UPDATE_REQUIRED:
                //do nothing until an update is triggered
                wait_for_update();
            break;

            case MD_UPDATE:

                //handle the update trigger
                if (updating == 0)
                {
                    lcd_init();
                    string_write("updating...\ndon't unplug");
                    updating = 1;
                    database_corrupted = 0;

                    //free the old database from program memory
                    database_free();
                }

                //read serial data from receive buffer when available
                if(rxReadPos != rxWritePos) {

                    serial_timer = 0;
                    holder = getChar();

                    //handle serial transfer end sequence
                    if(detectSerialEnd()) {

                        //terminate connection and update the database
                        terminate_serial(FL_SUCCESS);

                        //check for database corruption
                        check_database_integrity();
                        if (database_corrupted)
                        {
                            //wipe 100 stations worth of EEPROM and require a fresh update
                            wipe_eeprom();
                            op_mode = MD_UPDATE_REQUIRED;
                        }

                    } else {

                        //write real serial data bytes to EEPROM
                        eeprom_write_byte((uint8_t *)eeprom_index,holder);
                        eeprom_index ++;
                    }

                } else {

                    //no data was available to read this time around; work towards a timeout
                    serial_timer ++;

                    if (serial_timer > SERIAL_TIMEOUT)
                    {
                        //serial timeout --> close serial connection, wipe memory, and require a fresh update
                        terminate_serial(FL_FAIL);
                        wipe_eeprom();
                        op_mode = MD_UPDATE_REQUIRED;
                    }
                }
            break;

        }
        
    } //primary program loop

    return 0; //should never get here.
}


//---- FUNCTION DEFINITIONS ----//

//---- device configuration ----//

//serial receive interrupt behavior
ISR(USART1_RX_vect){
    
    //remember the last 3 bytes received (to handle start + end sequences)
    serial_history[2] = serial_history[1];
    serial_history[1] = serial_history[0];

    //Read most recent value out of the UART buffer
    serial_history[0] = UDR1;

    //if a serial update is in progress, write to the receive buffer
    if (op_mode==MD_UPDATE)
    {
        rxBuffer[rxWritePos] = serial_history[0];
        rxWritePos++;
    }   

    //trigger a serial database update if the start sequence has occurred
    if(detectSerialStart()){
        op_mode = MD_UPDATE;
    }

    //make the receive buffer loop
    if(rxWritePos >= RX_BUFFER_SIZE)
    {
        rxWritePos = 0;
    }
}

// Initialize USART
void InitUSART(void)
{
    // Set the Baud Rate to 9600 for 1 MHz system clock
    UBRR1H = 0;
    UBRR1L = 12;
    
    // Enable The Receiver and Transmitter
    UCSR1B |= (1 << RXCIE1) | (1 << TXCIE1) | (1 << RXEN1) | (1 << TXEN1);
    
    // Set U2X0 to reduce the Baud Rate error
    UCSR1A |= (1 << U2X1 );
    
    // Set the Frame Format to 8
    // Set the Parity to No Parity
    // Set the Stop Bits to 2
    UCSR1C |= (1 << UCSZ11) | (1 << UCSZ10) | (1 << USBS1);

    //stdin = &my_stream;
    //stdout= &my_stream;
}

//set up GPIO, initialize interrupts, serial comm, and LCD
void prepare_device(void)
{
    DDRB = 0xFF;
    cli();
    //Init usart
    InitUSART();
    //Enable Global Interrupts. Sets SREG Interrupt bit.
    sei();
    //Intitialize LCD. Set Blinking cursor.
    lcd_init();
    _delay_ms(1000);
}

//load the FM Stations database from EEPROM into program memory
void database_load(void)
{
    int i;
    //figure out how many stations there are by reading the first number written to EEPROM
    num_stations = my_eeprom_read_int(0);

    if (num_stations==255)
        num_stations = 0;

    _delay_ms(500);
    lcd_init();
    string_write("importing ");
    string_write_int(num_stations,3);
    string_write("\nstations...");

    //allocate memory for all the station structures
    all_stations = (Station *)malloc(num_stations*sizeof(Station));

    //populate the array containing the number of stations in each grid cell (next NUM_GRID_CELLS bytes in EEPROM)
    int total = 0;
    for (i=0; i < NUM_GRID_CELLS; i ++)
    {
        stations_in_cell[i] = my_eeprom_read_int(i+1);
        cell_offsets[i] = FIRST_STATION_OFFSET+total*STATION_BLOCKSIZE;
        total += stations_in_cell[i];
    }

    /*for (i=0; i<NUM_GRID_CELLS; i++)
    {
        string_write_int(cell_offsets[i],3);
        _delay_ms(50);
    }*/

    //load in the stations one by one into the all_stations array of Station structs
    for (i=0; i<num_stations; i++)
    {
        int start = FIRST_STATION_OFFSET+i*STATION_BLOCKSIZE;
        my_eeprom_read_string(all_stations[i].callsign,start,8); start += 8;
        all_stations[i].freq = my_eeprom_read_float(start); start += 4;
        all_stations[i].lat = my_eeprom_read_float(start); start += 4;
        all_stations[i].lon = my_eeprom_read_float(start); start += 4;
        all_stations[i].erp = my_eeprom_read_float(start); start += 4;
        all_stations[i].haat = my_eeprom_read_float(start); start += 4;
    }
}

//free the FM Stations database from program memory
void database_free(void)
{
    num_stations = 0;

    free(all_stations);
    all_stations = NULL;

    int i;
    for (i=0; i<NUM_GRID_CELLS; i++)
    {
        stations_in_cell[i] = 0;
        cell_offsets[i] = 0;
    }
}

//---- EEPROM Operations ----//

//wipe 100-stations-worth of EEPROM data
void wipe_eeprom(void)
{
    int i;
    lcd_init();
    string_write("wiping\nmemory...");
    for (i=0; i<FIRST_STATION_OFFSET+100*STATION_BLOCKSIZE; i++)
    {
        if (op_mode==MD_UPDATE) return;
        eeprom_write_byte((uint8_t *)i,255);
    }
}

//read a single-byte integer from an EEPROM address
int my_eeprom_read_int(int address)
{
    int temp_num = ((int)eeprom_read_byte((uint8_t *)address));
    return (temp_num);
}

//read a char from an EEPROM address
char my_eeprom_read_char(int address)
{
    return (char)eeprom_read_byte((uint8_t *)address);
}

//read a 4-byte float from an EEPROM address
float my_eeprom_read_float(int address)
{
    return (float)(eeprom_read_float((const float *)address));
}

//read a string from an EEPROM address
void my_eeprom_read_string(char *dest, int address, int num_chars)
{
    eeprom_read_block((void *)dest,(const void *)address,num_chars);
}

//---- LCD Routines ----//

//write a multi-char integer to the LCD as a string
void string_write_int(int num, int num_digits)
{
    char *temp = (char *)malloc(num_digits*sizeof(char));
    sprintf(temp,"%d",num);
    string_write(temp);
    free(temp);
}

//write a floating point number to the LCD as a string
void string_write_float(float num, int dec_digits)
{
    double intpart, fractpart;
    fractpart = modf(num, &intpart);

    string_write_int((int)intpart,4); string_write("."); 

    int temp = (int)(abs((round((fractpart*pow(10,dec_digits))))));
    int digits = 0;

    if (temp!=0)
    {
        digits = floor(log10(abs(temp)))+1;
    } else {
        digits = 0;
    }

    int i=0;
    for (i=0; i<(dec_digits-digits); i++)
    {
        string_write("0");
    }

    string_write_int(temp,4);
}

//print the informatoin held for a single station to the LCD
void print_station(int index)
{
    string_write_int(index+1,3); string_write(": "); print_callsign(index); _delay_ms(250); string_write("\n"); 
    if (op_mode==MD_UPDATE)
        return;
    string_write("freq: "); string_write_float(all_stations[index].freq,1); _delay_ms(250); string_write("\n");
    if (op_mode==MD_UPDATE)
        return;
    string_write("lat: "); string_write_float(all_stations[index].lat,4); _delay_ms(250); string_write("\n");
    if (op_mode==MD_UPDATE)
        return;
    string_write("lon: "); string_write_float(all_stations[index].lon,4); _delay_ms(250); string_write("\n");
    if (op_mode==MD_UPDATE)
        return;
    string_write("erp: "); string_write_float(all_stations[index].erp,1); _delay_ms(250); string_write("\n");
    if (op_mode==MD_UPDATE)
        return;
    string_write("haat: "); string_write_float(all_stations[index].haat,0); _delay_ms(250); string_write("\n");
    if (op_mode==MD_UPDATE)
        return;
}

//print the callsign for a given station index to the LCD
void print_callsign(int station_index)
{
    int i;
    for (i=0; i<8; i++) 
    {
        char_write(all_stations[station_index].callsign[i]);
    }
}

//---- UPDATE UTILITIES ----//

//catch the serial update start sequence
int detectSerialStart(void)
{
    if ((serial_history[0]==serialStartChar)&&(serial_history[1]==serialStartChar)&&(serial_history[2]==serialStartChar))
        return 1;
    else
        return 0;
}

//catch the serial update end sequence
int detectSerialEnd(void)
{
    if ((serial_history[0]==serialEndChar)&&(serial_history[1]==serialEndChar)&&(serial_history[2]==serialEndChar))
        return 1;
    else
        return 0;
}

//read a char from the serial update buffer
char getChar(void)
{
    char ret = '\0';
    
    ret = rxBuffer[rxReadPos];
     
    rxReadPos++;
     
    if(rxReadPos >= RX_BUFFER_SIZE)
    {
        rxReadPos = 0;
    }
    
    return ret;
}

//peek at the next char in the serial update buffer
char peekChar(void)
{
    char ret = '\0';
     
    if(rxReadPos != rxWritePos)
    {
        ret = rxBuffer[rxReadPos];
    }
     
    return ret;
}

//terminate the serial update with a certain status
void terminate_serial(int flag)
{
    op_mode = MD_NORMAL;
    updating = 0;
    serial_timer = 0;

    //import the new database
    lcd_init();

    if (flag==FL_SUCCESS)
        string_write("reading\ndatabase ...");
    else
        string_write("ERROR:\ntimeout ...");

    database_load();
    _delay_ms(1000);

    if (flag==FL_SUCCESS)
        string_write("\nupdate complete\n");
    else
        string_write("\nupdate failed\n");

    _delay_ms(500);
}

//check for database corruption
void check_database_integrity(void)
{
    int i, j;
    for (i=0; i<num_stations; i++)
    {
        char * call = all_stations[i].callsign;
        for (j=0; j<8; j++)
        {
            //indicate corruption if any station callsigns contain abnormal characters
            if (((call[j] < 33)||(call[j] > 126))&&(call[j]!=' '))
            {
                database_corrupted = 1;
                lcd_init();
                string_write("CORRUPTION\nDETECTED");
                _delay_ms(2000);
                lcd_init();
                string_write("tracing\ncorruption...");
                _delay_ms(500);
                lcd_init();
                string_write("station #");
                string_write_int(i+1,3);
                string_write(" : ");
                char_write('\'');
                char_write(call[j]);
                char_write('\'');
                char_write('\n');
                _delay_ms(2000);
                print_callsign(i);
                _delay_ms(4000);
                return;
            }
        }
    }
}

//---- MODES OF OPERATION ----//

//print the EEPROM contents for an address range
void print_eeprom_contents(int start_addr, int end_addr)
{
    int i=0;
    char one_byte;

    if (end_addr == -1)
        end_addr = 1+NUM_GRID_CELLS+num_stations*STATION_BLOCKSIZE;

    lcd_init();

    for (i=start_addr; i<end_addr; i++)
    {
        if (op_mode==MD_UPDATE) return;
        one_byte = my_eeprom_read_char(i);
        if (one_byte == '\0')
            one_byte = '?';
        char_write(one_byte);
        _delay_ms(100);
    }
}

//print the EEPROM contents for all stations
void print_eeprom_station_contents(void)
{
    int start = FIRST_STATION_OFFSET;
    int i=0;
    char one_byte;

    for (i=0; i<STATION_BLOCKSIZE*num_stations; i++)
    {
        if (op_mode==MD_UPDATE) return;
        one_byte = my_eeprom_read_char(start+i);
        if (one_byte == '\0')
            one_byte = '?';
        char_write(one_byte);
        _delay_ms(100);
    }
}

//print the information held for all stations to the screen
void print_all_known_stations(void)
{
    int i;
    lcd_init();
    string_write_int(num_stations,3);
    string_write(" known\nstations");

    _delay_ms(2000);

    if (op_mode==MD_UPDATE) return;

    for (i=0; i<num_stations; i++)
    {
        if (op_mode==MD_UPDATE) return;

        lcd_init();
        print_station(i);

        if (op_mode==MD_UPDATE) return;

        _delay_ms(200);   
    }
}

//quickly print all known callsigns to the screen
void print_all_callsigns(void)
{
    int i;
    lcd_init();
    string_write_int(num_stations,3);
    string_write(" known\nstations");
    _delay_ms(2000);

    lcd_init();
    string_write("\n");

    if (op_mode==MD_UPDATE) return;

    for (i=0; i<num_stations; i++)
    {
        if (op_mode==MD_UPDATE) return;

         string_write("\n"); string_write_int(i+1,3); string_write(": "); print_callsign(i);

        if (op_mode==MD_UPDATE) return;

        _delay_ms(250);   
    }
}

void wait_for_update(void)
{
    lcd_init();
    string_write("update required\n...feed me...");
    while (1)
    {
        if (op_mode==MD_UPDATE) return;
    }
}