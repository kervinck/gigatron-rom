
// Concept tester for writing on screen
// Gigatron TTL microcomputer
// Marcel van Kervinck
// Apr 2018

// XXX Read real user input from serial port of Arduino

// Arduino AVR    Gigatron Schematic Controller PCB
// Uno     Name   OUT bit            CD4021     74HC595 (U39)
// ------- ------ -------- --------- ---------- ----------------
// Pin 13  PORTB5 None     SER_DATA  11 SER INP 14 SER
// Pin 12  PORTB4 7 vSync  SER_LATCH  0 PAR/SER None
// Pin 11  PORTB3 6 hSync  SER_PULSE 10 CLOCK   11 SRCLK 12 RCLK

#define buttonDown  4
#define buttonStart 16
#define buttonA     128

byte checksum; // Global is simplest

byte terminalApp[] = {
  // 0x0200
  // Compiled from Terminal/Main.gcl
  0xcd, 0x4a, 0x2b, 0x30, 0xe6, 0x52, 0x35, 0x53, 0x10, 0xe3, 0x32,
  0x2b, 0x32, 0x11, 0x00, 0x07, 0x90, 0x15, 0x2b, 0x32, 0x11, 0x00,
  0x08, 0x2b, 0x34, 0x21, 0x32, 0xe9, 0xe9, 0x99, 0x32, 0x99, 0x34,
  0x2b, 0x34, 0x59, 0x20, 0x5e, 0x24, 0x59, 0x0f, 0x5e, 0x25, 0x21,
  0x36, 0x2b, 0x28, 0x11, 0xe1, 0x04, 0x2b, 0x22, 0x59, 0xfb, 0x2b,
  0x32, 0x21, 0x34, 0x7f, 0x00, 0x93, 0x34, 0x5e, 0x26, 0xb4, 0xcb,
  0x93, 0x28, 0x93, 0x32, 0x21, 0x32, 0x35, 0x72, 0x36, 0xff, 0x2b,
  0x38, 0x11, 0x00, 0x78, 0x2b, 0x36, 0xcd, 0x9f, 0x59, 0x00, 0x5e,
  0x36, 0x11, 0x00, 0x08, 0x99, 0x36, 0x35, 0x53, 0x62, 0x11, 0x00,
  0x08, 0x2b, 0x36, 0x21, 0x36, 0x2b, 0x28, 0x59, 0x20, 0x5e, 0x24,
  0x5e, 0x25, 0x11, 0xe1, 0x04, 0x2b, 0x22, 0xb4, 0xcb, 0x93, 0x28,
  0x1a, 0x28, 0x8c, 0xa0, 0x35, 0x72, 0x73, 0x11, 0xee, 0x01, 0x2b,
  0x3a, 0x21, 0x3a, 0xad, 0xe6, 0x78, 0x35, 0x53, 0x8f, 0x8c, 0x80,
  0x90, 0x91, 0x8c, 0x08, 0xf0, 0x3a, 0x21, 0x3a, 0xe6, 0x02, 0x2b,
  0x3a, 0x8c, 0xfe, 0x35, 0x72, 0x83, 0xff, 0x2b, 0x3c, 0xcf, 0x18,
  0x59, 0x7f, 0xcf, 0x38, 0x1a, 0x0f, 0x2b, 0x3e, 0x1a, 0x0f, 0x2b,
  0x30, 0xfc, 0x3e, 0x35, 0x72, 0xb8, 0x21, 0x30, 0x90, 0xa9, 0x21,
  0x30, 0x8c, 0x0a, 0x35, 0x72, 0xc7, 0x59, 0x20, 0xcf, 0x38, 0xcf,
  0x3c, 0x90, 0xa3, 0x21, 0x30, 0xe6, 0x20, 0x35, 0x50, 0xa3, 0xe6,
  0x5f, 0x35, 0x53, 0xa3, 0x21, 0x30, 0xcf, 0x38, 0x21, 0x36, 0xe3,
  0x06, 0x2b, 0x36, 0x82, 0xff, 0xe6, 0x9b, 0x35, 0x50, 0xe6, 0xcf,
  0x3c, 0x90, 0xa3,
};

void setup() {
  // Enable output pin (pins are set to input by default)
  PORTB |= 1<<PORTB5; // Send 1 when idle
  DDRB = 1<<PORTB5;

  // In case we power on together with the Gigatron, this is a
  // good pause to wait for the video loop to have started
  delay(350);

  // Soft reset: hold start for >128 frames (>2.1 seconds)
  sendController(~buttonStart, 128+32);

  // Wait for main menu to be ready
  delay(1500);

  // Navigate menu. 'Loader' is at the bottom
  for (int i=0; i<10; i++) {
    sendController(~buttonDown, 4);
    delay(50);
  }

  // Start 'Loader' application on Gigatron
  for (int i=0; i<3; i++) {
    sendController(~buttonA, 4);
    delay(100);
  }

  // Wait for Loader to be running
  delay(500);

  // Setup checksum properly
  checksum = 'g';

  byte message[60];
  memset(message, 0, sizeof message);

  noInterrupts();

  // Upload Terminal application in chunks
  for (int ix=0; ix<sizeof terminalApp; ix+=sizeof message) {
    int chunkSize = min(sizeof message, sizeof terminalApp - ix);
    memcpy(message, terminalApp+ix, chunkSize);
    sendFrame('L', chunkSize, 0x200+ix, message);
  }

  // Start execution of Terminal application on Gigatron
  sendFrame('L', 0, 0x0200, message);

  interrupts(); // So delay() can work again

  delay(100);
}

void loop() {

  // Now send key strokes, simulating a user
  static int n = 1;
  char text[200];
  sprintf(text, " *** Test %d ***\n\n"
                "Hello Gigatron!\n"
                "This is Arduino Uno pretending to be an ASCII keyboard. Hope you like it!\n"
                "The quick brown fox jumps over the lazy dog.\n\n", n++);
  for (int i=0; text[i]; i++) {
    sendController(text[i], 2);
    delay(40);
  }
  delay(500);
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

void sendFrame(byte firstByte, byte len, unsigned address, byte message[60])
{
  // Send one frame of data
  //
  // A frame has 65*8-2=518 bits, including protocol byte and checksum.
  // The reasons for the two "missing" bits are:
  // 1. From start of vertical pulse, there are 35 scanlines remaining
  // in vertical blank. But we also need the buffer bytes to align
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
  for (byte i=0; i<60; i++)    // Payload bytes
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

