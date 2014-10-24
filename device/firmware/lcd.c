// Marcel Marki & Kyle Nolan
// LCD Module Library File

#include <stdlib.h>
#include "lcd.h"

int chars_written = 0;
int line_num = 1;

int lcd_init()
{
    chars_written = 0;
    line_num = 1;
	//Power Port D as outputs.
	DDRA |= 0xFF;

	//Wait more than 15ms after Vcc = 4.5V
	_delay_ms(20);

	//Function Set Command: 8-Bit interface
    PORTA &= ~_BV(E);
    PORTA &= ~_BV(RS);
    PORTA &= ~_BV(RW);
    PORTA &= ~_BV(DB7);
    PORTA &= ~_BV(DB6);
    PORTA |= _BV(DB5);
    PORTA |= _BV(DB4);
    datarw();

	//Wait more than 4.1 ms
	_delay_ms(5);

	//Function Set Command: 8-Bit interface. Part 2
    datarw();
	
	//Wait more than 100 us.
	_delay_ms(1);
	
	//Function Set Command 8-Bit interface. Part 3
	datarw();
	_delay_ms(1);

	//Now that this command is written, BF (busy flag) can be checked.

	//Function Set: Sets interface to 4-bit.
	PORTA &= ~_BV(DB7);
    PORTA &= ~_BV(DB6);
    PORTA |= _BV(DB5);
    PORTA &= ~_BV(DB4);
    datarw();
    _delay_ms(1);

	//Need to change how we send data at this point to accomodate 4-bit mode.

    //Function Set: Interface
    PORTA &= ~_BV(DB7);
    PORTA &= ~_BV(DB6);
    PORTA |= _BV(DB5);
    PORTA &= ~_BV(DB4);
    datarw();
    _delay_ms(1);


    PORTA |= _BV(DB7); //diff
    PORTA &= ~_BV(DB6);
    PORTA &= ~_BV(DB5);
    PORTA &= ~_BV(DB4);
    datarw();
    _delay_ms(1);

    //Display OFF
    PORTA &= ~_BV(DB7);
    PORTA &= ~_BV(DB6);
    PORTA &= ~_BV(DB5);
    PORTA &= ~_BV(DB4);
    datarw();
    _delay_ms(1);

    PORTA |= _BV(DB7);
    PORTA |= _BV(DB6);//diff
    PORTA |= _BV(DB5); //diff
    PORTA &= ~_BV(DB4);
    datarw();
    _delay_ms(1);

    //Clear Display
    PORTA &= ~_BV(DB7);
    PORTA &= ~_BV(DB6);
    PORTA &= ~_BV(DB5);
    PORTA &= ~_BV(DB4);
    datarw();
    _delay_ms(1);
    
    PORTA &= ~_BV(DB7);
    PORTA &= ~_BV(DB6);
    PORTA &= ~_BV(DB5);
    PORTA |= _BV(DB4);
    datarw();
    _delay_ms(1);
	
    //Entry Mode Set
    PORTA &= ~_BV(DB7);
    PORTA &= ~_BV(DB6);
    PORTA &= ~_BV(DB5);
    PORTA &= ~_BV(DB4);
    datarw();
    _delay_ms(1);

    PORTA &= ~_BV(DB7);
    PORTA |= _BV(DB6);
    PORTA |= _BV(DB5);
    PORTA &= ~_BV(DB4);//diff
    datarw();
    _delay_ms(1);

    //Display On. Cursor and Blink on.
    PORTA &= ~_BV(DB7);
    PORTA &= ~_BV(DB6);
    PORTA &= ~_BV(DB5);
    PORTA &= ~_BV(DB4);
    datarw();
    _delay_ms(1);

    PORTA |= _BV(DB7);
    PORTA |= _BV(DB6);
    PORTA |= _BV(DB5);
    PORTA |= _BV(DB4);
    datarw();
    _delay_ms(1);

	return 0;
}

void letterwrite(int a, int b, int c, int d){
	
	if(a==1) PORTA |= _BV(DB7);
	else   PORTA &= ~_BV(DB7);

	if (b==1) PORTA |= _BV(DB6);
	else PORTA &= ~_BV(DB6);
    
    	if(c==1) PORTA |= _BV(DB5);
	else PORTA &= ~_BV(DB5);
	
	if (d==1) PORTA |= _BV(DB4);
    	else PORTA &= ~_BV(DB4);
   	datarw();
    	_delay_ms(1);
}

void datarw(){
	//Set Enable bit high, wait, set enable bit low
	//writes out data on other bits.
    PORTA |= _BV(E);
	_delay_ms(5);
	PORTA &= ~_BV(E);
}

void string_write(char *mystring)
{
    int i;
    get_current_address();
    //printf("writing string\n");
    for (i=0; i<strlen(mystring); i++) {
        char_write(mystring[i]);
        get_current_address();
    }
}

int get_current_address() {
    int address;
    address = instruction_read()&0x7F;
    return address+1;
}

void set_ddram_address(int address) {
    PORTA &=~ _BV(RS);
    PORTA &=~ _BV(RW);
    write_db74(1,(address>>6)&1,(address>>5)&1,(address>>4)&1);
    datarw();
    write_db74((address>>3)&1,(address>>2)&1,(address>>1)&1,(address>>0)&1);
    datarw();
}

int instruction_read() 
{
    int busy = 0, address = 0;
    uint8_t value;
    // Set LCD pins DB4-7 to be inputs with pull-ups
    DDRA &=~ (_BV(DB7)|_BV(DB6)|_BV(DB5)|_BV(DB4));    //Set pins as inputs;

    // PORTA LCD pins RS to 0 and RW to 1
    PORTA &=~ _BV(RS);
    PORTA |= _BV(RW);

    // load first 4 bits of read data into DB4-7
    // set the clock high and wait
    PORTA |= _BV(E);
    _delay_ms(5);

    // first bit is busy flag, last 3 are most significant 3 (of 7) bits of address counter
    // read data from the gpio pins
    value = PORTA & _BV(DB7); //this or PIND
    if (value) busy = 1;
    value = PORTA & _BV(DB6);
    address |= value<<6;
    value = PORTA & _BV(DB5);
    address |= value<<5;
    value = PORTA & _BV(DB4);
    address |= value<<4;
    
    // set the clock back to low
    PORTA &=~ _BV(E);
    _delay_ms(1);

    // load last 4 bits of read data into DB4-7
    // set the clock high and wait
    PORTA |= _BV(E);
    _delay_ms(5);

    // read data from the GPIO pins
    value = PORTA & _BV(DB7); //this or PIND
    address |= value<<3;
    value = PORTA & _BV(DB6);
    address |= value<<2;
    value = PORTA & _BV(DB5);
    address |= value<<1;
    value = PORTA & _BV(DB4);
    address |= value<<0;

    // set the clock back to low
    PORTA &=~ _BV(E);

    // Operation complete. Set DB4-7 pins back to outputs
    DDRA |= (_BV(DB7)|_BV(DB6)|_BV(DB5)|_BV(DB4)); 

    // Write LCD pins RS back to 0 and RW back to 0
    PORTA &=~ _BV(RS);
    PORTA &=~ _BV(RW);

    // Return the address and whether or not the device is busy
    return (address | busy<<7);
}

void char_write(char mychar)
{
    chars_written++;

    if ((chars_written==17)||(mychar=='\n'))
    {
        if (line_num==1)
        {
            set_ddram_address(0x40);
            line_num = 2;
            chars_written = 0;
        }
        else if (line_num==2)
        {
            lcd_init();
        }
    }

    if (mychar!='\n')
        data_write((uint8_t)mychar);
    
}

void data_write(uint8_t val)
{

    //write data to DD RAM
    PORTA |= _BV(RS);
    PORTA &=~ _BV(RW);
    write_db74((val>>7)&1,(val>>6)&1,(val>>5)&1,(val>>4)&1);
    datarw();
    write_db74((val>>3)&1,(val>>2)&1,(val>>1)&1,(val>>0)&1);
    datarw();
}

void write_db74(int DB7_val, int DB6_val, int DB5_val, int DB4_val)
{
    if (DB7_val == 0) PORTA &=~ _BV(DB7);
    else PORTA |= _BV(DB7);
    if (DB6_val == 0) PORTA &=~ _BV(DB6);
    else PORTA |= _BV(DB6);
    if (DB5_val == 0) PORTA &=~ _BV(DB5);
    else PORTA |= _BV(DB5);
    if (DB4_val == 0) PORTA &=~ _BV(DB4);
    else PORTA |= _BV(DB4);
}

//write a multi-char integer to the LCD as a string
void string_write_int(int num, int num_digits)
{
    char *temp = (char *)malloc(num_digits*sizeof(char));
    if (temp==NULL)
    {
            lcd_init();
            string_write("bad malloc");
            return;
    }
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

void string_write_numchars(char *mystring, int num_chars)
{
    int i;
    get_current_address();
    //printf("writing string\n");
    for (i=0; i<num_chars; i++) {
        
        if (i >= strlen(mystring)) 
            break;
        char_write(mystring[i]);
        get_current_address();
    }
}