
// Concept tester for loading programs into the
// Gigatron TTL microcomputer
// Marcel van Kervinck / Chris Lord
// May 2018

// The object file is embedded (in PROGMEM) in GT1 format. It would be
// GREAT if we can find a way to receive the file over the Arduino's
// serial interface without adding upstream complexity. But as the
// Arduino's 2K of RAM can't buffer an entire file at once, some
// intelligence is needed there and we haven't found a good way yet.
// The Arduino doesn't implement any form of flow control on its
// USB/serial interface (RTS/CTS or XON/XOFF).

// This interface program can also receive data over the USB serial interface.
// Use the sendGt1.py Python program on the computer to send a file.
// The file must be in GT1 format (.gt1 extension)
// For example:
//   python sendGt1.py life3.gt1

// XXX Better check if Gigatron is running OK (judge signals better)
// XXX Try higher than 9600 transfer rates again
// XXX Wild idea: let the ROM communicate back by modulating vPulse
// XXX Hardware: Put reset line on the DB9 jack
// XXX Hardware: Put an output line on the DB9 jack
// XXX Embed a Gigatron Terminal program. Or better: GigaMon

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

// Keyboard
// Pin  4  PS/2 Data
// Pin  3  PS/2 Clock

#include <PS2Keyboard.h> // Install from the Arduino IDE's Library Manager

// Pins for PS/2 keyboard (Arduino Uno)
const int keyboardClockPin = 3;  // Pin 2 or 3 for IRQ
const int keyboardDataPin  = 4;  // Any available free pin

PS2Keyboard keyboard;
#define buttonRight  1
#define buttonLeft   2
#define buttonDown   4
#define buttonUp     8
#define buttonStart  16
#define buttonSelect 32
#define buttonB      64
#define buttonA      128

#define N 60 // Payload bytes per transmission frame

byte checksum; // Global is simplest

void setup()
{
  // Enable output pin (pins are set to input by default)
  PORTB |= 1<<PORTB5; // Send 1 when idle
  DDRB = 1<<PORTB5;

  // Open upstream communication
  Serial.begin(9600);
  doVersion();

  // In case we power on together with the Gigatron, this is a
  // good pause to wait for the video loop to have started
  delay(350);

  // PS/2 keyboard should be awake by now
  keyboard.begin(keyboardDataPin, keyboardClockPin);

  prompt();
}

void loop()
{
  static char line[20];
  static byte lineIndex = 0;

  if (Serial.available()) {
    byte next = Serial.read();
    if (lineIndex < sizeof line)
      line[lineIndex++] = next;
    if (next == '\n') {
      line[lineIndex-1] = '\0';
      doCommand(line);
      lineIndex = 0;
    }
  }

  if (keyboard.available()) {
    char c = keyboard.read();
    switch (c) {
      case PS2_PAGEDOWN:   sendController(~buttonSelect,2); break;
      case PS2_PAGEUP:     sendController(~buttonStart, 128+32); break;
      case PS2_TAB:        sendController(~buttonA,     2); break;
      case PS2_ESC:        sendController(~buttonB,     2); break;
      case PS2_LEFTARROW:  sendController(~buttonLeft,  2); break;
      case PS2_RIGHTARROW: sendController(~buttonRight, 2); break;
      case PS2_UPARROW:    sendController(~buttonUp,    2); break;
      case PS2_DOWNARROW:  sendController(~buttonDown,  2); break;
      case PS2_ENTER:      sendController('\n', 1);         break;
      case PS2_DELETE:     sendController(127, 1);          break;
      default:             sendController(c, 1);            break;
    }
    delay(50); // Allow Gigatron software to process key code
  }
}

void prompt()
{
  Serial.println(detectGigatron() ? ":Gigatron OK" : "!Gigatron offline");
  Serial.println("\nCmd?");
}

bool detectGigatron()
{
  unsigned long timeout = millis() + 85;
  long T[] = {0, 0, 0, 0};

  // Sample the sync signals coming out of the controller port
  while (millis() < timeout)
    T[(PINB >> PORTB3) & 3]++; // capture PORTB3 and PORTB4

  float S = T[0] + T[1] + T[2] + T[3];
  float vSync = (T[0] + T[1]) / ( 8 * S / 521); // Gigatron VGA
  float hSync = (T[0] + T[2]) / (96 * S / 800); // Default VGA

  return 0.95 <= vSync && vSync <= 1.20 && 0.95 <= hSync && hSync <= 1.05;
}

void doCommand(char line[])
{
  switch (toupper(line[0])) {
  case 'V': doVersion();                      break;
  case 'H': doHelp();                         break;
  case 'R': doReset();                        break;
  case 'L': doLoader();                       break;
  case 'P': doTransfer(gt1File);              break;
  case 'U': doTransfer(NULL);                 break;
  case 'W': sendController(~buttonUp,     2); break;
  case 'A': sendController(~buttonLeft,   2); break;
  case 'S': sendController(~buttonDown,   2); break;
  case 'D': sendController(~buttonRight,  2); break;
  case 'Z': sendController(~buttonA,      2); break;
  case 'X': sendController(~buttonB,      2); break;
  case 'Q': sendController(~buttonSelect, 2); break;
  case 'E': sendController(~buttonStart,  2); break;
  case 0: /* Empty line */                    break;
  default:  Serial.println("!Unknown command (type 'H' for help)");
  }
  prompt();
}

void doVersion()
{
  Serial.println(":Gigatron Interface Adapter [Arduino]\n"
                 ":Type 'H' for help");
}

void doHelp()
{
  Serial.println(":Commands are");
  Serial.println(": V        Show version");
  Serial.println(": H        Show this help");
  Serial.println(": R        Reset Gigatron");
  Serial.println(": L        Start Loader");
  Serial.println(": P        Transfer object file from PROGMEM");
  Serial.println(": U        Transfer object file from USB");
  Serial.println(": W/A/S/D  Up/left/down/right arrow");
  Serial.println(": Z/X      A/B button ");
  Serial.println(": Q/E      Select/start button");
}

void doReset()
{
  // Soft reset: hold start for >128 frames (>2.1 seconds)
  Serial.println(":Resetting Gigatron");
  Serial.flush();

  sendController(~buttonStart, 128+32);

  // Wait for main menu to be ready
  delay(1500);
}

void doLoader()
{
  // Navigate menu. 'Loader' is at the bottom
  Serial.println(":Starting Loader from menu");
  Serial.flush();
  for (int i=0; i<10; i++) {
    sendController(~buttonDown, 2);
    delay(50);
  }

  // Start 'Loader' application on Gigatron
  sendController(~buttonA, 2);

  // Wait for Loader to be running
  delay(1000);
}

// Because the Arduino doesn't have enough RAM to buffer
// a complete GT1 file, it processes these files segment
// by segment. Each segment is transmitted downstream in
// concatenated frames to the Gigatron. Between segments
// it is communicating upstream with the serial port.

void doTransfer(const byte *gt1)
{
  int nextByte;

  #define readNext() {\
    nextByte = gt1 ? pgm_read_byte(gt1++) : nextSerial();\
    if (nextByte < 0) return;\
  }

  #define ask(n)\
    if (!gt1) {\
      Serial.print(n);\
      Serial.println("?");\
    }

  byte segment[300] = {0}; // Multiple of N for padding

  ask(3);
  readNext();
  word address = nextByte;

  // Any number n of segments (n>0)
  do {
    // Segment start and length
    readNext();
    address = (address << 8) + nextByte;
    readNext();
    int len = nextByte ? nextByte : 256;

    ask(len);

    // Copy data into send buffer
    for (int i=0; i<len; i++) {
      readNext();
      segment[i] = nextByte;
    }

    // Check that segment doesn't cross the page boundary
    if ((address & 255) + len > 256) {
      Serial.println("!Data error (page overflow)");
      return;
    }

    // Send downstream
    Serial.print(":Loading ");
    Serial.print(len);
    Serial.print(" bytes at $");
    Serial.println(address, HEX);
    Serial.flush();

    sendGt1Segment(address, len, segment);

    // Signal that we're ready to receive more
    ask(3);
    readNext();
    address = nextByte;

  } while (address != 0);

  // Two bytes for start address
  readNext();
  address = nextByte;
  readNext();
  address = (address << 8) + nextByte;
  if (address != 0) {
    Serial.print(":Executing from $");
    Serial.println(address, HEX);
    Serial.flush();
    sendGt1Execute(address, segment+240);
  }
}

int nextSerial()
{
  unsigned long timeout = millis() + 5000;

  while (!Serial.available() && millis() < timeout)
    ;

  int nextByte = Serial.read();
  if (nextByte < 0)
    Serial.println("!Timeout error (no data)");

  return nextByte;
}

// Send a 1..256 byte code or data segment into the Gigatron by
// repacking it into Loader frames of max N=60 payload bytes each.
void sendGt1Segment(word address, int len, byte data[])
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
void sendGt1Execute(word address, byte data[])
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

void resetChecksum(byte n, word address, byte dummyData[])
{
  // Send one frame with false checksum to force
  // a checksum resync at the receiver
  checksum = 0;
  sendFrame(-1, n, address, dummyData);

  // Setup checksum properly
  checksum = 'g';
}

void sendFrame(byte firstByte, byte len, word address, byte message[])
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

