// Marcel Marki & Kyle Nolan
#include <avr/io.h>
#include <avr/delay.h>
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
#define BLOCKSIZE 28
#define START_OFFSET 102// 1 + 1 + 100

void InitUSART(void);
char getChar(void);
char peekChar(void);
int serialStart(void);
int serialEnd(void);

struct station{
    char callsign[8];
    float freq;
    float lat;
    float lon;
    float erp;
    float haat;
};

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
    //int i = 0;
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
    
    //string_write("Loading Database. Don't Update.");

    struct station *stat_test = (struct station*)malloc(sizeof(struct station));


    //ByteofData = eeprom_read_byte((uint8_t)0);
    int i=0;
    for (i=0; i<232; i++)
    {
        //eeprom_write_byte((uint8_t *)i,'A');
        readbyte = (char)eeprom_read_byte((int *)i);
        char_write(readbyte);
        _delay_ms(100);
        
        if (i%32==0)
            lcd_init();
    }

    //eeprom_read_block((void*)stat_test->callsign, (const void*)(0), 1);
    //char_write(stat_test->callsign[0]);

    /*
    char_write(stat_test.callsign[1]);
    char_write(stat_test.callsign[2]);
    char_write(stat_test.callsign[3]);
    char_write(stat_test.callsign[4]);
    char_write(stat_test.callsign[5]);
    char_write(stat_test.callsign[6]);
    char_write(stat_test.callsign[7]);*/

    while(1){

        if (update_progress == 1){
            //string_write("y");
            holder = getChar();
            if (holder != '\0'){
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
                if ((read_index-1)%32==0)
                    lcd_init();

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
     
    if(rxReadPos != rxWritePos)
    {
        ret = rxBuffer[rxReadPos];
         
        rxReadPos++;
         
        if(rxReadPos >= RX_BUFFER_SIZE)
        {
            rxReadPos = 0;
        }
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