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
#define FIRST_STATION_OFFSET 101// 1 + 100
#define NUM_GRID_CELLS 100

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
void print_eeprom_contents();
void print_eeprom_station_contents();
void print_station(int index);

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

volatile char empty  = 'A';

//Serial Variables
char rxBuffer[RX_BUFFER_SIZE];
uint8_t rxReadPos = 0;
uint8_t rxWritePos = 0;
volatile char serial_history[3] = {'\0'};
char serialStartChar = '$';
char serialEndChar = '^';

//State Variables
volatile int update_progress = 0;
int read_ready = 0;
volatile int eeprom_index = 0;
int read_index = 0;

union float2bytes { 
    float f; 
    char b[sizeof(float)];
};

ISR(USART1_RX_vect){
    

    //shift serial history back one
    serial_history[2] = serial_history[1];
    serial_history[1] = serial_history[0];
    //Read value out of the UART buffer
    serial_history[0] = UDR1;

    if (update_progress==1)
    {
        rxBuffer[rxWritePos] = serial_history[0];
        rxWritePos++;
    }   

    if(serialStart()){
        update_progress = 1;
        //lcd_cursor();
        //eeprom_index = 0;
        //string_write("1");
    } 

    if(rxWritePos >= RX_BUFFER_SIZE)
    {
        rxWritePos = 0;
    }

}

int main (int argc, char *argv[])
{
    int i = 0;
    char holder = 'B';
    char readbyte = 'B';
    //char * prueba = "\0";
    //uint8_t ByteofData;
    
    DDRB = 0xFF;

    cli();

    //Init usart
    InitUSART();

    //Enable Global Interrupts. Sets SREG Interrupt bit.
    sei();

    //Intitialize LCD. Set Blinking cursor.
    lcd_init();

    _delay_ms(1000);
    
    //figure out how many stations there are by reading first eeprom byte
    num_stations = my_eeprom_read_int(0);

    string_write("ns = ");
    string_write_int(num_stations,3);
    string_write("\n");

    _delay_ms(500);


    //allocate memory for all the station structures
    all_stations = (Station *)malloc(num_stations*sizeof(Station));
    //populate the array containing the number of stations in each grid cell (next NUM_GRID_CELLS bytes in EEPROM)
    int total = 0;
    for (i=0; i<NUM_GRID_CELLS; i++)
    {
        stations_in_cell[i] = my_eeprom_read_int(i+1); //atoi(&(char)eeprom_read_byte((int *)i+1));
        cell_offsets[i] = FIRST_STATION_OFFSET+total*STATION_BLOCKSIZE;
        total += stations_in_cell[i];
    }

    /*for (i=0; i<NUM_GRID_CELLS; i++)
    {
        string_write_int(cell_offsets[i],3);
        _delay_ms(50);
    }*/

    //load in the stations one by one into the all_stations array of Station structs

    lcd_init();

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

    for (i=0; i<num_stations; i++)
    {
        print_station(i);
        _delay_ms(1000);
        lcd_init();
    }

    //

    //print_eeprom_contents();
    //print_eeprom_station_contents();


    while(1){

        if (update_progress == 1){
            //string_write("y");
            if(rxReadPos != rxWritePos) {
                holder = getChar();
            	if(serialEnd()) update_progress = 0;
            	else{
	            	eeprom_write_byte((uint8_t *)eeprom_index,holder);
                    /*if (eeprom_index < 16)
                        char_write(holder);*/
	            	eeprom_index ++;
	            	read_ready = 1;
	            }
            } 
        }

        if (read_ready){
            readbyte = (char)eeprom_read_byte((const uint8_t *)read_index);
            read_index ++;
            if (read_index < 232)
            {
                //if ((read_index-1)%32==0)
                //    lcd_init();
                if (readbyte=='\0')
                    string_write("fuck");
                char_write(readbyte);
            }
            read_ready = 0;             
        }
        

    }
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
    char temp_num = ((char)eeprom_read_byte((int *)address));
    return (atoi(&temp_num));
}

char my_eeprom_read_char(int address)
{
    return (char)eeprom_read_byte((int *)address);
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

void print_eeprom_contents()
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

void print_eeprom_station_contents()
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
    string_write(all_stations[index].callsign); string_write(" "); _delay_ms(500);
    string_write_float(all_stations[index].freq,1); string_write(" "); _delay_ms(500);
    string_write_float(all_stations[index].lat,4); string_write(" "); _delay_ms(500);
    string_write_float(all_stations[index].lon,4); string_write(" "); _delay_ms(500);
    string_write_float(all_stations[index].erp,1); string_write(" "); _delay_ms(500);
    string_write_float(all_stations[index].haat,0); string_write(" "); _delay_ms(500);
}