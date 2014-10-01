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
int USARTgetchar(FILE *);
int USARTputchar(char c, FILE *);

volatile int count = 0;
volatile char c[16] = {0};
volatile char empty  = 'A';

//Open Filestream
static FILE my_stream = FDEV_SETUP_STREAM (USARTputchar, USARTgetchar, _FDEV_SETUP_RW);

ISR(USART1_RX_vect){
    //Read value out of the UART buffer
    //PORTB ^= _BV(PB0);
    empty = UDR1;
    //sprintf(&empty,"%d",count);
    char_write(empty);
    count ++;
}

int main (int argc, char *argv[])
{
    DDRB = 0xFF;

    cli();

    //Init usart
    InitUSART();

    //Enable Global Interrupts. Sets SREG Interrupt bit.
    sei();

    //Intitialize LCD. Set Blinking cursor.
    lcd_cursor();
    
    //_delay_ms(10000);
    
    while(1){

    }
    return 0; //should never get here.
}

// Initialize USART
void InitUSART(void)
{
    // Set the Baud Rate.
    UBRR1H = 0;
    UBRR1L = 12;
    
    // Enable The Receiver and Transmitter
    UCSR1B |= (1 << RXCIE1) | (1 << TXCIE1) | (1 << RXEN1) | (1 << TXEN1);
    
    // Set U2X0 to reduce the Baud Rate error
    UCSR1A |= (1 << U2X1 );
    
    // Set the Frame Format to 8
    // Set the Parity to No Parity
    // Set the Stop Bits to 1
    UCSR1C |= (1 << UCSZ11) | (1 << UCSZ10);

    stdin = &my_stream;
    stdout= &my_stream;
}

// Function to recieve character from buffer.
int USARTgetchar(FILE * fd)
{
    // Wait for incoming data
    while ( !(UCSR1A & (1<<RXC1)) );
    // Return the received data from buffer
    return UDR1;
}

// Function to transmit character.
int USARTputchar(char c, FILE *stream)
{
    // Wait for an empty transmit buffer
    while ( !( UCSR1A & (1<<UDRE1)) );
    //Put data into buffer. Sends the data automatically.
    UDR1 = c;
    return 0;
}