
// Concept tester for writing on screen
// Gigatron TTL microcomputer
// Marcel van Kervinck
// Apr 2018

// XXX Read real user input from serial port of Arduino
// XXX Write a character on screen instead of changing a pixel

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
}

byte terminal[] = {
 0x11, // 0200 LDWI
 0xe9,
 0x02,
 0x2b, // 0203 STW
 0x30, // 0203 'NextByteIn_32'
 0x11, // 0205 LDWI
 0x0c,
 0x5b,
 0x2b, // 0208 STW
 0x32, // 0208 'B'
 0x21, // 020a LDW
 0x32, // 020a 'B'
 0x2b, // 020c STW
 0x24,
 0x59, // 020e LDI
 0xcf,
 0x5e, // 0210 ST
 0x27,
 0x21, // 0212 LDW
 0x30, // 0212 'NextByteIn_32'
 0x2b, // 0214 STW
 0x22,
 0xb4, // 0216 SYS
 0xfe,
 0x59, // 0218 LDI
 0xdb,
 0x5e, // 021a ST
 0x27,
 0xb4, // 021c SYS
 0xfe,
 0x59, // 021e LDI
 0xeb,
 0x5e, // 0220 ST
 0x27,
 0xb4, // 0222 SYS
 0xfe,
 0x59, // 0224 LDI
 0xfb,
 0x5e, // 0226 ST
 0x27,
 0xb4, // 0228 SYS
 0xfe,
 0x59, // 022a LDI
 0x02,
 0x5e, // 022c ST
 0x27,
 0x21, // 022e LDW
 0x30, // 022e 'NextByteIn_32'
 0x2b, // 0230 STW
 0x22,
 0xb4, // 0232 SYS
 0xfe,
 0x59, // 0234 LDI
 0x06,
 0x5e, // 0236 ST
 0x27,
 0x21, // 0238 LDW
 0x30, // 0238 'NextByteIn_32'
 0x2b, // 023a STW
 0x22,
 0xb4, // 023c SYS
 0xfe,
 0x1a, // 023e LD
 0x27,
 0xe3, // 0240 ADDI
 0x04,
 0x5e, // 0242 ST
 0x27,
 0x8c, // 0244 XORI
 0xf2,
 0x35, // 0246 BCC
 0x72, // 0247 NE
 0x36,
 0x59, // 0249 LDI
 0xb9,
 0x5e, // 024b ST
 0x27,
 0xb4, // 024d SYS
 0xfe,
 0x90, // 024f BRA
 0x08,
};

void loop() {
  static byte payload[2*60];
  memcpy(payload, terminal, sizeof terminal);
  noInterrupts();
  for (;;) {
    // Send one frame with false checksum to force
    // a checksum resync at the receiver
    checksum = 0;
    sendFrame(-1, 60, 0x200, payload);

    // Setup checksum properly
    checksum = 'g';
    sendFrame('L', 60,                   0x200,      payload);
    sendFrame('L', sizeof terminal - 60, 0x200 + 60, payload + 60);

    // Force execution
    sendFrame('L', 0, 0x0200, payload + 60);
  }
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

