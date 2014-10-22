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

#include "lcd.h"
#include "geolocation.h"
#include "eeprom.h"

//---- DEVICE CONSTANTS ----//

#define RX_BUFFER_SIZE  128
#define STATION_BLOCKSIZE 28
#define FIRST_STATION_OFFSET 1
#define SERIAL_TIMEOUT 50000

//serial termination flags
#define FL_SUCCESS 0
#define FL_FAIL 1

//modes of operation
#define MD_NORMAL 0
#define MD_UPDATE 1
#define MD_UPDATE_REQUIRED 2

//---- FUNCTION DECLARATIONS ----//

//device + database config
void InitUSART(void);
void disable_gps(void);
void enable_gps(void);
void prepare_device(void);
void database_load(void);
void database_free(void);

//update utilities
int detectSerialStart(void);
int detectSerialEnd(void);
char getChar(void);
char peekChar(void);
void terminate_serial(int flag);
void check_database_integrity(void);

//modes of operation
void wipe_eeprom(void);
void show_nearest_station(void);
void wait_for_update(void);
void print_eeprom_station_contents(void);
void print_eeprom_contents(int start_addr, int end_addr);
void print_all_known_stations(void);
void print_all_callsigns(void);
void print_callsign(int station_index);
void print_station(int index);
void print_gps_data(void);
void print_raw_gps_data(void);
void test_earth_distance(void);
void wait_for_gps_lock(void);

//---- VARS ----//

//Station array held in program memory, populated from EEPROM
Station *all_stations;
int num_stations = 0;
//index of nearest station
int nearest_station = -1;

//User data held in program memory, populated with parsed GPS data
UserData *user;

//Serial Variables
volatile char rxBuffer[RX_BUFFER_SIZE];
volatile uint8_t rxReadPos = 0;
volatile uint8_t rxWritePos = 0;
volatile char serial_history[3] = {'\0'};
char serialStartChar = '$';
char serialEndChar = '^';
double serial_timer = 0;

//GPS Variables
volatile int gps_rxCount = 0;
volatile char gps_rxBuffer[80] = {'\0'};   //should really be about 66 chars
char *gps_data[13];

//Device State Variables
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
    int i;

    //set up GPIO, initialize interrupts, serial comm, and LCD
    prepare_device();

    //allocate memory for each of the raw gps_data fields
    for (i=0; i<13; i++)
    {
        gps_data[i] = (char *)malloc(16*sizeof(char));
    }

    //allocate memory for the UserData struct
    user = (UserData *)malloc(sizeof(UserData));

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
                if (num_stations < 1) {
                    op_mode = MD_UPDATE_REQUIRED;
                    break;
                }

                //behave normally
                enable_gps();

                //print the callsigns of all stations in the database
                print_all_callsigns();

                if (gps_locked(user))
                {
                    lcd_init();
                    string_write("GPS Locked!\n");
                    _delay_ms(500);
                    string_write_float(user->lat,3);
                    _delay_ms(250);
                    string_write(", ");
                    _delay_ms(250);
                    string_write_float(user->lon,3);
                    _delay_ms(2000);

                    //compute and show the nearest station
                    show_nearest_station();

                    //print the most recent gps data
                    print_gps_data();

                } else {
                    lcd_init();
                    string_write("No GPS Fix\n");
                    string_write("Be Patient...");
                    /*_delay_ms(500);
                    string_write(gps_data[3]);
                    string_write(", ");
                    string_write(gps_data[5]);*/
                    _delay_ms(2000);
                }

                //go through the complete list of known stations
                print_all_known_stations();

                //print_raw_gps_data();
            break;

            case MD_UPDATE_REQUIRED:
                //do nothing until an update is triggered
                wait_for_update();
            break;

            case MD_UPDATE:

                //make sure gps interrupts do not disrupt the update process
                disable_gps();

                //handle the update trigger
                if (updating == 0)
                {
                    lcd_init();
                    string_write("updating...\ndon't unplug");
                    updating = 1;
                    database_corrupted = 0;
                    eeprom_index = 0;

                    //scrap the outdated database structures
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
                        } else {
                            lcd_init();
                            string_write("update complete");
                            _delay_ms(1000);
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
                        print_eeprom_contents(0,32);
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

//serial database update receive interrupt behavior
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

//GPS serial receive interrupt behavior
ISR(USART0_RX_vect) {
    int i;

    //prevent buffer overflow
    if (gps_rxCount > 80)
    {
        for (i=0; i<80; i++)
            gps_rxBuffer[i]='\0';

        gps_rxCount = 0; 
    }

    //Read value out of the UART buffer
    gps_rxBuffer[gps_rxCount] = UDR0;

    gps_rxCount ++;

    //start new if receive $
    if (gps_rxBuffer[gps_rxCount-1]=='$')
    {
        for (i=1; i<80; i++)
            gps_rxBuffer[i]='\0'; 

        gps_rxBuffer[0] = '$';
        gps_rxCount = 1; 
    }

    //carriage return ----> parse the string and update the gps_data fields
    if ((gps_rxBuffer[gps_rxCount-1]=='\r')) {
        if (tag_check(gps_rxBuffer))
        {
            disable_gps();
            
            //strip off the rxBuffer carriage return and replace with ,
            gps_rxBuffer[gps_rxCount-1] = ',';

            //update the application gps_data fields
            parse_nmea(gps_rxBuffer, gps_data);

            //use the raw gps_data fields to populate the UserData struct
            update_user_gps_data(gps_data, user);

            //clear the rxBuffer
            for (i=0; i<80; i++)
                gps_rxBuffer[i]='\0';
            gps_rxCount = 0;
        }
    }
}

// Initialize USART
void InitUSART(void)
{
    // Set the serial Baud Rate to 9600 for 1 MHz system clock
    UBRR1H = 0;
    UBRR1L = 12;

    // Set the GPS Baud Rate.
    UBRR0H = 0;
    UBRR0L = 12;
    
    // Enable Receiver and Transmitter
    UCSR1B |= (1 << RXCIE1) | (1 << TXCIE1) | (1 << RXEN1) | (1 << TXEN1);
    UCSR0B |= (1 << RXCIE0) | (1 << TXCIE0) | (1 << RXEN0) | (1 << TXEN0);
    
    // Set U2X0 to reduce the Baud Rate error
    UCSR1A |= (1 << U2X1 );
    UCSR0A |= (1 << U2X0 );
    
    // Set the DB Update Frame Format to 8
    // Set the Parity to No Parity
    // Set the Stop Bits to 2
    UCSR1C |= (1 << UCSZ11) | (1 << UCSZ10) | (1 << USBS1);

    // Set the GPS Frame Format to 8
    // Set the Parity to No Parity
    // Set the Stop Bits to 1
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);

}

void disable_gps(void)
{
    UCSR0B &= ~((1 << RXCIE0) | (1 << TXCIE0) | (1 << RXEN0) | (1 << TXEN0));
}

void enable_gps(void)
{
    UCSR0B |= (1 << RXCIE0) | (1 << TXCIE0) | (1 << RXEN0) | (1 << TXEN0);
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

    if (flag==FL_FAIL)
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
        end_addr = 1+num_stations*STATION_BLOCKSIZE;

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
    if (op_mode==MD_UPDATE) return;
    int i;
    lcd_init();
    string_write_int(num_stations,3);
    string_write(" known\nstations");

    _delay_ms(2000);

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
    if (op_mode==MD_UPDATE) return;
    int i;
    lcd_init();
    string_write_int(num_stations,3);
    string_write(" known\nstations");
    _delay_ms(2000);

    lcd_init();
    string_write("\n");

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

void show_nearest_station(void)
{
    if (op_mode==MD_UPDATE) return;
    lcd_init();
    string_write("Finding Nearest\nStation...");
    _delay_ms(2000);

    lcd_init();
    nearest_station = get_nearest_station(all_stations, num_stations, user->lat, user->lon);
    print_callsign(nearest_station); string_write("\n");
    string_write_float(my_distance_to_station(user, all_stations, nearest_station),1); string_write(" km");
    if (op_mode==MD_UPDATE) return;
    _delay_ms(4000);

    if (op_mode==MD_UPDATE) return;
    lcd_init();    
    print_station(nearest_station);
    _delay_ms(2000);
}

//print the formatted data stored in the UserData struct to the screen
void print_gps_data(void)
{
    if (op_mode==MD_UPDATE) return;
    lcd_init();
    string_write("Latest\nGPS Data:");
    _delay_ms(1000);
    lcd_init();
    int i=0;
    for (i=0; i<11; i++)
    {

        if (i>0)
            string_write("\n");

        switch (i)
        {
            case 0:
                string_write("Message: "); 
                string_write_numchars(user->msg_type,8);
            break;

            case 1:
                string_write("Time: ");
                string_write_numchars(user->utc_time,8);
            break;

            case 2:
                string_write("NRW: ");
                char_write(user->nrw);
            break;

            case 3:
                string_write("Lat: ");
                string_write_float(user->lat,4);
            break;

            case 4:
                string_write("Lon: ");
                string_write_float(user->lon,4);
            break;

            case 5:
                string_write("Speed: ");
                string_write_float(user->speed,1);
            break;

            case 6:
                string_write("Course: ");
                string_write_float(user->course,3);
            break;

            case 7:
                string_write("Date: ");
                string_write_numchars(user->date,8);
            break;

            case 8:
                string_write("MagVar: ");
                string_write_numchars(user->mag_var,8);
            break;

            case 9:
                string_write("Mode: ");
                char_write(user->mode);
            break;

            case 10:
                string_write("Checksum: ");
                string_write_numchars(user->checksum,3);
            break;
        }

        _delay_ms(1000);
        if (op_mode==MD_UPDATE) return;
    }
}

//print the raw gps data in the gps_data string array to the screen
void print_raw_gps_data(void)
{
    if (op_mode==MD_UPDATE) return;
    lcd_init();
    string_write("Raw\nGPS Data");
    _delay_ms(1000);
    lcd_init();
    int i=0;
    for (i=0; i<13; i++)
    {

        if (i>0)
            string_write("\n");

        switch (i)
        {
            case 0:
                string_write("Message");
            break;

            case 1:
                string_write("Time");
            break;

            case 2:
                string_write("NRW");
            break;

            case 3:
                string_write("Lat");
            break;

            case 4:
                string_write("N/S");
            break;

            case 5:
                string_write("Lon");
            break;

            case 6:
                string_write("E/W");
            break;

            case 7:
                string_write("Speed");
            break;

            case 8:
                string_write("Course");
            break;

            case 9:
                string_write("Date");
            break;

            case 10:
                string_write("MagVar");
            break;

            case 11:
                string_write("Mode");
            break;

            case 12:
                string_write("Checksum");
            break;
        }

        string_write(": ");
        string_write(gps_data[i]);

        _delay_ms(500);
        if (op_mode==MD_UPDATE) return;
    }
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

//print the callsign for a given station index to the LCD
void print_callsign(int station_index)
{
    int i;
    for (i=0; i<8; i++) 
    {
        char_write(all_stations[station_index].callsign[i]);
    }
}

void test_earth_distance(void)
{
    lcd_init();
    string_write("Calculating\nEarth Distances");
    _delay_ms(2000);

    lcd_init();
    string_write("Denver -> NYC:\n");
    float lat1 = 40+43/60;
    float lon1 = -1*(74+1/60);
    float lat2 = 39+45/60;
    float lon2 = -1*(104+59/60);
    float distance = earth_distance(lat1, lon1, lat2, lon2);
    //should be about 2625 km
    string_write_float(distance,1); string_write(" km");
    _delay_ms(2500);

    lcd_init();
    string_write("North KC -> KC:\n");
    lat1 = 39.131;
    lon1 = -94.563;
    lat2 = 39.0832;
    lon2 = -94.559;
    distance = earth_distance(lat1, lon1, lat2, lon2);
    //should be somewhere around 5 km
    string_write_float(distance,1); string_write(" km");
    _delay_ms(2500);

    //just for reference, UMaine coords are: 44.900 -68.667
}

void wait_for_gps_lock(void)
{
    lcd_init();
    string_write("Waiting For\nGPS Lock...");
    _delay_ms(1000);
    while (!(gps_locked(user)))
    {
        lcd_init();
        print_gps_data();
        if (op_mode==MD_UPDATE) return;
        _delay_ms(250);
    }
    lcd_init();
    print_gps_data();
}