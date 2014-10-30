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
#include "database.h"
#include "dev_ops.h"

#define F_CPU 1000000UL

//Global Device State Structure
volatile DEV_STATE *device;


//---- PRIMARY APPLICATION ----//

int main (int argc, char *argv[])
{
    char holder;
    int i, result;

    //allocate memory for the device state structure
    device = (volatile DEV_STATE *)malloc(sizeof(DEV_STATE));
    if (device == NULL)
    {
        lcd_init();
        string_write("bad malloc");
        return 0;
    }

    //set up GPIO, initialize interrupts, prepare for serial comm, init lcd, choose DEV_STATE params
    result = prepare_device(device);

    //catch device prep errors
    if (result==0)
        return 0;

    //create the GPS_DATA struct
    GPS_DATA *gps_data = (GPS_DATA *)malloc(sizeof(GPS_DATA));
    if (gps_data == NULL)
    {
            lcd_init();
            string_write("bad malloc");
            return 0;
    }

    //createthe DATABASE struct
    DATABASE *fm_stations = (DATABASE *)malloc(sizeof(DATABASE));
    if (fm_stations == NULL)
    {
            lcd_init();
            string_write("bad malloc");
            return 0;
    }

    //load in the FM stations database from EEPROM
    string_write("reading\ndatabase...");
    database_load(fm_stations);
    _delay_ms(1000);
    
    //primary program loop
    while(1){
        //remember the current mode of operation
        device->op_mode_prior = device->op_mode;
        //synchronize op_mode led state
        sync_leds(device);
        //behave according to the mode of operation
        switch (device->op_mode)
        {
            case MD_NORMAL:
                //ask for an update if no known stations
                if (fm_stations->num_stations < 1) {
                    device->op_mode = MD_UPDATE_REQUIRED;
                    break;
                }
                //allow gps interrupts
                enable_gps();
                //title screen
                lcd_init();
                string_write("fmHarmony\n");
                _delay_ms(250);
                string_write("UTC: ");
                string_write_numchars(gps_data->utc_time,8);
                _delay_ms(750);
                if (device->op_mode != device->op_mode_prior) break;
                //parse available data and pull formatted params into the GPS_DATA struct
                sync_gps_data(device, gps_data);
                if (gps_locked(gps_data))
                {
                    //compute and show the nearest station
                    show_nearest_station(device, fm_stations, gps_data);
                } else {
                    lcd_init();
                    string_write("No GPS Fix...\n");
                    string_write("Be Patient...");
                    _delay_ms(2000);
                    //print_raw_gps_data(device);
                }
            break;

            case MD_GPS:
                lcd_init();
                //allow gps interrupts
                enable_gps();
                //parse available data and pull formatted params into the GPS_DATA struct
                sync_gps_data(device, gps_data);
                if (gps_locked(gps_data))
                {
                    string_write("GPS Fix:\n");
                    string_write_float(gps_data->lat,3);
                    string_write(", ");
                    string_write_float(gps_data->lon,3);
                    _delay_ms(3000);
                    //print the most recent gps data
                    print_gps_data_concise(device, gps_data);
                } else {
                    string_write("No GPS Fix...\n");
                    string_write("Be Patient...");
                    _delay_ms(2000);
                    print_raw_gps_data(device);
                }
            break;

            case MD_DATABASE:
                lcd_init();
                string_write("FM Stations\nDatabase");
                _delay_ms(3000);
                //go through the complete list of known stations
                print_all_known_stations(device, fm_stations);
            break;

            case MD_GPS_LONG:
                lcd_init();
                //allow gps interrupts
                enable_gps();
                //parse available data and pull formatted params into the GPS_DATA struct
                sync_gps_data(device, gps_data);
                string_write("DEBUG 1");
                _delay_ms(1000);
                print_gps_data(device, gps_data);
                print_raw_gps_data(device);
            break;

            case MD_DEBUG:
                lcd_init();
                //allow gps interrupts
                enable_gps();
                //parse available data and pull formatted params into the GPS_DATA struct
                sync_gps_data(device, gps_data);
                string_write("DEBUG 2");
                _delay_ms(1000);
                lcd_init();
                string_write("CMG: ");
                string_write_float(gps_data->course,1); char_write(DEG_SYMBOL); string_write(" ");
                //write out the abs bearing chars
                for (i=0; i<3; i++)
                    char_write(gps_data->str_course[i]);
                string_write("\nSpeed: ");
                string_write_float(gps_data->speed,1);
                _delay_ms(3000);
            break;                

            case MD_UPDATE_REQUIRED:
                //do nothing until an update is triggered
                wait_for_update(device);
            break;

            case MD_UPDATE:
                //make sure gps interrupts do not disrupt the update process
                disable_gps();
                //handle the update trigger
                if (device->updating == 0)
                {
                    lcd_init();
                    string_write("updating...\ndon't unplug");
                    device->updating = 1;
                    fm_stations->corrupted = 0;
                    device->eeprom_index = 0;

                    //scrap the outdated database structures
                    database_free(fm_stations);
                }
                //read serial data from receive buffer when available
                if(device->rxReadPos != device->rxWritePos) {
                    device->serial_timer = 0;
                    holder = getChar(device);
                    //handle serial transfer end sequence
                    if(detectSerialEnd(device)) {
                        //terminate connection and update the database
                        terminate_serial(device, fm_stations, FL_SUCCESS);
                        //check for database corruption
                        check_database_integrity(fm_stations);
                        if (fm_stations->corrupted)
                        {
                            //wipe 100 stations worth of EEPROM and require a fresh update
                            wipe_eeprom(device);
                            device->op_mode = MD_UPDATE_REQUIRED;
                        } else {
                            lcd_init();
                            string_write("update complete");
                            _delay_ms(1000);
                        }
                    } else {
                        //write real serial data bytes to EEPROM
                        eeprom_write_byte((uint8_t *)device->eeprom_index,holder);
                        device->eeprom_index ++;
                    }
                } else {
                    //no data was available to read this time around; work towards a timeout
                    device->serial_timer ++;
                    if (device->serial_timer > SERIAL_TIMEOUT)
                    {
                        //serial timeout --> close serial connection, wipe memory, and require a fresh update
                        terminate_serial(device, fm_stations, FL_FAIL);
                        //print_eeprom_contents(device, fm_stations, 0,32);
                        wipe_eeprom(device);
                        device->op_mode = MD_UPDATE_REQUIRED;
                    }
                }
            break;
        }
    } //primary program loop
    return 0; //should never get here.
}


//---- PIN CHANGE INTERRUPT (PUSHBUTTON PRESSED) ----//
ISR(INT2_vect) {
    if ((device->button_pressable)&&(device->updating==0))
    {
        //increment the op_mode
        device->op_mode++;
        //loop the op_mode
        if (device->op_mode >= NUM_MODES)
            device->op_mode = 0;
        //light up the mode LEDs
        sync_leds(device);
        //debounce the button
        device->button_pressable = 0;
        //reset the debounce timer
        TCNT1 &= ~(0xFFFF);
    }
}

//---- TIMER INTERRUPT (PUSHBUTTON DEBOUNCE) ----//
ISR(TIMER1_COMPA_vect) {
    //PORTB ^= (1<<PB4);
    //reset the debounce timer
    TCNT1 &= ~(0xFFFF);
    if (device->button_pressable == 0)
    {
        //start accepting new button presses
        device->button_pressable = 1;
    }
}

//---- SERIAL DATABASE UPDATE INTERRUPT ----//
ISR(USART1_RX_vect) {
    //remember the last 3 bytes received (to handle start + end sequences)
    device->serial_history[2] = device->serial_history[1];
    device->serial_history[1] = device->serial_history[0];
    //Read most recent value out of the UART buffer
    device->serial_history[0] = UDR1;
    //if a serial update is in progress, write to the receive buffer
    if (device->op_mode==MD_UPDATE)
    {
        device->rxBuffer[device->rxWritePos] = device->serial_history[0];
        device->rxWritePos++;
    }
    //trigger a serial database update if the start sequence has occurred
    if(detectSerialStart(device)){
        device->op_mode = MD_UPDATE;
    }
    //make the receive buffer loop
    if(device->rxWritePos >= RX_BUFFER_SIZE)
    {
        device->rxWritePos = 0;
    }
}

//---- SERIAL GPS INTERRUPT ----//
ISR(USART0_RX_vect) { 
        int k;
        //prevent buffer overflow
        if (device->gps_rxCount >= GPS_RX_BUFFER_SIZE)
        {
            for (k=0; k<GPS_RX_BUFFER_SIZE; k++)
                device->gps_rxBuffer[k]='\0';

            device->gps_rxCount = 0; 
        }
        //Read value out of the UART buffer
        device->gps_rxBuffer[device->gps_rxCount] = UDR0;
        device->gps_rxCount ++;
        //start new buffer if receive $
        if (device->gps_rxBuffer[device->gps_rxCount-1]=='$')
        {
            for (k=1; k<GPS_RX_BUFFER_SIZE; k++)
                device->gps_rxBuffer[k]='\0'; 

            device->gps_rxBuffer[0] = '$';
            device->gps_rxCount = 1; 
        }
        //carriage return ----> parse the raw sentence data and set the gps struct update trigger
        if ((device->gps_rxBuffer[device->gps_rxCount-1]=='\r')) {
            if (tag_check(device))
            {
                //no more gps interrupts are needed (or desired) for now
                disable_gps();
                //trigger a gps_data struct update
                device->gps_update_trigger = 1;
            }
        }
}