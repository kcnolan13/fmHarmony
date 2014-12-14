EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:kyles_library
LIBS:fmHarmony-cache
EELAYER 27 0
EELAYER END
$Descr USLetter 11000 8500
encoding utf-8
Sheet 1 1
Title ""
Date "13 dec 2014"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L ATMEGA1284 IC2
U 1 1 5417310A
P 7250 4300
F 0 "IC2" H 6400 6180 40  0000 L BNN
F 1 "ATMEGA1284" H 7600 3500 40  0000 L BNN
F 2 "DIL40" H 7250 3850 30  0000 C CIN
F 3 "" H 7250 3850 60  0000 C CNN
	1    7250 4300
	1    0    0    -1  
$EndComp
$Comp
L R R1
U 1 1 54173710
P 5150 3800
F 0 "R1" V 5230 3800 40  0000 C CNN
F 1 "1K" V 5157 3801 40  0000 C CNN
F 2 "" V 5080 3800 30  0000 C CNN
F 3 "" H 5150 3800 30  0000 C CNN
	1    5150 3800
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR?
U 1 1 54173864
P 5150 4100
F 0 "#PWR?" H 5150 4200 30  0001 C CNN
F 1 "VCC" H 5150 4200 30  0000 C CNN
F 2 "" H 5150 4100 60  0000 C CNN
F 3 "" H 5150 4100 60  0000 C CNN
	1    5150 4100
	-1   0    0    1   
$EndComp
$Comp
L VCC #PWR?
U 1 1 54173B40
P 5500 3500
F 0 "#PWR?" H 5500 3600 30  0001 C CNN
F 1 "VCC" H 5500 3600 30  0000 C CNN
F 2 "" H 5500 3500 60  0000 C CNN
F 3 "" H 5500 3500 60  0000 C CNN
	1    5500 3500
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 54173B8B
P 5500 3900
F 0 "#PWR?" H 5500 3900 30  0001 C CNN
F 1 "GND" H 5500 3830 30  0001 C CNN
F 2 "" H 5500 3900 60  0000 C CNN
F 3 "" H 5500 3900 60  0000 C CNN
	1    5500 3900
	1    0    0    -1  
$EndComp
Text Notes 6650 7750 0    60   ~ 0
FM Harmony Project Schematic
$Comp
L POT VR1
U 1 1 5417656B
P 8450 2000
F 0 "VR1" H 8450 1900 50  0000 C CNN
F 1 "10K" H 8450 2000 50  0000 C CNN
F 2 "" H 8450 2000 60  0000 C CNN
F 3 "" H 8450 2000 60  0000 C CNN
	1    8450 2000
	-1   0    0    1   
$EndComp
$Comp
L GND #PWR?
U 1 1 5417678B
P 8200 2100
F 0 "#PWR?" H 8200 2100 30  0001 C CNN
F 1 "GND" H 8200 2030 30  0001 C CNN
F 2 "" H 8200 2100 60  0000 C CNN
F 3 "" H 8200 2100 60  0000 C CNN
	1    8200 2100
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR?
U 1 1 5417679F
P 8700 1900
F 0 "#PWR?" H 8700 2000 30  0001 C CNN
F 1 "VCC" H 8700 2000 30  0000 C CNN
F 2 "" H 8700 1900 60  0000 C CNN
F 3 "" H 8700 1900 60  0000 C CNN
	1    8700 1900
	1    0    0    -1  
$EndComp
$Comp
L LCD16X2 LCD1
U 1 1 5416EEC2
P 9600 3050
F 0 "LCD1" H 8800 3450 40  0000 C CNN
F 1 "LCD16X2" H 10300 3450 40  0000 C CNN
F 2 "WC1602A" H 9600 3000 35  0000 C CIN
F 3 "" H 9600 3050 60  0000 C CNN
	1    9600 3050
	0    1    1    0   
$EndComp
$Comp
L GND #PWR?
U 1 1 54177862
P 8900 3250
F 0 "#PWR?" H 8900 3250 30  0001 C CNN
F 1 "GND" H 8900 3180 30  0001 C CNN
F 2 "" H 8900 3250 60  0000 C CNN
F 3 "" H 8900 3250 60  0000 C CNN
	1    8900 3250
	1    0    0    -1  
$EndComp
$Comp
L R R2
U 1 1 54177ABA
P 8700 3700
F 0 "R2" V 8780 3700 40  0000 C CNN
F 1 "120" V 8707 3701 40  0000 C CNN
F 2 "" V 8630 3700 30  0000 C CNN
F 3 "" H 8700 3700 30  0000 C CNN
	1    8700 3700
	0    1    1    0   
$EndComp
$Comp
L VCC #PWR?
U 1 1 54177B7E
P 8450 3550
F 0 "#PWR?" H 8450 3650 30  0001 C CNN
F 1 "VCC" H 8450 3650 30  0000 C CNN
F 2 "" H 8450 3550 60  0000 C CNN
F 3 "" H 8450 3550 60  0000 C CNN
	1    8450 3550
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 54177CDD
P 9000 3900
F 0 "#PWR?" H 9000 3900 30  0001 C CNN
F 1 "GND" H 9000 3830 30  0001 C CNN
F 2 "" H 9000 3900 60  0000 C CNN
F 3 "" H 9000 3900 60  0000 C CNN
	1    9000 3900
	1    0    0    -1  
$EndComp
$Comp
L ZENER D1
U 1 1 5424A46E
P 2550 1700
F 0 "D1" H 2550 1800 50  0000 C CNN
F 1 "TVS" H 2550 1600 40  0000 C CNN
F 2 "~" H 2550 1700 60  0000 C CNN
F 3 "~" H 2550 1700 60  0000 C CNN
	1    2550 1700
	0    -1   -1   0   
$EndComp
$Comp
L DIODE D2
U 1 1 5424A48B
P 2200 1350
F 0 "D2" H 2200 1450 40  0000 C CNN
F 1 "1N4004" H 2200 1250 40  0000 C CNN
F 2 "~" H 2200 1350 60  0000 C CNN
F 3 "~" H 2200 1350 60  0000 C CNN
	1    2200 1350
	1    0    0    -1  
$EndComp
$Comp
L FUSE F1
U 1 1 5424A4A5
P 1600 1350
F 0 "F1" H 1700 1400 40  0000 C CNN
F 1 "2A" H 1500 1300 40  0000 C CNN
F 2 "~" H 1600 1350 60  0000 C CNN
F 3 "~" H 1600 1350 60  0000 C CNN
	1    1600 1350
	1    0    0    -1  
$EndComp
$Comp
L FUSE F2
U 1 1 5424A4B4
P 4850 1350
F 0 "F2" H 4950 1400 40  0000 C CNN
F 1 "250 mA" H 4750 1300 40  0000 C CNN
F 2 "~" H 4850 1350 60  0000 C CNN
F 3 "~" H 4850 1350 60  0000 C CNN
	1    4850 1350
	1    0    0    -1  
$EndComp
$Comp
L C C2
U 1 1 5424AA0A
P 3350 1700
F 0 "C2" H 3350 1800 40  0000 L CNN
F 1 "100n" H 3356 1615 40  0000 L CNN
F 2 "~" H 3388 1550 30  0000 C CNN
F 3 "~" H 3350 1700 60  0000 C CNN
	1    3350 1700
	1    0    0    -1  
$EndComp
$Comp
L CP1 C1
U 1 1 5424AA4E
P 2950 1700
F 0 "C1" H 3000 1800 50  0000 L CNN
F 1 "1u" H 3000 1600 50  0000 L CNN
F 2 "~" H 2950 1700 60  0000 C CNN
F 3 "~" H 2950 1700 60  0000 C CNN
	1    2950 1700
	1    0    0    -1  
$EndComp
$Comp
L CP1 C3
U 1 1 5424AA5D
P 6450 1600
F 0 "C3" H 6500 1700 50  0000 L CNN
F 1 "1u" H 6500 1500 50  0000 L CNN
F 2 "~" H 6450 1600 60  0000 C CNN
F 3 "~" H 6450 1600 60  0000 C CNN
	1    6450 1600
	1    0    0    -1  
$EndComp
$Comp
L 7805 IC1
U 1 1 5424AB68
P 4000 1400
F 0 "IC1" H 4150 1204 60  0000 C CNN
F 1 "7805" H 4000 1600 60  0000 C CNN
F 2 "" H 4000 1400 60  0000 C CNN
F 3 "" H 4000 1400 60  0000 C CNN
	1    4000 1400
	1    0    0    -1  
$EndComp
Text GLabel 950  1350 0    60   Input ~ 0
JP1
$Comp
L GND #PWR?
U 1 1 5424ADF7
P 1000 2000
F 0 "#PWR?" H 1000 2000 30  0001 C CNN
F 1 "GND" H 1000 1930 30  0001 C CNN
F 2 "" H 1000 2000 60  0000 C CNN
F 3 "" H 1000 2000 60  0000 C CNN
	1    1000 2000
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR?
U 1 1 5424B3C1
P 6650 1400
F 0 "#PWR?" H 6650 1500 30  0001 C CNN
F 1 "VCC" H 6650 1500 30  0000 C CNN
F 2 "" H 6650 1400 60  0000 C CNN
F 3 "" H 6650 1400 60  0000 C CNN
	1    6650 1400
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 5424B4D9
P 5600 2000
F 0 "#PWR?" H 5600 2000 30  0001 C CNN
F 1 "GND" H 5600 1930 30  0001 C CNN
F 2 "" H 5600 2000 60  0000 C CNN
F 3 "" H 5600 2000 60  0000 C CNN
	1    5600 2000
	1    0    0    -1  
$EndComp
Text Notes 1050 1050 0    60   ~ 0
Protective Circuitry: Cigarette Lighter Input
$Comp
L C C4
U 1 1 5424B599
P 5500 3650
F 0 "C4" H 5500 3750 40  0000 L CNN
F 1 "100n" H 5506 3565 40  0000 L CNN
F 2 "~" H 5538 3500 30  0000 C CNN
F 3 "~" H 5500 3650 60  0000 C CNN
	1    5500 3650
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR?
U 1 1 5424B654
P 3400 4900
F 0 "#PWR?" H 3400 5000 30  0001 C CNN
F 1 "VCC" H 3400 5000 30  0000 C CNN
F 2 "" H 3400 4900 60  0000 C CNN
F 3 "" H 3400 4900 60  0000 C CNN
	1    3400 4900
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 5424B65A
P 3400 5300
F 0 "#PWR?" H 3400 5300 30  0001 C CNN
F 1 "GND" H 3400 5230 30  0001 C CNN
F 2 "" H 3400 5300 60  0000 C CNN
F 3 "" H 3400 5300 60  0000 C CNN
	1    3400 5300
	1    0    0    -1  
$EndComp
$Comp
L C C5
U 1 1 5424B662
P 3400 5050
F 0 "C5" H 3400 5150 40  0000 L CNN
F 1 "100n" H 3406 4965 40  0000 L CNN
F 2 "~" H 3438 4900 30  0000 C CNN
F 3 "~" H 3400 5050 60  0000 C CNN
	1    3400 5050
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 5424B722
P 4000 3850
F 0 "#PWR?" H 4000 3850 30  0001 C CNN
F 1 "GND" H 4000 3780 30  0001 C CNN
F 2 "" H 4000 3850 60  0000 C CNN
F 3 "" H 4000 3850 60  0000 C CNN
	1    4000 3850
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR?
U 1 1 5424B83F
P 9450 1600
F 0 "#PWR?" H 9450 1700 30  0001 C CNN
F 1 "VCC" H 9450 1700 30  0000 C CNN
F 2 "" H 9450 1600 60  0000 C CNN
F 3 "" H 9450 1600 60  0000 C CNN
	1    9450 1600
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 5424B845
P 9450 2000
F 0 "#PWR?" H 9450 2000 30  0001 C CNN
F 1 "GND" H 9450 1930 30  0001 C CNN
F 2 "" H 9450 2000 60  0000 C CNN
F 3 "" H 9450 2000 60  0000 C CNN
	1    9450 2000
	1    0    0    -1  
$EndComp
$Comp
L C C6
U 1 1 5424B84B
P 9450 1750
F 0 "C6" H 9450 1850 40  0000 L CNN
F 1 "100n" H 9456 1665 40  0000 L CNN
F 2 "~" H 9488 1600 30  0000 C CNN
F 3 "~" H 9450 1750 60  0000 C CNN
	1    9450 1750
	1    0    0    -1  
$EndComp
Text Notes 6750 1400 0    60   ~ 0
+5V\n
$Comp
L GND #PWR?
U 1 1 5426F810
P 5750 5950
F 0 "#PWR?" H 5750 5950 30  0001 C CNN
F 1 "GND" H 5750 5880 30  0001 C CNN
F 2 "" H 5750 5950 60  0000 C CNN
F 3 "" H 5750 5950 60  0000 C CNN
	1    5750 5950
	1    0    0    -1  
$EndComp
$Comp
L C C7
U 1 1 5426F816
P 5750 5650
F 0 "C7" H 5750 5750 40  0000 L CNN
F 1 "100n" H 5756 5565 40  0000 L CNN
F 2 "~" H 5788 5500 30  0000 C CNN
F 3 "~" H 5750 5650 60  0000 C CNN
	1    5750 5650
	1    0    0    -1  
$EndComp
$Comp
L SHEAFF-MONK_PROGRAMMER JP2
U 1 1 5426FD03
P 4600 4600
F 0 "JP2" V 4200 4950 60  0000 C CNN
F 1 "SHEAFF-MONK_PROGRAMMER" H 4650 4500 60  0000 C CNN
F 2 "" H 4650 4500 60  0000 C CNN
F 3 "" H 4650 4500 60  0000 C CNN
	1    4600 4600
	1    0    0    -1  
$EndComp
$Comp
L ULTIMATE_GPS_BREAKOUT_V3 IC3
U 1 1 5426FD12
P 4000 5950
F 0 "IC3" H 3450 7050 60  0000 C CNN
F 1 "ULTIMATE_GPS_BREAKOUT_V3" H 4000 5900 60  0000 C CNN
F 2 "" H 3450 7050 60  0000 C CNN
F 3 "" H 3450 7050 60  0000 C CNN
	1    4000 5950
	0    1    1    0   
$EndComp
$Comp
L GND #PWR?
U 1 1 5428D3A8
P 8350 3550
F 0 "#PWR?" H 8350 3550 30  0001 C CNN
F 1 "GND" H 8350 3480 30  0001 C CNN
F 2 "" H 8350 3550 60  0000 C CNN
F 3 "" H 8350 3550 60  0000 C CNN
	1    8350 3550
	1    0    0    -1  
$EndComp
$Comp
L JUMPER3 JP4
U 1 1 542C4C4B
P 6450 1050
F 0 "JP4" H 6500 950 40  0000 L CNN
F 1 "JUMPER3" H 6450 1150 40  0000 C CNN
F 2 "~" H 6450 1050 60  0000 C CNN
F 3 "~" H 6450 1050 60  0000 C CNN
	1    6450 1050
	1    0    0    -1  
$EndComp
$Comp
L SHEAFF-MONK_SERIAL_TRANSLATOR IC4
U 1 1 5426F555
P 7200 7100
F 0 "IC4" H 6400 9030 40  0000 L BNN
F 1 "SHEAFF-MONK_SERIAL_TRANSLATOR" H 7100 6950 40  0000 L BNN
F 2 "DIL32" H 7250 7200 30  0000 C CIN
F 3 "" H 7250 7200 60  0000 C CNN
	1    7200 7100
	1    0    0    -1  
$EndComp
Text Notes 6050 950  0    60   ~ 0
Car
Text Notes 6700 950  0    60   ~ 0
Update
$Comp
L GND #PWR?
U 1 1 54316390
P 6450 2000
F 0 "#PWR?" H 6450 2000 30  0001 C CNN
F 1 "GND" H 6450 1930 30  0001 C CNN
F 2 "" H 6450 2000 60  0000 C CNN
F 3 "" H 6450 2000 60  0000 C CNN
	1    6450 2000
	1    0    0    -1  
$EndComp
$Comp
L SW_PUSH SW1
U 1 1 5456CDD5
P 5400 2800
F 0 "SW1" H 5550 2910 50  0000 C CNN
F 1 " " H 5400 2720 50  0000 C CNN
F 2 "~" H 5400 2800 60  0000 C CNN
F 3 "~" H 5400 2800 60  0000 C CNN
	1    5400 2800
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 5456CE68
P 5100 2900
F 0 "#PWR?" H 5100 2900 30  0001 C CNN
F 1 "GND" H 5100 2830 30  0001 C CNN
F 2 "" H 5100 2900 60  0000 C CNN
F 3 "" H 5100 2900 60  0000 C CNN
	1    5100 2900
	1    0    0    -1  
$EndComp
$Comp
L R R3
U 1 1 5456CF07
P 5500 2400
F 0 "R3" V 5580 2400 40  0000 C CNN
F 1 "100" V 5507 2401 40  0000 C CNN
F 2 "" V 5430 2400 30  0000 C CNN
F 3 "" H 5500 2400 30  0000 C CNN
	1    5500 2400
	0    -1   -1   0   
$EndComp
$Comp
L R R4
U 1 1 5456CF0E
P 5500 2550
F 0 "R4" V 5580 2550 40  0000 C CNN
F 1 "100" V 5507 2551 40  0000 C CNN
F 2 "" V 5430 2550 30  0000 C CNN
F 3 "" H 5500 2550 30  0000 C CNN
	1    5500 2550
	0    -1   -1   0   
$EndComp
$Comp
L LED LED1
U 1 1 5456CF16
P 4950 2400
F 0 "LED1" H 4950 2500 50  0000 C CNN
F 1 " " H 4950 2300 50  0000 C CNN
F 2 "~" H 4950 2400 60  0000 C CNN
F 3 "~" H 4950 2400 60  0000 C CNN
	1    4950 2400
	-1   0    0    1   
$EndComp
$Comp
L LED LED2
U 1 1 5456CF41
P 4950 2600
F 0 "LED2" H 4950 2700 50  0000 C CNN
F 1 " " H 4950 2500 50  0000 C CNN
F 2 "~" H 4950 2600 60  0000 C CNN
F 3 "~" H 4950 2600 60  0000 C CNN
	1    4950 2600
	-1   0    0    1   
$EndComp
$Comp
L GND #PWR?
U 1 1 5456CF47
P 4650 2700
F 0 "#PWR?" H 4650 2700 30  0001 C CNN
F 1 "GND" H 4650 2630 30  0001 C CNN
F 2 "" H 4650 2700 60  0000 C CNN
F 3 "" H 4650 2700 60  0000 C CNN
	1    4650 2700
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 5456CF4D
P 4650 2500
F 0 "#PWR?" H 4650 2500 30  0001 C CNN
F 1 "GND" H 4650 2430 30  0001 C CNN
F 2 "" H 4650 2500 60  0000 C CNN
F 3 "" H 4650 2500 60  0000 C CNN
	1    4650 2500
	1    0    0    -1  
$EndComp
$Comp
L R R5
U 1 1 5456D29A
P 5850 3000
F 0 "R5" V 5930 3000 40  0000 C CNN
F 1 "100" V 5857 3001 40  0000 C CNN
F 2 "" V 5780 3000 30  0000 C CNN
F 3 "" H 5850 3000 30  0000 C CNN
	1    5850 3000
	0    -1   -1   0   
$EndComp
$Comp
L LED LED3
U 1 1 5456D2A0
P 5300 3000
F 0 "LED3" H 5300 3100 50  0000 C CNN
F 1 " " H 5300 2900 50  0000 C CNN
F 2 "~" H 5300 3000 60  0000 C CNN
F 3 "~" H 5300 3000 60  0000 C CNN
	1    5300 3000
	-1   0    0    1   
$EndComp
$Comp
L GND #PWR?
U 1 1 5456D34B
P 5050 3050
F 0 "#PWR?" H 5050 3050 30  0001 C CNN
F 1 "GND" H 5050 2980 30  0001 C CNN
F 2 "" H 5050 3050 60  0000 C CNN
F 3 "" H 5050 3050 60  0000 C CNN
	1    5050 3050
	1    0    0    -1  
$EndComp
Connection ~ 3350 1350
Wire Wire Line
	3350 1350 3350 1500
Connection ~ 2950 1350
Wire Wire Line
	2950 1350 2950 1500
Connection ~ 2550 1350
Wire Wire Line
	2550 1500 2550 1350
Wire Wire Line
	2400 1350 3600 1350
Wire Wire Line
	1000 1900 5600 1900
Wire Wire Line
	1000 1900 1000 2000
Wire Wire Line
	1850 1350 2000 1350
Wire Wire Line
	950  1350 1350 1350
Wire Wire Line
	9000 3800 9000 3900
Wire Wire Line
	9100 3800 9000 3800
Wire Wire Line
	8950 3700 9100 3700
Wire Wire Line
	8450 3550 8450 3700
Wire Wire Line
	8650 3600 9100 3600
Wire Wire Line
	8650 3200 8650 3600
Wire Wire Line
	8250 3200 8650 3200
Wire Wire Line
	8700 3500 9100 3500
Wire Wire Line
	8700 3100 8700 3500
Wire Wire Line
	8250 3100 8700 3100
Wire Wire Line
	8750 3400 9100 3400
Wire Wire Line
	8750 3000 8750 3400
Wire Wire Line
	8250 3000 8750 3000
Wire Wire Line
	8800 3300 9100 3300
Wire Wire Line
	8800 2900 8800 3300
Wire Wire Line
	8250 2900 8800 2900
Connection ~ 8900 3000
Wire Wire Line
	9100 2900 8900 2900
Connection ~ 8900 3100
Wire Wire Line
	9100 3000 8900 3000
Connection ~ 8900 3200
Wire Wire Line
	8900 3100 9100 3100
Wire Wire Line
	8900 2900 8900 3250
Wire Wire Line
	9100 3200 8900 3200
Wire Wire Line
	9100 2800 8250 2800
Wire Wire Line
	9100 2700 8250 2700
Wire Wire Line
	8250 2600 9100 2600
Wire Wire Line
	8450 2500 8450 2150
Wire Wire Line
	9100 2500 8450 2500
Wire Wire Line
	9100 2400 8950 2400
Wire Wire Line
	8700 2000 8700 1900
Wire Wire Line
	8200 2000 8200 2100
Wire Wire Line
	6250 3900 5800 3900
Wire Wire Line
	5500 3850 5500 3900
Wire Wire Line
	5500 3500 6250 3500
Wire Wire Line
	5150 4050 5150 4100
Wire Wire Line
	5150 3400 5150 3550
Wire Wire Line
	4650 3300 6250 3300
Wire Wire Line
	4650 3800 4650 3300
Wire Wire Line
	4850 3200 6250 3200
Wire Wire Line
	4850 3800 4850 3200
Wire Wire Line
	4550 3100 6250 3100
Wire Wire Line
	4550 3800 4550 3100
Wire Wire Line
	4450 3800 4450 3400
Connection ~ 2550 1900
Connection ~ 2950 1900
Wire Wire Line
	4000 1650 4000 1900
Connection ~ 3350 1900
Wire Wire Line
	4400 1350 4600 1350
Connection ~ 4000 1900
Wire Wire Line
	5100 1350 6200 1350
Connection ~ 5250 1350
Wire Wire Line
	5600 1900 5600 2000
Connection ~ 5250 1900
Wire Wire Line
	4450 3400 6250 3400
Wire Wire Line
	5500 3850 5700 3850
Wire Wire Line
	5700 3850 5700 3600
Wire Wire Line
	5700 3600 6250 3600
Wire Wire Line
	3400 5250 3400 5300
Wire Wire Line
	3400 5250 3600 5250
Wire Wire Line
	3400 4900 4250 4900
Wire Wire Line
	4000 3850 4000 3800
Wire Wire Line
	4000 3800 4350 3800
Wire Wire Line
	4250 4900 4250 5150
Wire Wire Line
	4350 5150 4350 5050
Wire Wire Line
	4350 5050 3600 5050
Wire Wire Line
	3600 5050 3600 5250
Wire Wire Line
	9450 1950 9450 2000
Wire Wire Line
	8950 2400 8950 1600
Wire Wire Line
	8950 1600 9450 1600
Wire Wire Line
	9100 2300 9100 1950
Wire Wire Line
	9100 1950 9450 1950
Wire Wire Line
	5800 3900 5800 4900
Wire Wire Line
	5800 4900 4550 4900
Wire Wire Line
	4550 4900 4550 5150
Wire Wire Line
	5750 5850 5750 5950
Connection ~ 5750 5900
Wire Wire Line
	5750 5500 5950 5500
Wire Wire Line
	5950 5500 5950 5650
Wire Wire Line
	5950 5650 6250 5650
Wire Wire Line
	5750 5850 5600 5850
Wire Wire Line
	5600 5850 5600 5350
Wire Wire Line
	5600 5350 6100 5350
Wire Wire Line
	6100 5350 6100 5550
Wire Wire Line
	6100 5550 6250 5550
Wire Wire Line
	6250 5950 5900 5950
Wire Wire Line
	5900 5950 5900 5900
Wire Wire Line
	5900 5900 5750 5900
Wire Wire Line
	6250 4200 6000 4200
Wire Wire Line
	6000 4200 6000 5250
Wire Wire Line
	6000 5250 5500 5250
Wire Wire Line
	5500 5250 5500 6050
Wire Wire Line
	5500 6050 6250 6050
Wire Wire Line
	6250 6150 5400 6150
Wire Wire Line
	5400 6150 5400 5150
Wire Wire Line
	5400 5150 5900 5150
Wire Wire Line
	5900 5150 5900 4100
Wire Wire Line
	5900 4100 6250 4100
Wire Wire Line
	8250 3500 8350 3500
Wire Wire Line
	8350 3500 8350 3550
Wire Wire Line
	8250 3600 8300 3600
Wire Wire Line
	8300 3600 8300 3650
Wire Wire Line
	8300 3650 8450 3650
Connection ~ 8450 3650
Wire Wire Line
	6450 1150 6450 1400
Wire Wire Line
	6450 1400 6650 1400
Wire Wire Line
	6200 1350 6200 1050
Wire Wire Line
	5750 5500 5750 5300
Wire Wire Line
	5750 5300 6200 5300
Wire Wire Line
	6200 5300 6200 5100
Wire Wire Line
	6200 5100 10200 5100
Wire Wire Line
	10200 5100 10200 1050
Wire Wire Line
	10200 1050 6700 1050
Wire Wire Line
	6450 1800 6450 2000
Wire Wire Line
	6250 2800 5700 2800
Wire Wire Line
	5100 2900 5100 2800
Wire Wire Line
	6250 2600 6050 2600
Wire Wire Line
	6050 2600 6050 2400
Wire Wire Line
	6050 2400 5750 2400
Wire Wire Line
	5750 2550 5950 2550
Wire Wire Line
	5950 2550 5950 2700
Wire Wire Line
	5950 2700 6250 2700
Wire Wire Line
	5250 2550 5250 2600
Wire Wire Line
	5250 2600 5150 2600
Wire Wire Line
	5250 2400 5150 2400
Wire Wire Line
	4750 2400 4650 2400
Wire Wire Line
	4650 2400 4650 2500
Wire Wire Line
	4750 2600 4650 2600
Wire Wire Line
	4650 2600 4650 2700
Wire Wire Line
	5600 3000 5500 3000
Wire Wire Line
	5100 3000 5050 3000
Wire Wire Line
	5050 3000 5050 3050
Wire Wire Line
	6250 2900 6100 2900
Wire Wire Line
	6100 2900 6100 3000
$EndSCHEMATC
