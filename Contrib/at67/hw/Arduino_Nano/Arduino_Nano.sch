EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:switches
LIBS:relays
LIBS:motors
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
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
LIBS:arduino
LIBS:Arduino_Nano-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Arduino Nano"
Date "2018-06-18"
Rev "0.1.1"
Comp "at67"
Comment1 "plus Controller passthrough."
Comment2 "Arduino Nano interface to Gigatron TTL Hardware, supports all Arduino interface features"
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L ARDUINO_NANO ARDUINO_NANO
U 1 1 5B267A19
P 3800 3100
F 0 "ARDUINO_NANO" H 3800 3975 70  0000 C CNN
F 1 " " V 3800 3100 70  0000 C CNN
F 2 "" H 3800 3100 60  0000 C CNN
F 3 "" H 3800 3100 60  0000 C CNN
	1    3800 3100
	1    0    0    -1  
$EndComp
Text Notes 3225 2350 0    49   ~ 0
J1-1
Text Notes 3225 2450 0    49   ~ 0
J1-2\n
Text Notes 3225 2550 0    49   ~ 0
J1-3
Text Notes 3225 2650 0    49   ~ 0
J1-4
Text Notes 3225 2750 0    49   ~ 0
J1-5
Text Notes 3225 2850 0    49   ~ 0
J1-6
Text Notes 3225 2950 0    49   ~ 0
J1-7
Text Notes 3225 3050 0    49   ~ 0
J1-8
Text Notes 3225 3150 0    49   ~ 0
J1-9
Text Notes 3225 3250 0    49   ~ 0
J1-10
Text Notes 3225 3350 0    49   ~ 0
J1-11
Text Notes 3225 3450 0    49   ~ 0
J1-12
Text Notes 3225 3550 0    49   ~ 0
J1-13
Text Notes 3225 3650 0    49   ~ 0
J1-14
Text Notes 3225 3750 0    49   ~ 0
J1-15
Text Notes 4175 2350 0    49   ~ 0
J2-1
Text Notes 4175 2450 0    49   ~ 0
J2-2\n
Text Notes 4175 2550 0    49   ~ 0
J2-3
Text Notes 4175 2650 0    49   ~ 0
J2-4
Text Notes 4175 2750 0    49   ~ 0
J2-5
Text Notes 4175 2850 0    49   ~ 0
J2-6
Text Notes 4175 2950 0    49   ~ 0
J2-7
Text Notes 4175 3050 0    49   ~ 0
J2-8
Text Notes 4175 3150 0    49   ~ 0
J2-9
Text Notes 4175 3250 0    49   ~ 0
J2-10
Text Notes 4175 3350 0    49   ~ 0
J2-11
Text Notes 4175 3450 0    49   ~ 0
J2-12
Text Notes 4175 3550 0    49   ~ 0
J2-13
Text Notes 4175 3650 0    49   ~ 0
J2-14
Text Notes 4175 3750 0    49   ~ 0
J2-15
$Comp
L DB9_Female J1
U 1 1 5B267EF4
P 6375 2775
F 0 "J1" H 6375 3325 50  0000 C CNN
F 1 "DB9_Female" H 6375 2200 50  0000 C CNN
F 2 "" H 6375 2775 50  0001 C CNN
F 3 "" H 6375 2775 50  0001 C CNN
	1    6375 2775
	1    0    0    -1  
$EndComp
$Comp
L DB9_Male J2
U 1 1 5B2680D7
P 6375 4525
F 0 "J2" H 6375 5075 50  0000 C CNN
F 1 "DB9_Male" H 6375 3950 50  0000 C CNN
F 2 "" H 6375 4525 50  0001 C CNN
F 3 "" H 6375 4525 50  0001 C CNN
	1    6375 4525
	1    0    0    -1  
$EndComp
Wire Wire Line
	6350 2575 5450 2575
Wire Wire Line
	5450 2575 5450 3750
Wire Wire Line
	5450 3750 4400 3750
Wire Wire Line
	6350 2775 5550 2775
Wire Wire Line
	5550 2775 5550 4025
Wire Wire Line
	5550 4025 5550 4525
Wire Wire Line
	3200 4025 5550 4025
Wire Wire Line
	3200 4025 3200 3750
Wire Wire Line
	6350 2975 5650 2975
Wire Wire Line
	5650 2975 5650 4125
Wire Wire Line
	5650 4125 5650 4325
Wire Wire Line
	3100 4125 5650 4125
Wire Wire Line
	3100 4125 3100 3650
Wire Wire Line
	3100 3650 3200 3650
Wire Wire Line
	5650 4325 6350 4325
Connection ~ 5650 4125
Wire Wire Line
	5550 4525 6350 4525
Connection ~ 5550 4025
Wire Wire Line
	6075 4725 3000 4725
Wire Wire Line
	3000 4725 3000 3550
Wire Wire Line
	3000 3550 3200 3550
Text Notes 6575 2825 0    60   Italic 12
Plugs into Gigatron TTL male DB9 connector.
Text Notes 6575 4525 0    60   Italic 12
Controller female DB9 plugs into this connector.
Wire Wire Line
	4850 4825 6075 4825
Wire Wire Line
	4850 2650 4850 4825
Wire Wire Line
	4850 2650 4400 2650
Wire Wire Line
	4950 4425 6350 4425
Wire Wire Line
	4950 4425 4950 2450
Wire Wire Line
	4950 2450 4400 2450
$EndSCHEMATC
