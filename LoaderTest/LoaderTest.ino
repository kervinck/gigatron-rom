
// Concept tester for loading programs into the
// Gigatron TTL microcomputer
// Marcel van Kervinck
// Jan 2018

// The object file is embedded (in PROGMEM) in GT1 format. It would be
// GREAT if we can find a way to receive the file over the Arduino's
// serial interface without adding upstream complexity. But as the
// Arduino's 2K of RAM can't buffer an entire file at once, some
// intelligence is needed there and we haven't found a good way yet.

const byte gt1File[] PROGMEM = {
  //#include "Blinky.h" // Blink pixel in middle of screen
  #include "Lines.h"    // Draw randomized lines
};

// Arduino AVR    Gigatron Schematic Controller PCB
// Uno     Name   OUT bit            CD4021     74HC595 (U39)
// ------- ------ -------- --------- ---------- ----------------
// Pin 13  PORTB5 None     SER_DATA  11 SER INP 14 SER
// Pin 12  PORTB4 7 vSync  SER_LATCH  0 PAR/SER None
// Pin 11  PORTB3 6 hSync  SER_PULSE 10 CLOCK   11 SRCLK 12 RCLK

#define buttonDown  4
#define buttonStart 16
#define buttonA     128

#define N           60 // Payload bytes per transmission frame

byte checksum; // Global is simplest

void setup() {
  // Enable output pin (pins are set to input by default)
  PORTB |= 1<<PORTB5; // Send 1 when idle
  DDRB = 1<<PORTB5;

  // Open upstream communication
  Serial.begin(57600);
  doVersion();

  // In case we power on together with the Gigatron, this is a
  // good pause to wait for the video loop to have started
  delay(350);
}

void loop()
{
  doLoad();
  countdown(10);
}

void doVersion()
{
  Serial.println("*** Arduino Gigatron Extender");
}

void doReset()
{
  // Soft reset: hold start for >128 frames (>2.1 seconds)
  Serial.println("Reset Gigatron");
  sendController(~buttonStart, 128+32);

  // Wait for main menu to be ready
  delay(1500);
}

void doLoad()
{
  doReset();

  // Navigate menu. 'Loader' is at the bottom
  Serial.println("Start Loader from menu");
  for (int i=0; i<10; i++) {
    sendController(~buttonDown, 2);
    delay(50);
  }

  // Start 'Loader' application on Gigatron
  sendController(~buttonA, 2);

  // Wait for Loader to be running
  delay(1000);

  // Send GT1 file
  Serial.println("Send GT1 file");
  sendGt1File(gt1File);
}

// Countdown from n to 1
void countdown(int n)
{
  for (int i=n; i>0; i--) {
    Serial.print(" ");
    Serial.print(i);
    delay(1000);
  }
  Serial.print("\n");
}

// Because the Arduino doesn't have enough RAM to buffer
// a complete GT1 file, it processes these files segment
// by segment. Each segment is transmitted downstream in
// concatenated frames to the Gigatron. Between segments
// it is communicating upstream with the serial port.

void sendGt1File(const byte *gt1)
{
  #define readNext() pgm_read_byte(gt1++)

  byte segment[300] = {0};
  word address = readNext();

  // Any number n of segments (n>0)
  do {
    // Segment start and length
    address = (address << 8) + readNext();
    int len = readNext();
    if (!len)
      len = 256;

    // Copy data into send buffer
    for (int i=0; i<len; i++)
      segment[i] = readNext();

    // Check that segment doesn't cross the page boundary
    if ((address & 255) + len > 256) {
      Serial.println("? Invalid GT1 data");
      return;
    }

    // Send downstream
    Serial.print("> Load $");
    Serial.print(address, HEX);
    Serial.print(" (");
    Serial.print(len);
    Serial.println(" bytes)");
    sendGt1Segment(address, len, segment);

    address = readNext();
  } while (address != 0);

  // Read start address
  address = readNext();
  address = (address << 8) + readNext();
  if (address != 0) {
    Serial.print("> Start $");
    Serial.println(address, HEX);
    sendGt1Execute(address, NULL);
  }
}

// Send a 1..256 byte code or data segment into the Gigatron by
// repacking it into Loader frames of max N=60 payload bytes each.
void sendGt1Segment(word address, int len, const byte data[])
{
  noInterrupts();
  byte n = min(N, len);
  resetChecksum(n, address, data);

  // Send segment data
  for (int i=0; i<len; i+=n) {
    n = min(N, len-i);
    sendFrame('L', n, address+i, data+i);
  }
  interrupts();
}

// Send execute command
void sendGt1Execute(word address, const byte data[])
{
  noInterrupts();
  resetChecksum(0, address, data);
  sendFrame('L', 0, address, data);
  interrupts();
}

// Pretend to be a game controller
// Send the same byte a few frames like a human user
void sendController(byte value, int n)
{
  noInterrupts();

  // Send controller code for n frames
  // E.g. 4 frames = 3/60s = ~50 ms
  for (int i=0; i<n; i++) {
    sendFirst(value, 8);
    PORTB |= 1<<PORTB5; // Send 1 when idle
  }

  interrupts(); // So delay() can work again
}

void resetChecksum(byte n, word address, const byte *data)
{
  // Send one frame with false checksum to force
  // a checksum resync at the receiver
  checksum = 0;
  sendFrame(-1, n, address, data);

  // Setup checksum properly
  checksum = 'g';
}

void sendFrame(byte firstByte, byte len, word address, const byte *message)
{
  // Send one frame of data
  //
  // A frame has 65*8-2=518 bits, including protocol byte and checksum.
  // The reasons for the two "missing" bits are:
  // 1. From start of vertical pulse, there are 35 scanlines remaining
  // in vertical blank. But we also need the payload bytes to align
  // with scanlines where the interpreter runs, so the Gigatron doesn't
  // have to shift everything it receives by 1 bit.
  // 2. There is a 1 bit latency inside the 74HCT595 for the data bit,
  // but (obviously) not for the sync signals.
  // All together, we drop 2 bits from the 2nd byte in a frame. This achieves
  // byte alignment for the Gigatron at visible scanline 3, 11, 19, ... etc.

  sendFirst(firstByte, 8);     // Protocol byte
  checksum += firstByte << 6;  // Keep Loader.gcl dumb
  sendBits(len, 6);            // Length 0, 1..60
  sendBits(address&255, 8);    // Low address bits
  sendBits(address>>8, 8);     // High address bits
  for (byte i=0; i<N; i++)     // Payload bytes
    sendBits(message[i], 8);
  byte lastByte = -checksum;   // Checksum must come out as 0
  sendBits(lastByte, 8);
  checksum = lastByte;         // Concatenate checksums
  PORTB |= 1<<PORTB5;          // Send 1 when idle
}

void sendFirst(byte value, byte n)
{
  // Wait vertical sync NEGATIVE edge to sync with loader
  while (~PINB & (1<<PORTB4)) // Ensure vSync is HIGH first
    ;
  while (PINB & (1<<PORTB4)) // Then wait for vSync to drop
    ;
  sendBits(value, n);
}

// Send n bits, highest first
void sendBits(byte value, byte n)
{
  for (byte bit=1<<(n-1); bit; bit>>=1) {
    // Send next bit
    if (value & bit)
      PORTB |= 1<<PORTB5;
    else
      PORTB &= ~(1<<PORTB5);

    // Wait for bit transfer at horizontal sync POSITIVE edge.
    // This timing is tight for the first bit of the first byte and
    // the reason that interrupts must be disabled on the Arduino.
    while (PINB & (1<<PORTB3))  // Ensure hSync is LOW first
      ;
    while (~PINB & (1<<PORTB3)) // Then wait for hSync to rise
      ;
  }
  checksum += value;
}

