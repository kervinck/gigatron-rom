// NOTE: make sure you disable the Arduino reset when using it as a serial terminal
// run: `systemctl enable serial-getty@ttyACM0.service` (or the appropriate Arduino serial device 
// edit: `/etc/systemd/system/getty.target.wants/serial-getty@ttyACM0.service`
// and change
// ExecStart=...... $TERM
// into:
// ExecStart=...... vt52
// then run: `systemctl daemon-reload`
// and: `systemctl restart serial-getty@ttyACM0.service`

#define SERIAL_BUFFER_SIZE 1024

//from https://forum.arduino.cc/index.php?topic=52111.0
#include "pins_arduino.h"

volatile char serialByte = 0;
volatile char spiByte = 0;

void setup (void)
{
  // Note: The Atmega16u2 uses U2X mode, where 300 baud isn't possible
  // CH340G has a minimum of 2400 baud
  // https://github.com/arduino/ArduinoCore-avr/issues/265
  // Fetch updated firmware from: https://github.com/facchinm/Arduino_avrusb_firmware/tree/master/arduino-usbserial
  // https://www.arduino.cc/en/Hacking/DFUProgramming8U2
  // see https://forum.arduino.cc/index.php?topic=385567.0
  Serial.begin( 300 );

  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);

  // turn on SPI in slave mode
  SPCR |= _BV(SPE);

  // turn on interrupts
  SPCR |= _BV(SPIE);
}

// SPI interrupt routine
ISR (SPI_STC_vect)
{
  spiByte = SPDR;
  SPDR = serialByte;
  serialByte = 0;
}

// main loop - wait for flag set in interrupt routine
void loop (void)
{
  if ( spiByte && spiByte > 0 )
  {
    Serial.print( spiByte );
    spiByte = 0;
  }
  if ( !serialByte && Serial.available() )
    serialByte = Serial.read();  
}
