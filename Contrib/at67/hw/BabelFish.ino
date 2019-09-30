
// Concept tester for interfacing with the
// Gigatron TTL microcomputer using a microcontroller
// hooked to the input port (J4).

// This sketch serves several purpuses:
// 1. Transfer a GT1 file into the Gigatron for execution.
//    This can be done with two methods: over USB or from PROGMEM
//    Each has their advantages and disadvantages:
//    a. USB can accept a regular GT1 file but needs a (Python)
//       program on the PC/laptop as sender (sendGt1.py).
//    b. PROGMEM only requires the Arduino IDE on the PC/laptop,
//       but needs the GT1 file as a hexdump (C notation).
// 2. Hookup a PS/2 keyboard for typing on the Gigatron
// 3. Controlling the Gigatron over USB from a PC/laptop
// Not every microcontroller supports all functions.

// Select 1 of the platforms:
#define ArduinoUno   0 // Default
#define ArduinoNano  1
#define ArduinoMicro 0
#define ATtiny85     0

// Select a built-in GT1 image:
const byte gt1File[] PROGMEM = {
  //#include "Blinky.h" // Blink pixel in middle of screen
#include "Lines.h"  // Draw randomized lines (at67)
};

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

// Todo/idea list:
// XXX Wild idea: let the ROM communicate back by modulating vPulse
// XXX Hardware: Put reset line on the DB9 jack
// XXX Hardware: Put an output line on the DB9 jack
// XXX Keyboard: Map Ctrl-Alt-Del to Gigatron reset (instead of PageUp)
// XXX Keyboard: Map Enter to both newline AND buttonA
// XXX Keyboard: Delete = buttonA (same code 0x7f). Change delete code?
// XXX Keyboard: Is it possible to mimic key hold-down properly???
// XXX Timeouts/reset in case of hanging (dead man switch function?)
// XXX Embed a Gigatron Terminal program. Or better: GigaMon

/*----------------------------------------------------------------------+
  |      Arduino Uno config                                              |
  +----------------------------------------------------------------------*/

// Arduino AVR    Gigatron Schematic Controller PCB              Gigatron
// Uno     Name   OUT bit            CD4021     74HC595 (U39)    DB9 (J4)
// ------- ------ -------- --------- ---------- ---------------- --------
// Pin 13  PORTB5 None     SER_DATA  11 SER INP 14 SER           2
// Pin 12  PORTB4 7 vSync  SER_LATCH  0 PAR/SER None             3
// Pin 11  PORTB3 6 hSync  SER_PULSE 10 CLOCK   11 SRCLK 12 RCLK 4

#if ArduinoUno
#define version "ArduinoUno"

// Pins for Gigatron
#define SER_DATA  PB5
#define SER_LATCH PB4
#define SER_PULSE PB3

// Pins for PS/2 keyboard (Arduino Uno)
#define keyboardClockPin PB3 // Pin 2 or 3 for IRQ
#define keyboardDataPin  PB4 // Any available free pin

// Link to PC/laptop
#define hasSerial 1

// Controller pass through
#define hasController 0
#endif

/*----------------------------------------------------------------------+
  |      Arduino Nano config                                             |
  +----------------------------------------------------------------------*/

// Arduino   AVR    Gigatron Schematic Controller PCB              Gigatron Controller
// Nano      Name   OUT bit            CD4021     74HC595 (U39)    DB9 (J4) DB9
// -------   ------ -------- --------- ---------- ---------------- -------- -------
// Pin J2-15 PORTB5 None     SER_DATA  11 SER INP 14 SER           2        None
// Pin J1-15 PORTB4 7 vSync  SER_LATCH  0 PAR/SER None             3        3
// Pin J1-14 PORTB3 6 hSync  SER_PULSE 10 CLOCK   11 SRCLK 12 RCLK 4        4
// Pin J1-13 PORTB2 None     None      None       None             None     2

#if ArduinoNano
#define version "ArduinoNano"

// Pins for Gigatron
#define SER_DATA  PB5
#define SER_LATCH PB4
#define SER_PULSE PB3

// Pins for Controller
#define JOY_DATA PB2

// Pins for PS/2 keyboard (Arduino Nano)
#define keyboardClockPin PB3 // Pin 2 or 3 for IRQ
#define keyboardDataPin  PB4 // Any available free pin

// Link to PC/laptop
#define hasSerial 1

// Controller pass through
#define hasController 1
#endif

/*----------------------------------------------------------------------+
  |      Arduino Micro config                                            |
  +----------------------------------------------------------------------*/

// See also: https://forum.gigatron.io/viewtopic.php?f=4&t=33

// Arduino AVR    Gigatron Schematic Controller PCB              Gigatron
// Micro   Name   OUT bit            CD4021     74HC595 (U39)    DB9 (J4)
// ------- ------ -------- --------- ---------- ---------------- --------
// SCLK    PORTB1 None     SER_DATA  11 SER INP 14 SER           2
// MISO    PORTB3 7 vSync  SER_LATCH  0 PAR/SER None             3
// MOSI    PORTB2 6 hSync  SER_PULSE 10 CLOCK   11 SRCLK 12 RCLK 4

// --------------------+
//               Reset |
// Arduino       +---+ |
//  Micro        | O | |
//               +---+ |
//                     |
//               2 4 6 |
//        ICSP-> o o o |
//        Port  .o o o |
//               1 3 5 |
// --------------------+

#if ArduinoMicro
#define version "ArduinoMicro"

// Pins for Gigatron
#define SER_DATA  PB1
#define SER_LATCH PB3
#define SER_PULSE PB2

// Pins for PS/2 keyboard (XXX These are still for Arduino Uno)
#define keyboardClockPin PB3 // Pin 2 or 3 for IRQ
#define keyboardDataPin  PB4 // Any available free pin

// Link to PC/laptop
#define hasSerial 1

// Controller pass through
#define hasController 0
#endif

/*----------------------------------------------------------------------+
  |      ATtiny85 config                                                 |
  +----------------------------------------------------------------------*/

// Used settings in Arduino IDE 1.8.1:
//   Board:       ATtiny
//   Processor:   ATtiny85
//   Clock:       8 MHz (internal) --> Use "Burn Bootloader" to configure this
//   Programmer:  Arduino as ISP
// See also:
//   https://create.arduino.cc/projecthub/arjun/programming-attiny85-with-arduino-uno-afb829

//                       +------+
//              ~RESET --|1.   8|-- Vcc
// PS/2 data       PB3 --|2    7|-- PB2  Serial data out
// PS/2 clock      PB4 --|3    6|-- PB1  Serial latch in
//                 GND --|4    5|-- PB0  Serial pulse in
//                       +------+
//                       ATtiny85

#if ATtiny85
#define version "ATtiny85"

// Pins for Gigatron
#define SER_DATA  PB2
#define SER_LATCH PB1
#define SER_PULSE PB0

// Pins for PS/2 keyboard
#define keyboardClockPin PB4
#define keyboardDataPin  PB3

// Link to PC/laptop
#define hasSerial 0

// Controller pass through
#define hasController 0

// PS2Keyboard.h uses attachInterrupt() which doesn't work on the ATtiny85.
// Workaround as follows:
void ps2interrupt(void); // As provided by PS2Keyboard
ISR(PCINT0_vect) {
  if (~PINB & (1 << keyboardClockPin)) // FALLING edge of PS/2 clock
    ps2interrupt();
}
#endif

/*----------------------------------------------------------------------+
  |      End config section                                              |
  +----------------------------------------------------------------------*/

/*
    Loader protocol
*/

#define N 60 // Payload bytes per transmission frame
byte checksum; // Global is simplest

/*
    PS/2 keyboard hookup to Arduino
*/

#include <PS2Keyboard.h> // Install from the Arduino IDE's Library Manager

PS2Keyboard keyboard;

const byte terminalGt1[] PROGMEM = {
#include "WozMon.h" // Monitor program ported from Apple-1
};

/*
    Game controller button mapping
*/

#define buttonRight  1
#define buttonLeft   2
#define buttonDown   4
#define buttonUp     8
#define buttonStart  16
#define buttonSelect 32
#define buttonB      64
#define buttonA      128
// Note: The kit's controller gives inverted signals.

/*
   Emulator control
*/

#define EMU_PS2_LEFT    1
#define EMU_PS2_RIGHT   2
#define EMU_PS2_UP      3
#define EMU_PS2_DOWN    4
#define EMU_PS2_START   7
#define EMU_PS2_SELECT  8
#define EMU_PS2_INPUT_A 9
#define EMU_PS2_INPUT_B 27
#define EMU_PS2_CR      13
#define EMU_PS2_DEL     127

#define EMU_PS2_ENABLE  5
#define EMU_PS2_DISABLE 6

bool emulatorControl = false;

/*
    Setup runs once when the Arduino wakes up
*/
void setup()
{
  // Enable output pin (pins are set to input by default)
  PORTB |= 1 << SER_DATA; // Send 1 when idle
  DDRB = 1 << SER_DATA;

  // Open upstream communication
#if hasSerial
  Serial.begin(115200);
#endif
  doVersion();

  // In case we power on together with the Gigatron, this is a
  // good pause to wait for the video loop to have started
  delay(350);

  // PS/2 keyboard should be awake by now
#if !ATtiny85
  keyboard.begin(keyboardDataPin, keyboardClockPin);
#else
  keyboard.begin(keyboardDataPin, 255);
  GIMSK |= 1 << PCIE;           // Pin change interrupt enable
  PCMSK |= 1 << keyboardClockPin; // Pin change mask
#endif

  prompt();
}

/*
    Loop runs repeatedly
*/
void loop()
{
  // Controller pass through
#if hasController
  ((PINB >> JOY_DATA) & 1) ? PORTB |= 1 << SER_DATA : PORTB &= ~(1 << SER_DATA);
#endif

#if hasSerial
  static char line[20];
  static byte lineIndex = 0;

  if (Serial.available()) {
    byte next = Serial.read();
    if (lineIndex < sizeof line)
      line[lineIndex++] = next;
    if (next == '\n') {
      line[lineIndex - 1] = '\0';
      (emulatorControl) ? doEmulator(line) : doCommand(line);
      lineIndex = 0;
    }
  }
#endif

  if (keyboard.available()) {
    char c = keyboard.read();
    switch (c) {
      // XXX These mappings are for testing purposes only
      case PS2_PAGEDOWN:   sendController(~buttonSelect, 1); break;
      case PS2_PAGEUP:     sendController(~buttonStart,  128 + 32); break; // XXX Change to Ctrl-Alt-Del
      case PS2_TAB:        sendController((byte)~buttonA, 1); break;
#if !ATtiny85
      case PS2_ESC:      sendController(~buttonB,      1); break;
#else
      case PS2_ESC:      doTransfer(terminalGt1);          break; // XXX HACK Find some proper short-cut. Ctrl-T?
#endif
      case PS2_LEFTARROW:  sendController(~buttonLeft,   2); break;
      case PS2_RIGHTARROW: sendController(~buttonRight,  2); break;
      case PS2_UPARROW:    sendController(~buttonUp,     2); break;
      case PS2_DOWNARROW:  sendController(~buttonDown,   2); break;
      case PS2_ENTER:      sendController('\n',          1); break;
      case PS2_DELETE:     sendController(127,           1); break;
      default:             sendController(c,             1); break;
    }
    delay(50); // Allow Gigatron software to process key code
  }
}

void prompt()
{
#if hasSerial
  Serial.println(detectGigatron() ? ":Gigatron OK" : "!Gigatron offline");
  Serial.println("\nCmd?");
#endif
}

bool detectGigatron()
{
  unsigned long timeout = millis() + 85;
  long T[2][2] = {{0, 0}, {0, 0}};

  // Sample the sync signals coming out of the controller port
  while (millis() < timeout) {
    byte pinb = PINB; // capture SER_PULSE and SER_LATCH at the same time
    T[ (pinb >> SER_LATCH) & 1 ][ (pinb >> SER_PULSE) & 1 ]++;
  }

  float S = T[0][0] + T[0][1] + T[1][0] + T[1][1] + .1; // Avoid zero division (pedantic)
  float vSync = (T[0][0] + T[0][1]) / ( 8 * S / 521); // Adjusted vSync signal
  float hSync = (T[0][0] + T[1][0]) / (96 * S / 800); // Standard hSync signal

  // Check that vSync and hSync characteristics look normal
  return 0.95 <= vSync && vSync <= 1.20 && 0.95 <= hSync && hSync <= 1.05;
}

void doCommand(char line[])
{
  switch (toupper(line[0])) {
    case 'V': doVersion();                       break;
    case 'H': doHelp();                          break;
    case 'R': doReset();                         break;
    case 'L': doLoader();                        break;
    case 'P': doTransfer(gt1File);               break;
    case 'U': doTransfer(NULL);                  break;
    case 'W': sendController(~buttonUp,      2); break;
    case 'A': sendController(~buttonLeft,    2); break;
    case 'S': sendController(~buttonDown,    2); break;
    case 'D': sendController(~buttonRight,   2); break;
    case 'Z': sendController((byte)~buttonA, 2); break;
    case 'X': sendController(~buttonB,       2); break;
    case 'Q': sendController(~buttonSelect,  2); break;
    case 'E': sendController(~buttonStart,   2); break;
    case 0:   /* Empty line */                   break;

    case EMU_PS2_ENABLE: emulatorControl = true; break;

#if hasSerial
    default:
      Serial.println("!Unknown command (type 'H' for help)");
#endif
  }
  prompt();
}

void doEmulator(char line[])
{
  switch (line[0]) {
    case EMU_PS2_LEFT:    sendController(~buttonLeft,   2);      break;
    case EMU_PS2_RIGHT:   sendController(~buttonRight,  2);      break;
    case EMU_PS2_UP:      sendController(~buttonUp,     2);      break;
    case EMU_PS2_DOWN:    sendController(~buttonDown,   2);      break;
    case EMU_PS2_START:   sendController(~buttonStart,  128 + 32); break;
    case EMU_PS2_SELECT:  sendController(~buttonSelect, 2);      break;
    case EMU_PS2_INPUT_A: sendController((byte)~buttonA, 2);      break;
    case EMU_PS2_INPUT_B: sendController(~buttonB,      2);      break;
    case EMU_PS2_CR:      sendController('\n',          2);      break;
    case EMU_PS2_DEL:     sendController(127,           2);      break;

    case EMU_PS2_DISABLE: emulatorControl = false;               break;

    default:              sendController(line[0],       2);      break;
  }
  prompt();
}

void doVersion()
{
#if hasSerial
  Serial.println(":Gigatron Interface Adapter [" version "]\n"
                 ":Type 'H' for help");
#endif
}

void doHelp()
{
#if hasSerial
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
#endif
}

void doReset()
{
  // Soft reset: hold start for >128 frames (>2.1 seconds)
#if hasSerial
  Serial.println(":Resetting Gigatron");
  Serial.flush();
#endif
  sendController(~buttonStart, 128 + 32);

  // Wait for main menu to be ready
  delay(1500);
}

void doLoader()
{
  // Navigate menu. 'Loader' is at the bottom
#if hasSerial
  Serial.println(":Starting Loader from menu");
  Serial.flush();
#endif

  for (int i = 0; i < 10; i++) {
    sendController(~buttonDown, 2);
    delay(50);
  }

  // Start 'Loader' application on Gigatron
  sendController((byte)~buttonA, 2);

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

#if hasSerial
#define readNext() {\
    nextByte = gt1 ? pgm_read_byte(gt1++) : nextSerial();\
    if (nextByte < 0) return;\
  }
#define ask(n)\
  if (!gt1) {\
    Serial.print(n);\
    Serial.println("?");\
  }
#else
#define readNext() {\
    nextByte = pgm_read_byte(gt1++);\
    if (nextByte < 0) return;\
  }
#define ask(n)
#endif

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
    for (int i = 0; i < len; i++) {
      readNext();
      segment[i] = nextByte;
    }

    // Check that segment doesn't cross the page boundary
    if ((address & 255) + len > 256) {
#if hasSerial
      Serial.println("!Data error (page overflow)");
#endif
      return;
    }

    // Send downstream
#if hasSerial
    Serial.print(":Loading ");
    Serial.print(len);
    Serial.print(" bytes at $");
    Serial.println(address, HEX);
    Serial.flush();
#endif
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
#if hasSerial
    Serial.print(":Executing from $");
    Serial.println(address, HEX);
    Serial.flush();
#endif
    sendGt1Execute(address, segment + 240);
  }
}

int nextSerial()
{
#if hasSerial
  unsigned long timeout = millis() + 5000;

  while (!Serial.available() && millis() < timeout)
    ;

  int nextByte = Serial.read();
  if (nextByte < 0)
    Serial.println("!Timeout error (no data)");

  return nextByte;
#endif
}

// Send a 1..256 byte code or data segment into the Gigatron by
// repacking it into Loader frames of max N=60 payload bytes each.
void sendGt1Segment(word address, int len, byte data[])
{
  noInterrupts();
  byte n = min(N, len);
  resetChecksum();

  // Send segment data
  for (int i = 0; i < len; i += n) {
    n = min(N, len - i);
    sendFrame('L', n, address + i, data + i);
  }
  interrupts();

  // Wait for vBlank to start so we're 100% sure to skip one frame and
  // the checksum resets on the other side. (This is a bit pedantic)
  while (PINB & (1 << SER_LATCH)) // ~160 us
    ;
}

// Send execute command
void sendGt1Execute(word address, byte data[])
{
  noInterrupts();
  resetChecksum();
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
  for (int i = 0; i < n; i++) {
    sendFirstByte(value);
    PORTB |= 1 << SER_DATA; // Send 1 when idle
  }

  interrupts(); // So delay() can work again
}

void resetChecksum()
{
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
  // 2. There is a 1 bit latency inside the 74HC595 for the data bit,
  // but (obviously) not for the sync signals.
  // All together, we drop 2 bits from the 2nd byte in a frame. This achieves
  // byte alignment for the Gigatron at visible scanline 3, 11, 19, ... etc.

  sendFirstByte(firstByte);    // Protocol byte
  checksum += firstByte << 6;  // Keep Loader.gcl dumb
  sendBits(len, 6);            // Length 0, 1..60
  sendBits(address & 255, 8);  // Low address bits
  sendBits(address >> 8, 8);   // High address bits
  for (byte i = 0; i < N; i++) // Payload bytes
    sendBits(message[i], 8);
  byte lastByte = -checksum;   // Checksum must come out as 0
  sendBits(lastByte, 8);
  checksum = lastByte;         // Concatenate checksums
  PORTB |= 1 << SER_DATA;      // Send 1 when idle
}

void sendFirstByte(byte value)
{
  // Wait vertical sync NEGATIVE edge to sync with loader
  while (~PINB & (1 << SER_LATCH)) // Ensure vSync is HIGH first
    ;
  while (PINB & (1 << SER_LATCH)) // Then wait for vSync to drop
    ;

  // Send first bit
  if (value & 128)
    PORTB |= 1 << SER_DATA;
  else
    PORTB &= ~(1 << SER_DATA);

  // Wait for bit transfer at horizontal sync POSITIVE edge.
  // This timing is tight for the first bit of the first byte and
  // the reason that interrupts must be disabled on the microcontroller.
  while (PINB & (1 << SER_PULSE)) // Ensure hSync is LOW first
    ;
  while (~PINB & (1 << SER_PULSE)) // Then wait for hSync to rise
    ;

  // Send remaining bits
  sendBits(value, 7);
}

// Send n bits, highest first
void sendBits(byte value, byte n)
{
  for (byte bit = 1 << (n - 1); bit; bit >>= 1) {
    // Send next bit
    if (value & bit)
      PORTB |= 1 << SER_DATA;
    else
      PORTB &= ~(1 << SER_DATA);

    // Wait for bit transfer at horizontal sync POSITIVE edge.
    while (PINB & (1 << SER_PULSE)) // Ensure hSync is LOW first
      ;
    while (~PINB & (1 << SER_PULSE)) // Then wait for hSync to rise
      ;
  }
  checksum += value;
}
