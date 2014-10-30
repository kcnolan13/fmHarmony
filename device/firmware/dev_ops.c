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

#include "dev_ops.h"
#include "lcd.h"
#include "database.h"
#include "geolocation.h"

//synchronize op mode LEDs
void sync_leds(volatile DEV_STATE *device)
{
    //light up the mode LEDs
    switch (device->op_mode)
    {
        case 0:
            PORTB |= 1<<PB0;
            PORTB &= ~((1<<PB3)|(1<<PB1));
        break;
        case 1:
            PORTB |= 1<<PB1;
            PORTB &= ~((1<<PB3)|(1<<PB0));
        break;
        case 2:
            PORTB |= 1<<PB3;
            PORTB &= ~((1<<PB1)|(1<<PB0));
        break;
        case 3:
            PORTB |= ((1<<PB1)|(1<<PB0));
            PORTB &= ~(1<<PB3);
        break;
        case 4:
            PORTB |= ((1<<PB3)|(1<<PB1));
            PORTB &= ~(1<<PB0);
        break;
        default:
            PORTB |= ((1<<PB3)|(1<<PB1)|(1<<PB0));
        break;
    }
}

//Initialize USART
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

//Set Up External Interrupt 2
void InitINT(void)
{
    //trigger external interrupt 2 on falling edge
    EICRA |= (1 << ISC21);
    EICRA &= ~(1<<ISC20);

    //enable external interrupt 2 
    EIMSK |= (1 << INT2);

    //set Port B Pin 2 as Input
    DDRB &= ~(1<<PB2);
    //enable pull-up resistor
    PORTB |= (1<<PB2);

    //enable PB0, PB1, PB3 as Outputs
    DDRB |= ((1<<PB0)|(1<<PB1)|(1<<PB3));

    //turn on the first LED
    PORTB |= (1<<PB0);


    //SET UP TIMER FOR DEBOUNCING

    //set normal output compare modes
    TCCR1A &= ~((1<<COM1A1)|(1<<COM1A0));
    
    //prescale the default clock by /1024
    TCCR1B |= ((1<<CS12)|(1<<CS10));
    TCCR1B &= ~((1<<CS11));

    //set the max internal counter value
    uint8_t debounce_delay = 122; //roughly 1/8th of a second
    OCR1AH &= ~(0xFF);
    OCR1AL |= 0xFF;
    OCR1AL &= debounce_delay;

    //enable the timer interrupt
    TIMSK1 |= (1<<OCIE1A);
}

//set up GPIO, initialize interrupts, serial comm, and LCD
int prepare_device(volatile DEV_STATE *device)
{
    int i, j;
    DDRB = 0xFF;

    cli();

    //Init usart
    InitUSART();

    //Enable Pin Change Interrupts (for the pushbutton)
    InitINT();

    //Enable GPS Interrupts
    enable_gps();

    //Enable Global Interrupts. Sets SREG Interrupt bit.
    sei();

    //blink PB0 during the startup sequence
    device->blinking = 1;
    device->blinker1 = 0;
    device->blinker2 = -2;
    device->blinker3 = -2;

    //Intitialize LCD. Set Blinking cursor.
    lcd_init();
    _delay_ms(1000);

    //choose starting DEV_STATE params
    device->serial_timer = 0;
    device->updating = 0;
    device->eeprom_index = 0;
    device->op_mode = MD_NORMAL;
    device->op_mode_prior = MD_NORMAL;
    device->serialStartChar = '$';
    device->serialEndChar = '^';

    device->rxReadPos = 0;
    device->rxWritePos = 0;
    device->gps_update_trigger = 0;
    device->button_pressable = 1;

    for (i=0; i<3; i++)
    {
        device->serial_history[i] = 'K';
    }

    device->gps_rxCount = 0;

    for (i=0; i<RX_BUFFER_SIZE; i++)
        device->rxBuffer[i] = '\0';

    for (i=0; i<GPS_RX_BUFFER_SIZE; i++)
        device->gps_rxBuffer[i] = '\0';

    //allocate memory for each of the raw_gps_data fields
    for (i=0; i<NUM_GPS_FIELDS; i++)
    {
        device->raw_gps_data[i] = (char *)malloc(GPS_FIELD_LEN*sizeof(char));
        if (device->raw_gps_data[i] == NULL)
        {
            lcd_init();
            string_write("bad malloc");
            return 0;
        } else {
            //null out each of the raw gps data strings
            for (j=0; j<GPS_FIELD_LEN; j++)
            {
                device->raw_gps_data[i][j] = '\0';
            }
        }
    }

    return 1;
}

//turn off GPS intterupts, receiver, and transmitter
void disable_gps(void)
{
    UCSR0B &= ~((1 << RXCIE0) | (1 << TXCIE0) | (1 << RXEN0) | (1 << TXEN0)); // 
}

//turn on GPS interrupts, receiver, and transmitter
void enable_gps(void)
{
    UCSR0B |= (1 << RXCIE0) | (1 << TXCIE0) | (1 << RXEN0) | (1 << TXEN0); //
}

//parse available GPS data and pull formatted params into the GPS_DATA struct
void sync_gps_data(volatile DEV_STATE *device, GPS_DATA *gps_data)
{
    if (device->gps_update_trigger)
    {
        //strip off the rxBuffer carriage return and replace with ,
        device->gps_rxBuffer[device->gps_rxCount-1] = ',';
        //parse the sentence and populate the raw_gps_data fields
        parse_nmea(device, device->gps_rxBuffer, device->raw_gps_data);
        //use the raw raw_gps_data fields to populate the GPS_DATA struct
        update_user_gps_data(device->raw_gps_data, gps_data);
        device->gps_update_trigger = 0;
    }
}

//---- UPDATE UTILITIES ----//

//catch the serial update start sequence
int detectSerialStart(volatile DEV_STATE *device)
{
    if ((device->serial_history[0]==device->serialStartChar)&&(device->serial_history[1]==device->serialStartChar)&&(device->serial_history[2]==device->serialStartChar))
        return 1;
    else
        return 0;
}

//catch the serial update end sequence
int detectSerialEnd(volatile DEV_STATE *device)
{
    if ((device->serial_history[0]==device->serialEndChar)&&(device->serial_history[1]==device->serialEndChar)&&(device->serial_history[2]==device->serialEndChar))
        return 1;
    else
        return 0;
}

//read a char from the serial update buffer
char getChar(volatile DEV_STATE *device)
{
    char ret = '\0';
    
    ret = device->rxBuffer[device->rxReadPos];
     
    device->rxReadPos++;
     
    if(device->rxReadPos >= RX_BUFFER_SIZE)
    {
        device->rxReadPos = 0;
    }
    
    return ret;
}

//peek at the next char in the serial update buffer
char peekChar(volatile DEV_STATE *device)
{
    char ret = '\0';
     
    if(device->rxReadPos != device->rxWritePos)
    {
        ret = device->rxBuffer[device->rxReadPos];
    }
     
    return ret;
}

//terminate the serial update with a certain status
int terminate_serial(volatile DEV_STATE *device, DATABASE *fm_stations, int flag)
{
    device->op_mode = MD_NORMAL;
    device->updating = 0;
    device->serial_timer = 0;

    //turn off the blinking update LEDs
    device->blinking = 0;
    sync_leds(device);

    lcd_init();

    if (flag==FL_SUCCESS)
        string_write("reading\ndatabase ...");
    else
        string_write("ERROR:\ntimeout ...");

    //import the new database
    database_load(fm_stations);
    _delay_ms(1000);

    if (flag==FL_FAIL)
        string_write("\nupdate failed\n");

    _delay_ms(500);
    return 1;
}

//wipe 100-stations-worth of EEPROM data
void wipe_eeprom(volatile DEV_STATE *device)
{
    int i;
    lcd_init();
    string_write("wiping \nmemory...");
    for (i=0; i<FIRST_STATION_OFFSET+100*STATION_BLOCKSIZE; i++)
    {
        if (device->op_mode != device->op_mode_prior) return;
        eeprom_write_byte((uint8_t *)i,255);
    }
}

//compute the nearest station and display pertinent, formatted info
void show_nearest_station(volatile DEV_STATE *device, DATABASE *fm_stations, GPS_DATA *gps_data)
{
    int i;
    if (device->op_mode != device->op_mode_prior) return;
    lcd_init();
    string_write("Finding Nearest\nStation...");
    _delay_ms(500);

    lcd_init();

    //calculate the nearest station
    fm_stations->nearest_station = get_nearest_station(fm_stations->all_stations, fm_stations->num_stations, gps_data->lat, gps_data->lon);

    //calculate bearings to nearest station
    calculate_bearings(gps_data, fm_stations);

    //print formatted info to the display
    print_callsign(fm_stations, fm_stations->nearest_station); string_write(" "); string_write_float(fm_stations->all_stations[fm_stations->nearest_station].freq,1); string_write("\n");
    string_write_float(my_distance_to_station(gps_data, fm_stations->all_stations, fm_stations->nearest_station),1); string_write(" km, ");

    //write out the absolute bearing chars
    for (i=0; i<3; i++)
        char_write(gps_data->str_abs_bearing_nearest[i]);

    if (device->op_mode != device->op_mode_prior) return;

    _delay_ms(3000);

    if (device->op_mode != device->op_mode_prior) return;

    lcd_init();

    //print bearings to the screen
    string_write("A.Bear: "); string_write_float(gps_data->abs_bearing_nearest,1); char_write(DEG_SYMBOL); char_write('\n');
    string_write("R.Bear: "); string_write_float(gps_data->rel_bearing_nearest,1); char_write(DEG_SYMBOL);

    if (device->op_mode != device->op_mode_prior) return;

    _delay_ms(3000);
}

//hold device state and wait for database update
void wait_for_update(volatile DEV_STATE *device)
{
    lcd_init();
    string_write("update required\n...feed me...");
    while (1)
    {
        if (device->op_mode != device->op_mode_prior) return;
    }
}

//print the EEPROM contents for all stations
void print_eeprom_station_contents(volatile DEV_STATE *device, DATABASE *fm_stations)
{
    int start = FIRST_STATION_OFFSET;
    int i=0;
    char one_byte;

    for (i=0; i<STATION_BLOCKSIZE*fm_stations->num_stations; i++)
    {
        if (device->op_mode != device->op_mode_prior) return;
        one_byte = my_eeprom_read_char(start+i);
        if (one_byte == '\0')
            one_byte = '?';
        char_write(one_byte);
        _delay_ms(100);
    }
}

//print the EEPROM contents for an address range
void print_eeprom_contents(volatile DEV_STATE *device, DATABASE *fm_stations, int start_addr, int end_addr)
{
    int i=0;
    char one_byte;

    if (end_addr == -1)
        end_addr = 1+fm_stations->num_stations*STATION_BLOCKSIZE;

    lcd_init();

    for (i=start_addr; i<end_addr; i++)
    {
        if (device->op_mode != device->op_mode_prior) return;
        one_byte = my_eeprom_read_char(i);
        if (one_byte == '\0')
            one_byte = '?';
        char_write(one_byte);
        _delay_ms(100);
    }
}

//print the information held for all stations to the screen
void print_all_known_stations(volatile DEV_STATE *device, DATABASE *fm_stations)
{
    if (device->op_mode != device->op_mode_prior) return;
    int i;
    lcd_init();
    string_write_int(fm_stations->num_stations,3);
    string_write(" known\nstations");

    _delay_ms(2000);

    for (i=0; i<fm_stations->num_stations; i++)
    {
        if (device->op_mode != device->op_mode_prior) return;

        lcd_init();
        print_station(device, fm_stations, i);

        if (device->op_mode != device->op_mode_prior) return;

        _delay_ms(200);   
    }
}

//quickly print all known callsigns to the screen
void print_all_callsigns(volatile DEV_STATE *device, DATABASE *fm_stations)
{
    if (device->op_mode != device->op_mode_prior) return;
    int i;
    lcd_init();
    string_write_int(fm_stations->num_stations,3);
    string_write(" known\nstations");
    _delay_ms(2000);

    lcd_init();
    char_write('\n');

    for (i=0; i<fm_stations->num_stations; i++)
    {
        if (device->op_mode != device->op_mode_prior) return;

        char_write('\n'); string_write_int(i+1,3); string_write(": "); print_callsign(fm_stations, i);

        if (device->op_mode != device->op_mode_prior) return;

        _delay_ms(250);   
    }
}

//print the callsign for a given station index to the LCD
void print_callsign(DATABASE *fm_stations, int station_index)
{
    int i;
    for (i=0; i<8; i++) 
    {
        char_write(fm_stations->all_stations[station_index].callsign[i]);
    }
}

//print the informatoin held for a single station to the LCD
void print_station(volatile DEV_STATE *device, DATABASE *fm_stations, int index)
{
    string_write_int(index+1,3); string_write(": "); print_callsign(fm_stations, index); _delay_ms(250); string_write("\n"); 
    if (device->op_mode==MD_UPDATE)
        return;
    string_write("freq: "); string_write_float(fm_stations->all_stations[index].freq,1); _delay_ms(250); string_write("\n");
    if (device->op_mode==MD_UPDATE)
        return;
    string_write("lat: "); string_write_float(fm_stations->all_stations[index].lat,4); _delay_ms(250); string_write("\n");
    if (device->op_mode==MD_UPDATE)
        return;
    string_write("lon: "); string_write_float(fm_stations->all_stations[index].lon,4); _delay_ms(250); string_write("\n");
    if (device->op_mode==MD_UPDATE)
        return;
    string_write("erp: "); string_write_float(fm_stations->all_stations[index].erp,1); _delay_ms(250); string_write("\n");
    if (device->op_mode==MD_UPDATE)
        return;
    string_write("haat: "); string_write_float(fm_stations->all_stations[index].haat,0); _delay_ms(250); string_write("\n");
    if (device->op_mode==MD_UPDATE)
        return;
}

//print the formatted data stored in the GPS_DATA struct to the screen
void print_gps_data(volatile DEV_STATE *device, GPS_DATA *gps_data)
{
    if (device->op_mode != device->op_mode_prior) return;
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
                string_write_numchars(gps_data->msg_type,8);
            break;

            case 1:
                string_write("Time: ");
                string_write_numchars(gps_data->utc_time,8);
            break;

            case 2:
                string_write("NRW: ");
                char_write(gps_data->nrw);
            break;

            case 3:
                string_write("Lat: ");
                string_write_float(gps_data->lat,4); char_write(DEG_SYMBOL);
            break;

            case 4:
                string_write("Lon: ");
                string_write_float(gps_data->lon,4); char_write(DEG_SYMBOL);
            break;

            case 5:
                string_write("Speed: ");
                string_write_float(gps_data->speed,1); string_write(" kts");
            break;

            case 6:
                string_write("Course: ");
                string_write_float(gps_data->course,1);
            break;

            case 7:
                string_write("Date: ");
                string_write_numchars(gps_data->date,8);
            break;

            case 8:
                string_write("MagVar: ");
                string_write_numchars(gps_data->mag_var,8);
            break;

            case 9:
                string_write("Mode: ");
                char_write(gps_data->mode);
            break;

            case 10:
                string_write("Checksum: ");
                string_write_numchars(gps_data->checksum,3);
            break;
        }

        _delay_ms(500);
        if (device->op_mode != device->op_mode_prior) return;
    }
}

//print a concise version of the formatted GPS data to the screen
void print_gps_data_concise(volatile DEV_STATE *device, GPS_DATA *gps_data)
{
    int i;
    if (device->op_mode != device->op_mode_prior) return;

    lcd_init();
    string_write("Date: ");
    string_write_numchars(gps_data->date,8); char_write('\n');
    string_write("UTC: ");
    string_write_numchars(gps_data->utc_time,8);
    _delay_ms(2000);

    if (device->op_mode != device->op_mode_prior) return;

    lcd_init();
    string_write("Lat: ");
    string_write_float(gps_data->lat,4);  char_write(DEG_SYMBOL); char_write('\n');
    string_write("Lon: ");
    string_write_float(gps_data->lon,4);  char_write(DEG_SYMBOL);
    _delay_ms(2000);

    if (device->op_mode != device->op_mode_prior) return;

    lcd_init();
    string_write("CMG: ");
    string_write_float(gps_data->course,1); char_write(DEG_SYMBOL); string_write(" ");

    //write out the absolute bearing chars
    for (i=0; i<3; i++)
        char_write(gps_data->str_course[i]);

    string_write("\nSpeed: ");
    string_write_float(gps_data->speed,1);  string_write(" kts");
    _delay_ms(2000);
}

//print the raw gps data in the gps_data string array to the screen
void print_raw_gps_data(volatile DEV_STATE *device)
{
    if (device->op_mode != device->op_mode_prior) return;

    lcd_init();
    string_write("Raw\nGPS Data:");
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
        string_write(device->raw_gps_data[i]);

        _delay_ms(500);
        if (device->op_mode != device->op_mode_prior) return;
    }
}

//crosscheck the earth-distance formula with a few known distances
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

//check for database corruption
int check_database_integrity(DATABASE *fm_stations)
{
    int i, j;
    for (i=0; i < fm_stations->num_stations; i++)
    {
        char * call = fm_stations->all_stations[i].callsign;
        for (j=0; j<8; j++)
        {
            //indicate corruption if any station callsigns contain abnormal characters
            if (((call[j] < 33)||(call[j] > 126))&&(call[j]!=' '))
            {
                fm_stations->corrupted = 1;
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
                print_callsign(fm_stations, i);
                _delay_ms(4000);
                //database is corrupted
                return 0;
            }
        }
    }
    //database is not corrupted
    fm_stations->corrupted = 0;
    return 1;
}