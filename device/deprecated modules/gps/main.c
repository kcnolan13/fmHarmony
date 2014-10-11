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
int USARTgetchar(void);

volatile int count = 0;
volatile char c[32] = {0};
volatile char empty  = 'A';

//Open Filestream
//FILE my_stream = FDEV_SETUP_STREAM(USARTputchar, USARTgetchar, _FDEV_SETUP_RW);

ISR(USART0_RX_vect){
    //Read value out of the UART buffer
    PORTB ^= _BV(PB0);
    if (count < 32) c[count] = UDR0;
    else empty = UDR0;
    //if (count == 32) string_write(c);
    count ++;
}


int main (int argc, char *argv[])
{
    DDRB = 0xFF;

    char *message = (char *) malloc(17);
    char *testmes = (char *) malloc(1);
 
    message = "Aoaoaoaoaoaoqqwwr";
    //disable interrupts
    cli();

    //Init usart
    InitUSART();

    //Enable Global Interrupts. Sets SREG Interrupt bit.
    sei();

	//Intitialize LCD. Set Blinking cursor.
	lcd_cursor();

    //InitUSART()

	//Write ECE to screen
    testmes[0] = message[16];
    string_write(message);
    string_write(testmes);
	
    //_delay_ms(10000);
	
	while(1){
        //string_write(message)

	}
	return 0; //should never get here.
}

// Initialize USART
void InitUSART(void)
{
    // Set the Baud Rate.
    UBRR0H = 0;
    UBRR0L = 12;
    
    // Enable The Receiver and Transmitter
    UCSR0B |= (1 << RXCIE0) | (1 << TXCIE0) | (1 << RXEN0) | (1 << TXEN0);
    
    // Set U2X0 to reduce the Baud Rate error
    UCSR0A |= (1 << U2X0 );
    
    // Set the Frame Format to 8
    // Set the Parity to No Parity
    // Set the Stop Bits to 1
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);

    //stdin = &my_stream;
    //stdout= &my_stream;
}

// Function to recieve character from buffer.
int USARTgetchar(void)
{
    // Wait for incoming data
    while ( !(UCSR0A & (1<<RXC0)) );
    // Return the received data from buffer
    return UDR0;
}

// Function to transmit character.
/*int USARTputchar(char c, FILE *stream)
{
    // Wait for an empty transmit buffer
    while ( !( UCSR0A & (1<<UDRE0)) );
    //Put data into buffer. Sends the data automatically.
    UDR0 = c;
    return 0;
}*/