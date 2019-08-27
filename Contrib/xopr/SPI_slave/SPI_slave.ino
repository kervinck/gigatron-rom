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
  Serial.begin( 9600 );

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
