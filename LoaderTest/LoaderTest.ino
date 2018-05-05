
// Concept tester for loading programs into the
// Gigatron TTL microcomputer
// Marcel van Kervinck
// Jan 2018

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

const byte blinkyGt1[] PROGMEM = {
  0x7f, 0x00, 0x0b, // Segment 7f00: 11 bytes
  0x11, 0x50, 0x44, // 7f00 LDWI $4450  ; Load address of center of screen
  0x2b, 0x30,       // 7f03 STW  'p'    ; Store in variable 'p' (at $0030)
  0xf0, 0x30,       // 7f05 POKE 'p'    ; Write low byte of accumulator there
  0xe3, 0x01,       // 7f07 ADDI 1      ; Increment accumulator
  0x90, 0x03,       // 7f09 BRA  $7f05  ; Loop forever
  0x00,             // No more segments
  0x7f, 0x00        // Start address
};

void setup() {
  // Enable output pin (pins are set to input by default)
  PORTB |= 1<<PORTB5; // Send 1 when idle
  DDRB = 1<<PORTB5;

  // Open upstream communication
  Serial.begin(57600);
  Serial.println("*** Arduino Gigatron Hub");
}

void loop()
{
  Serial.println("Reset Gigatron");

  // In case we power on together with the Gigatron, this is a
  // good pause to wait for the video loop to have started
  delay(350);

  // Soft reset: hold start for >128 frames (>2.1 seconds)
  sendController(~buttonStart, 128+32);

  // Wait for main menu to be ready
  delay(1500);

  // Navigate menu. 'Loader' is at the bottom
  Serial.println("Start Loader from menu");
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

  // Send Blinky as pseudo gt1 file
  Serial.println("Load gt1 file");
  sendGt1File(blinkyGt1, sizeof blinkyGt1);

  // Wait
  for (int i=10; i>0; i--) {
    Serial.println(i);
    delay(1000);
  }
}

// Because the Arduino doesn't have enough RAM to buffer
// a complete GT1 file, it processes these files segment
// by segment. Each segment is transmitted downstream in
// concatenated frames to the Gigatron. Between segments
// it is communicating upstream with the serial port.

void sendGt1File(const byte *gt1, unsigned gt1Size)
{
  #define byte_gt1(x) (pgm_read_byte_near(gt1 + (x)))
  #define adr_gt1(x) (((word)byte_gt1(x) << 8) | byte_gt1((x) + 1))

  // Any number n > 0 of segments
  word address;
  int seg_start = 0;
  byte segment[256];
  do {
    address = adr_gt1(seg_start);
    int len = byte_gt1(seg_start + 2);
    if (!len)
      len = 256;

    // Check that segment doesn't cross page boundary
    if ((address & 255) + len > 256) {
      Serial.println("Error: invalid stream");
      return;
    }

    // Read data out of PROGMEM
    for (int i = 0; i < len; ++i) {
      segment[i] = byte_gt1(seg_start + 3 + i);
    }

    // Send downstream
    Serial.print("> Load ");
    Serial.print(len);
    Serial.println(" bytes");
    int page_size = (address | 0xff) - address;
    sendGt1Segment(address, page_size, len, segment);

    // Adjust pointer to the start of the next segment
    seg_start += len + 3;
  } while (byte_gt1(seg_start) != 0);

  seg_start++; // Skip over terminating byte

  // Read start address
  address = adr_gt1(seg_start);
  if (address != 0) {
    Serial.print("> Start $");
    Serial.println(address, HEX);
    sendGt1Execute(address, NULL);
  }
}

// Send a 1..256 byte code or data segment into the Gigatron by
// repacking it into Loader frames of max N=60 payload bytes each.
void sendGt1Segment(word address, int page_size, int len, const byte data[])
{
  noInterrupts();
  byte n = min(N, len);
  byte pn = min(N, page_size);
  resetChecksum(pn, n, address, data);

  // Send segment data
  for (int i=0; i<page_size; i+=pn) {
    pn = min(N, page_size-i);
    n = (i >= len) ? 0 : min(N, len-i);
    sendFrame('L', pn, n, address+i, data+i);
  }
  interrupts();
}

// Send execute command
void sendGt1Execute(word address, const byte data[])
{
  noInterrupts();
  resetChecksum(0, 0, address, data);
  sendFrame('L', 0, 0, address, data);
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

void resetChecksum(byte pn, byte n, word address, const byte *data)
{
  // Send one frame with false checksum to force
  // a checksum resync at the receiver
  checksum = 0;
  sendFrame(-1, pn, n, address, data);

  // Setup checksum properly
  checksum = 'g';
}

void sendFrame(byte firstByte, byte page_len, byte data_len, word address, const byte *message)
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
  sendBits(page_len, 6);       // Length 0, 1..60
  sendBits(address&255, 8);    // Low address bits
  sendBits(address>>8, 8);     // High address bits
  for (byte i=0; i<N; i++)     // Payload bytes
    sendBits(i < data_len ? message[i] : 0, 8);
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

