
// Interfacing with the Gigatron TTL microcomputer using a microcontroller
//
//               4. Game Controller
//                  (optional)
//                       |
//                       v
//                 +-----------+
//   Gigatron      |           |     PC/laptop over USB (optional)
//  controller <---| BabelFish |<--- 1. Send GT1 files
//   port J4       |           |     2. Serial console
//                 +-----------+
//                       ^
//                       |
//               3. PS/2 keyboard
//                  (optional)
//
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
// 4. Pass through of game controller signals
//
// Select 1 of the preconfigured platforms:
// (not every microcontroller platform supports all functions)
//
#define ArduinoUno   0 // Default
#define ArduinoNano  0
#define ArduinoMicro 1
#define ATtiny85     0

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

/*----------------------------------------------------------------------+
 |                                                                      |
 |      Preset configuarations                                          |
 |                                                                      |
 +----------------------------------------------------------------------*/

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
 #define platform "ArduinoUno"
 #define maxStorage 32768

 // Pinout reference:
 // https://i2.wp.com/marcusjenkins.com/wp-content/uploads/2014/06/ARDUINO_V2.png

 // Pins for Gigatron (must be on PORTB)
 #define gigatronDataPin  13
 #define gigatronLatchPin 12
 #define gigatronPulsePin 11
 #define gigatronPinToBitMask digitalPinToBitMask

 // Pins for Controller
 #define gameControllerDataPin -1

 // Pins for PS/2 keyboard (Arduino Uno)
 #define keyboardClockPin 3 // Pin 2 or 3 for IRQ
 #define keyboardDataPin  4 // Any available free pin

 // Link to PC/laptop
 #define hasSerial 1
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
 // at67's setup
 #define platform "ArduinoNano"
 #define maxStorage 32768

 // Pinout reference:
 // http://lab.dejaworks.com/wp-content/uploads/2016/08/Arduino-Nano-1024x500.png
 // Note that pin 11 and 12 are wrong on some versions of these diagrams

 // Pins for Gigatron (must be on PORTB)
 #define gigatronDataPin  13 // PB5
 #define gigatronLatchPin 12 // PB4
 #define gigatronPulsePin 11 // PB3
 #define gigatronPinToBitMask digitalPinToBitMask // Regular Arduino pin numbers

 // Pins for Controller
 #define gameControllerDataPin 10 // PB2

 // Pins for PS/2 keyboard
 #define keyboardClockPin 3 // Pin 2 or 3 for IRQ
 #define keyboardDataPin  4 // Any available free pin

 // Link to PC/laptop
 #define hasSerial 1
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
 // WattSekunde's setup
 #define platform "ArduinoMicro"
 #define maxStorage 32768

 // Pinout reference:
 // http://1.bp.blogspot.com/-xqhL0OrJcxo/VJhVxUabhCI/AAAAAAABEVk/loDafkdqLxM/s1600/micro_pinout.png

 // Pins for Gigatron (must be on PORTB)
 #define gigatronDataPin  PB1
 #define gigatronLatchPin PB3
 #define gigatronPulsePin PB2
 // These are not regular Arduino pin numbers
 #define gigatronPinToBitMask(pin) (1 << (pin))

 // Pins for Controller
 #define gameControllerDataPin -1

 // Pins for PS/2 keyboard
 #define keyboardClockPin 3 // Pin 2 or 3 for IRQ
 #define keyboardDataPin  4 // Any available free pin

 // Link to PC/laptop
 #define hasSerial 1
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
 #define platform "ATtiny85"
 #define maxStorage 8192

 // Pins for Gigatron (must be on PORTB)
 #define gigatronDataPin  PB2
 #define gigatronLatchPin PB1
 #define gigatronPulsePin PB0
 // These are not regular Arduino pin numbers
 #define gigatronPinToBitMask(pin) (1 << (pin))

 // Pins for Controller
 #define gameControllerDataPin -1 // XXX Maybe use ~RESET for this some day

 // Pins for PS/2 keyboard
 #define keyboardClockPin PB4
 #define keyboardDataPin  PB3

 // Link to PC/laptop
 #define hasSerial 0
#endif

/*----------------------------------------------------------------------+
 |                                                                      |
 |      Built-in GT1 images                                             |
 |                                                                      |
 +----------------------------------------------------------------------*/

const byte TinyBASIC_gt1[] PROGMEM = {
  #include "TinyBASIC.h"
};
const byte WozMon_gt1[] PROGMEM = {
  #include "WozMon.h"
};
const byte Terminal_gt1[] PROGMEM = {
  #include "Terminal.h"
};
const byte Blinky_gt1[] PROGMEM = {
  #include "Blinky.h"
};
const byte bricks_gt1[] PROGMEM = {
  #include "bricks.h"
};

#if maxStorage >= 32768
const byte lines_gt1[] PROGMEM = {
  #include "lines.h"
};
const byte life3_gt1[] PROGMEM = {
  #include "life3.h"
};
const byte starfield_gt1[] PROGMEM = {
  #include "starfield.h"
};
const byte tetris_gt1[] PROGMEM = {
  #include "tetris.h"
};
#endif

struct { byte *gt1; char *name; } gt1Files[12] = {
/* 1  */ TinyBASIC_gt1, "Tiny BASIC",
#if maxStorage >= 32768
/* 2  */ WozMon_gt1,    "WozMon",
/* 3  */ Terminal_gt1,  "Terminal",
/* 4  */ Blinky_gt1,    "Blinky",
#else
         0,              "(Empty)",
         0,              "(Empty)",
         0,              "(Empty)",
#endif
/* 5  */ bricks_gt1,    "Bricks game [xbx]",
#if maxStorage >= 32768
/* 6  */ lines_gt1,     "Lines demo [at67]",
/* 7  */ life3_gt1,     "Game of Life demo [at67]",
/* 8  */ starfield_gt1, "Starfield demo [at67]",
/* 9  */ tetris_gt1,    "Tetris game [at67]",
#else
/* 7  */ 0,             "(Empty)",
/* 8  */ 0,             "(Empty)",
/* 9  */ 0,             "(Empty)",
#endif
/* 10 */ 0,             "(Empty)",
/* 11 */ 0,             "(Empty)",
/* 12 */ 0,             "(Empty)",
};

/*----------------------------------------------------------------------+
 |                                                                      |
 |      End config section                                              |
 |                                                                      |
 +----------------------------------------------------------------------*/

/*
 *  Bit masks for pins
 */
byte gigatronDataBit;
byte gigatronLatchBit;
byte gigatronPulseBit;

/*
 *  Loader protocol
 */
#define N 60 // Payload bytes per transmission frame
byte checksum; // Global is simplest

/*
 *  Game controller button mapping
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
 *  Terminal mode for upstream host
 */
static bool echo = false;

/*
 *  Setup runs once when the Arduino wakes up
 */
void setup()
{
  gigatronDataBit  = gigatronPinToBitMask(gigatronDataPin);
  gigatronLatchBit = gigatronPinToBitMask(gigatronLatchPin);
  gigatronPulseBit = gigatronPinToBitMask(gigatronPulsePin);

  // Enable output pin (pins are set to input by default)
  PORTB |= gigatronDataBit; // Send 1 when idle
  DDRB = gigatronDataBit;

  // Open upstream communication
  #if hasSerial
    Serial.begin(115200);
  #endif
  doVersion();

  // In case we power on together with the Gigatron, this is a
  // good pause to wait for the video loop to have started
  delay(350);

  // Note that it takes 500~750 ms for PS/2 keyboards to boot
  keyboard_setup();

  prompt();
}

/*
 *  Loop runs repeatedly
 */
void loop()
{
  // Controller pass through
  #if gameControllerDataPin >= 0
    digitalWrite(gigatronDataPin, digitalRead(gameControllerDataPin));
  #endif

  // Commands from upstream USB (PC/laptop)
  #if hasSerial
    static char line[32], next = 0, last;
    static byte lineIndex = 0;
    if (Serial.available()) {
      last = next;
      char next = Serial.read();
      sendEcho(next, last);
      if (lineIndex < sizeof line)
        line[lineIndex++] = next;
      if (next == '\r' || next == '\n') {
        line[lineIndex-1] = '\0';
        doCommand(line);
        lineIndex = 0;
      }
    }
  #endif

  // PS/2 keyboard events
  byte key = keyboard_getState();
  if (key != 255) {
    byte f = fnKey(key^64);
    if (f)                           // Ctrl + Function key
      if (gt1Files[f-1].gt1)
        doTransfer(gt1Files[f-1].gt1);// Send built-in GT1 file to Gigatron

    while (1) {
      if (!fnKey(key^64)) {          // Normal case (but skip Ctrl+Fxx)
        critical();
        sendFirstByte(key);          // Synchronize with vPulse and send code
        nonCritical();
      }
      if (key == 255)                // Until state is idle again
        break;
      delay(15);                     // Allow PS/2 interrupts for a reasonable window
      key = keyboard_getState();
    }
  }
}

void prompt()
{
  #if hasSerial
    Serial.println(detectGigatron() ? ":Gigatron OK" : "!Gigatron offline");
    Serial.println("Cmd?");
  #endif
}

bool detectGigatron()
{
  unsigned long timeout = millis() + 85;
  long T[4] = {0, 0, 0, 0};

  // Sample the sync signals coming out of the controller port
  while (millis() < timeout) {
    byte pinb = PINB; // capture SER_PULSE and SER_LATCH at the same time
    T[(pinb & gigatronLatchBit ? 2 : 0) + (pinb & gigatronPulseBit ? 1 : 0)]++;
  }

  float S = T[0] + T[1] + T[2] + T[3] + .1;     // Avoid zero division (pedantic)
  float vSync = (T[0] + T[1]) / ( 8 * S / 521); // Adjusted vSync signal
  float hSync = (T[0] + T[2]) / (96 * S / 800); // Standard hSync signal

  // Check that vSync and hSync characteristics look normal
  return 0.95 <= vSync && vSync <= 1.20 && 0.95 <= hSync && hSync <= 1.05;
}

void sendEcho(char next, char last)
{
  #if hasSerial
    if (echo)
      switch (next) {
        case 127:  Serial.print("\b \b"); break;
        case '\n': if (last == '\r')      break; // else FALL THROUGH
        case '\r': Serial.println();      break;
        default: Serial.print(next);
      }
  #endif
}

void doCommand(char line[])
{
  int arg = line[0] ? atoi(&line[1]) : 0;
  switch (toupper(line[0])) {
  case 'V': doVersion();                      break;
  case 'H': doHelp();                         break;
  case 'R': doReset(arg);                     break;
  case 'L': doLoader();                       break;
  case 'P': arg = constrain(arg, 1, 12);
            if (gt1Files[arg-1].gt1)
              doTransfer(gt1Files[arg-1].gt1);break;
  case 'U': doTransfer(NULL);                 break;
  case '.': doLine(&line[1]);                 break;
  case 'C': doEcho(!echo);                    break;
  case 'T': doTerminal();                     break;
  case 'W': sendController(~buttonUp,     2); break;
  case 'A': sendController(~buttonLeft,   2); break;
  case 'S': sendController(~buttonDown,   2); break;
  case 'D': sendController(~buttonRight,  2); break;
  case 'Z': sendController((byte)~buttonA,2); break;
  case 'X': sendController(~buttonB,      2); break;
  case 'Q': sendController(~buttonSelect, 2); break;
  case 'E': sendController(~buttonStart,  2); break;
  case 0: /* Empty line */                    break;
  default:
    #if hasSerial
      Serial.println("!Unknown command (type 'H' for help)");
    #endif
    ;
  }
  prompt();
}

void doVersion()
{
  #if hasSerial
    Serial.println(":BabelFish platform=" platform);
    Serial.println(":Pins:");
    #define V(s) #s
    #define Q(s) V(s)
    Serial.println(": Gigatron data=" Q(gigatronDataPin) " latch=" Q(gigatronLatchPin) " pulse=" Q(gigatronPulsePin));
    Serial.println(": Keyboard clock=" Q(keyboardClockPin) " data=" Q(keyboardDataPin));
    Serial.println(": Controller data=" Q(gameControllerDataPin));
    Serial.println(":PROGMEM slots:");
    for (int i=1; i<=12; i++) {
      Serial.print(": ");
      Serial.print(i);
      Serial.print(") ");
      Serial.println(gt1Files[i-1].name);
    }
    doEcho(echo);
    Serial.println(":Type 'H' for help");
  #endif
}

void doEcho(byte value)
{
  #if hasSerial
    echo = value;
    Serial.print(":Echo ");
    Serial.println(value ? "on" : "off");
  #endif
}

void doHelp()
{
  #if hasSerial
    Serial.println(":Commands are");
    Serial.println(": V        Show configuration");
    Serial.println(": H        Show this help");
    Serial.println(": R        Reset Gigatron");
    Serial.println(": L        Start Loader");
    Serial.println(": P[<n>]   Transfer object file from PROGMEM slot <n> [1..12]");
    Serial.println(": U        Transfer object file from USB");
    Serial.println(": .<text>  Send text line as ASCII key strokes");
    Serial.println(": C        Toggle echo mode (default off)");
    Serial.println(": T        Enter terminal mode");
    Serial.println(": W/A/S/D  Up/left/down/right arrow");
    Serial.println(": Z/X      A/B button ");
    Serial.println(": Q/E      Select/start button");
  #endif
}

void doReset(int n)
{
  // Soft reset: hold start for >128 frames (>2.1 seconds)
  #if hasSerial
    Serial.println(":Resetting Gigatron");
    Serial.flush();
  #endif
  sendController(~buttonStart, n ? n : 128+32);

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

  for (int i=0; i<10; i++) {
    sendController(~buttonDown, 2);
    delay(50);
  }

  // Start 'Loader' application on Gigatron
  sendController((byte)~buttonA, 2);

  // Wait for Loader to be running
  delay(1000);
}

void doLine(char *line)
{
  // Pass through the line of text
  for (int i=0; line[i]; i++) {
    sendController(line[i], 1);
    delay(20); // Allow Gigatron software to process key code
  }
  // And terminal with a CR
  sendController('\n', 1);
  delay(50); // Allow Gigatron software to process line
}

// In terminal mode we transfer every incoming character to
// the Gigatron, with some substitutions for convenience.
// This lets you type directly into BASIC and WozMon from
// a terminal window on your PC or laptop.
//
//      picomon -b 115200 /dev/tty.usbmodem1411
//      screen /dev/tty.usbmodem1411 115200

void doTerminal()
{
  #if hasSerial
    Serial.println(":Entering terminal mode");
    Serial.println(":Exit with Ctrl-D");
    char next = 0, last;
    byte out;
    bool ansi = false;
    for (;;) {
      if (Serial.available()) {
        last = next;
        next = Serial.read();
        sendEcho(next, last);

        // Mappings for newline and arrow sequences
        out = next;
        switch (next) {
          case 4:                                  return;   // Ctrl-D (EOT)
          case 9: out = ~buttonB;                  break;    // Same as PS/2 above
          case '\r': out = '\n';                   break;
          case '\n': if (last == '\r')             continue; // Swallow
          case '\e':                               continue; // ANSI escape sequence
          case '[': if (last == '\e') ansi = true; continue;
          case 'A': if (ansi) out = ~buttonUp;     break;    // Map cursor keys to buttons
          case 'B': if (ansi) out = ~buttonDown;   break;
          case 'C': if (ansi) out = ~buttonRight;  break;
          case 'D': if (ansi) out = ~buttonLeft;   break;
        }

        sendController(out, 2);
        ansi = false;
      }
    }
  #endif
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
    for (int i=0; i<len; i++) {
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
    sendGt1Execute(address, segment+240);
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

    // Workaround suspected bug in USB support for ATmega32U4 (Arduino Micro,
    // Leonardo, etc) in Arduino's USBCore.cpp.
    // From Atmel-7766J-USB-ATmega16U4/32U4-Datasheet_04/2016
    // 22.13 OUT endpoint management:
    //
    //   "RXOUTI shall always be cleared before clearing FIFOCON."
    //
    // (An identical remark in datasheet for ATmega32U6/AT90USB64/128)
    //
    // However:
    //   Serial.read() ->
    //   CDC.cpp/Serial_::read ->
    //   USBCore.cpp/USB_Recv() ->
    //   SBCore.cpp/ReleaseRX() ->
    //   UEINTX = 0x6B;  // FIFOCON=0 NAKINI=1 RWAL=1 NAKOUTI=0 RXSTPI=1 RXOUTI=0 STALLEDI=1 TXINI=1
    //
    // This last statement attempts to clear both bits AT ONCE. This fails to
    // clear FIFOCON when host data arrives in exact multiples of 64,128,192
    // etcetara bytes and when using double buffering with two banks of
    // 64 bytes, as USBCore.cpp does. A hangup situation is the result after
    // reading the first 64 bytes transmitted. This can only be solved by
    // resetting the board because no further host data reaches the sketch.
    // (A better fix would be to repair the Arduino's USB_Recv and ReleaseRX)
    #if defined(USBCON) && defined(UEINTX) && defined(UEBCLX)
      if (!UEBCLX)                 // If bank empty
        UEINTX &= ~(1 << FIFOCON); // Clear FIFOCON bit
    #endif

    return nextByte;
  #endif
}

static inline void critical()
{
  forbidPs2();
  noInterrupts();
}

static inline void nonCritical()
{
  interrupts();
  allowPs2();
}

// Send a 1..256 byte code or data segment into the Gigatron by
// repacking it into Loader frames of max N=60 payload bytes each.
void sendGt1Segment(word address, int len, byte data[])
{
  byte n = min(N, len);
  resetChecksum();

  // Send segment data
  critical();
  for (int i=0; i<len; i+=n) {
    n = min(N, len-i);
    sendFrame('L', n, address+i, data+i);
  }
  nonCritical();

  // Wait for vPulse to start so we're 100% sure to skip one frame and
  // the checksum resets on the other side. (This is a bit pedantic)
  while (PINB & gigatronLatchBit) // ~160 us
    ;
}

// Send execute command
void sendGt1Execute(word address, byte data[])
{
  critical();
  resetChecksum();
  sendFrame('L', 0, address, data);
  nonCritical();
}

// Pretend to be a game controller
// Send the same byte a few frames like a human user
void sendController(byte value, int n)
{
  // Send controller code for n frames
  // E.g. 4 frames = 3/60s = ~50 ms
  critical();
  for (int i=0; i<n; i++)
    sendFirstByte(value);
  nonCritical();

  PORTB |= gigatronDataBit; // Send 1 when idle
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
  sendBits(address&255, 8);    // Low address bits
  sendBits(address>>8, 8);     // High address bits
  for (byte i=0; i<N; i++)     // Payload bytes
    sendBits(message[i], 8);
  byte lastByte = -checksum;   // Checksum must come out as 0
  sendBits(lastByte, 8);
  checksum = lastByte;         // Concatenate checksums
  PORTB |= gigatronDataBit;        // Send 1 when idle
}

void sendFirstByte(byte value)
{
  // Wait vertical sync NEGATIVE edge to sync with loader
  while (~PINB & gigatronLatchBit) // Ensure vSync is HIGH first
    ;

  // Send first bit in advance
  if (value & 128)
    PORTB |= gigatronDataBit;
  else
    PORTB &= ~gigatronDataBit;

  while (PINB & gigatronLatchBit) // Then wait for vSync to drop
    ;

  // Wait for bit transfer at horizontal sync RISING edge. As this is at
  // the end of a short (3.8 us) pulse following VERY shortly (0.64us) after
  // vSync drop, this timing is tight. That is the reason that interrupts
  // must be disabled on the microcontroller and that 1 MHz is not enough.
  while (PINB & gigatronPulseBit) // Ensure hSync is LOW first
    ;
  while (~PINB & gigatronPulseBit) // Then wait for hSync to rise
    ;

  // Send remaining bits
  sendBits(value, 7);
}

// Send n bits, highest first
void sendBits(byte value, byte n)
{
  for (byte bit=1<<(n-1); bit; bit>>=1) {
    // Send next bit
    if (value & bit)
      PORTB |= gigatronDataBit;
    else
      PORTB &= ~gigatronDataBit;

    // Wait for bit transfer at horizontal sync POSITIVE edge.
    while (PINB & gigatronPulseBit)  // Ensure hSync is LOW first
      ;
    while (~PINB & gigatronPulseBit) // Then wait for hSync to rise
      ;
  }
  checksum += value;
}

