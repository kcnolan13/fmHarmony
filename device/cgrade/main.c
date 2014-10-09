// Marcel Marki & Kyle Nolan
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

//Serial Variables
#define RX_BUFFER_SIZE  128
#define STATION_BLOCKSIZE 28
#define FIRST_STATION_OFFSET 104// 4 + 100
#define NUM_GRID_CELLS 100
#define SERIAL_TIMEOUT 50000
#define FL_SUCCESS 0
#define FL_FAIL 1

void InitUSART(void);
char getChar(void);
char peekChar(void);
int serialStart(void);
int serialEnd(void);
int my_eeprom_read_int(int address);
char my_eeprom_read_char(int address);
float my_eeprom_read_float(int address);
void my_eeprom_read_string(char *dest, int address, int num_chars);
void string_write_int(int num, int num_digits);
void string_write_float(float num, int dec_digits);
void print_eeprom_contents(void);
void print_eeprom_station_contents(void);
void print_station(int index);
void print_callsign(int station_index);
void prepare_device(void);
void database_load(void);
void database_free(void);
void print_all_known_stations(void);
void print_all_callsigns(void);
void terminate_serial(int flag);
void check_integrity(void);

typedef struct station {
    char callsign[8];
    float freq;
    float lat;
    float lon;
    float erp;
    float haat;
} Station;

Station *all_stations;
int num_stations = 0;
int stations_in_cell[NUM_GRID_CELLS] = {0};
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
volatile int update_trigger = 0;
volatile int eeprom_index = 0;
int updating = 0;
int read_ready = 0;
int read_index = 0;
int database_corrupted = 0;

union float2bytes { 
    float f; 
    char b[sizeof(float)];
};


//serial receive interrupt behavior
ISR(USART1_RX_vect){
    
    //remember the last 3 bytes received (to handle start + end sequences)
    serial_history[2] = serial_history[1];
    serial_history[1] = serial_history[0];

    //Read most recent value out of the UART buffer
    serial_history[0] = UDR1;

    //if a serial update is in progress, write to the receive buffer
    if (update_trigger==1)
    {
        rxBuffer[rxWritePos] = serial_history[0];
        rxWritePos++;
    }   

    //trigger a serial database update if the start sequence has occurred
    if(serialStart()){
        update_trigger = 1;
    }

    //make the receive buffer loop
    if(rxWritePos >= RX_BUFFER_SIZE)
    {
        rxWritePos = 0;
    }

}

//main program
int main (int argc, char *argv[])
{
    char holder;//, readbyte;
    
    //set up GPIO, initialize interrupts, serial comm, and LCD
    prepare_device();

    //load in the FM stations database from EEPROM
    string_write("syncing\nmemory ...");
    database_load();
    _delay_ms(1000);
    
    //primary program loop
    while(1){

        //perform serial updates
        if (update_trigger == 1) {

            //handle the update trigger
            if (updating == 0)
            {
                lcd_init();
                string_write("downloading\nupdates ...");
                updating = 1;

                //free the old database from program memory
                database_free();
            }

            //read serial data from receive buffer when available
            if(rxReadPos != rxWritePos) {

                serial_timer = 0;
                holder = getChar();

                //handle serial transfer end sequence
            	if(serialEnd()) {
                    //terminate connection and update the database
                    terminate_serial(FL_SUCCESS);

                } else {
                    //this is not part of the end sequence --> write it to EEPROM!
	            	eeprom_write_byte((uint8_t *)eeprom_index,holder);
	            	eeprom_index ++;
	            	read_ready = 1;
	            }

            } else {
                //no data was available to read; make sure to timeout eventually
                serial_timer ++;

                //string_write_int(serial_timer,3); string_write(", ");
                if (serial_timer > SERIAL_TIMEOUT)
                {
                    //timeout --> close serial connection and import the new database
                    terminate_serial(FL_FAIL);
                }
            }

        } else {
            //behave normally
            print_all_callsigns();
            print_all_known_stations();
        }

        if (read_ready) {
            eeprom_read_byte((const uint8_t *)read_index);//readbyte = (char)eeprom_read_byte((const uint8_t *)read_index);
            read_index ++;
            //if (read_index < 232)
            {
                //char_write(readbyte);
            }
            read_ready = 0;             
        }
        
    } //primary program loop

    return 0; //should never get here.
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

char peekChar(void)
{
    char ret = '\0';
     
    if(rxReadPos != rxWritePos)
    {
        ret = rxBuffer[rxReadPos];
    }
     
    return ret;
}

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

int serialStart(void)
{
    if ((serial_history[0]==serialStartChar)&&(serial_history[1]==serialStartChar)&&(serial_history[2]==serialStartChar))
        return 1;
    else
        return 0;
}

int serialEnd(void)
{
    if ((serial_history[0]==serialEndChar)&&(serial_history[1]==serialEndChar)&&(serial_history[2]==serialEndChar))
        return 1;
    else
        return 0;
}

int my_eeprom_read_int(int address)
{
    char temp_num = ((char)eeprom_read_byte((uint8_t *)address));
    return (atoi(&temp_num));
}

char my_eeprom_read_char(int address)
{
    return (char)eeprom_read_byte((uint8_t *)address);
}

float my_eeprom_read_float(int address)
{
    return (float)(eeprom_read_float((const float *)address));
}

void my_eeprom_read_string(char *dest, int address, int num_chars)
{
    eeprom_read_block((void *)dest,(const void *)address,num_chars);
}

void string_write_int(int num, int num_digits)
{
    char *temp = (char *)malloc(num_digits*sizeof(char));
    sprintf(temp,"%d",num);
    string_write(temp);
    free(temp);
}

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

void print_eeprom_contents(void)
{
    int i=0;
    char one_byte;

    for (i=0; i<1+NUM_GRID_CELLS+num_stations*STATION_BLOCKSIZE; i++)
    {
        one_byte = my_eeprom_read_char(i);
        if (one_byte == '\0')
            one_byte = '?';
        char_write(one_byte);
        _delay_ms(100);
    }
}

void print_eeprom_station_contents(void)
{
    int start = FIRST_STATION_OFFSET;
    int i=0;
    char one_byte;

    for (i=0; i<STATION_BLOCKSIZE*num_stations; i++)
    {
        one_byte = my_eeprom_read_char(start+i);
        if (one_byte == '\0')
            one_byte = '?';
        char_write(one_byte);
        _delay_ms(100);
    }
}

void print_station(int index)
{
    string_write_int(index+1,3); string_write(": "); print_callsign(index); _delay_ms(250); string_write("\n"); 
    if (update_trigger)
        return;
    string_write("freq: "); string_write_float(all_stations[index].freq,1); _delay_ms(250); string_write("\n");
    if (update_trigger)
        return;
    string_write("lat: "); string_write_float(all_stations[index].lat,4); _delay_ms(250); string_write("\n");
    if (update_trigger)
        return;
    string_write("lon: "); string_write_float(all_stations[index].lon,4); _delay_ms(250); string_write("\n");
    if (update_trigger)
        return;
    string_write("erp: "); string_write_float(all_stations[index].erp,1); _delay_ms(250); string_write("\n");
    if (update_trigger)
        return;
    string_write("haat: "); string_write_float(all_stations[index].haat,0); _delay_ms(250); string_write("\n");
    if (update_trigger)
        return;
}

void print_callsign(int station_index)
{
    int i;
    for (i=0; i<8; i++) 
    {
        char_write(all_stations[station_index].callsign[i]);
    }
}

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

void database_load(void)
{
    int i;
    //figure out how many stations there are by reading the first number written to EEPROM
    num_stations = (int)my_eeprom_read_float(0);

    //allocate memory for all the station structures
    all_stations = (Station *)malloc(num_stations*sizeof(Station));

    //populate the array containing the number of stations in each grid cell (next NUM_GRID_CELLS bytes in EEPROM)
    int total = 0;
    for (i=0; i < NUM_GRID_CELLS; i ++)
    {
        stations_in_cell[i] = my_eeprom_read_int(i+4);
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

void print_all_known_stations(void)
{
    int i;
    lcd_init();
    string_write_int(num_stations,3);
    string_write(" known\nstations");

    _delay_ms(2000);

    if (update_trigger)
        return;

    for (i=0; i<num_stations; i++)
    {
        if (update_trigger)
            return;

        lcd_init();
        print_station(i);

        if (update_trigger)
            return;

        _delay_ms(200);   
    }
}

void print_all_callsigns(void)
{
    int i;
    lcd_init();
    string_write_int(num_stations,3);
    string_write(" known\nstations");
    _delay_ms(2000);

    lcd_init();

    if (update_trigger)
        return;

    for (i=0; i<num_stations; i++)
    {
        if (update_trigger)
            return;

        string_write_int(i+1,3); string_write(": "); print_callsign(i); string_write("\n");

        if (update_trigger)
            return;

        _delay_ms(250);   
    }
}

void terminate_serial(int flag)
{
    update_trigger = 0;
    updating = 0;
    serial_timer = 0;

    //import the new database
    lcd_init();

    if (flag==FL_SUCCESS)
        string_write("syncing\nmemory ...");
    else
        string_write("ERROR:\ntimeout ...");

    database_load();
    _delay_ms(2000);

    if (flag==FL_SUCCESS)
        string_write("\nDATABASE\nUPDATED");
    else
        string_write("\nUPDATE\nFAILED");

    _delay_ms(1500);

}

void check_integrity(void)
{
    int i, j;
    for (i=0; i<num_stations; i++)
    {
        char * call = all_stations[i].callsign;
        for (j=0; j<strlen(call); j++)
        {
            if ((call[j] < 33)||(call[j] > 126))
            {
                database_corrupted = 1;
                return;
            }
        }
    }
}