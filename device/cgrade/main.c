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

#include "display.h"

void InitUSART(void);

volatile int count = 0;
char c[16] = {0};
volatile char empty  = 'A';

#define RX_BUFFER_SIZE  128
char rxBuffer[RX_BUFFER_SIZE];
uint8_t rxReadPos = 0;
uint8_t rxWritePos = 0;

char getChar(void);
char peekChar(void);

volatile int update_progress = 0;

ISR(USART1_RX_vect){
    //Read value out of the UART buffer
    
    rxBuffer[rxWritePos] = UDR1;

    if(rxBuffer[rxWritePos] == 'A'){
        update_progress = 1;
        //string_write("1");
    } 
    else if(rxBuffer[rxWritePos] == 'Z'){
        update_progress = 0;
        //string_write("0");
    }
    else{
        //string_write("x");
    }

    rxWritePos++;
     
    if(rxWritePos >= RX_BUFFER_SIZE)
    {
        rxWritePos = 0;
    }

     update_progress = 1;
}

int main (int argc, char *argv[])
{
    char holder = 'B';
    DDRB = 0xFF;

    cli();

    //Init usart
    InitUSART();

    //Enable Global Interrupts. Sets SREG Interrupt bit.
    sei();

    //Intitialize LCD. Set Blinking cursor.
    lcd_cursor();
    
    while(1){
        if (update_progress == 1){
            //string_write("y");
            holder = getChar();
            if (holder != '\0') char_write(holder);
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


/*int main (int argc, char *argv[])
{
    //Intitialize LCD. Set Blinking cursor.
    int readbyte;

    lcd_cursor();
    eeprom_write_byte(0,7);
    readbyte = eeprom_read_byte((int*)0);

    char readchar = (char)readbyte;
    
    //Write ECE to screen
    //string_write(readchar);
    char_write(readchar);

    while(1){

    }
    return 0; //should never get here.
}*/