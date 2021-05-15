
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
//       program on the PC/laptop as sender (sendFile.py).
//    b. PROGMEM only requires the Arduino IDE on the PC/laptop,
//       but needs the GT1 file as a hexdump (C notation).
// 2. Hookup a PS/2 keyboard for typing on the Gigatron
// 3. Forward text from USB to Gigatron as keystrokes
//    For example to get a long BASIC program loaded into BASIC
// 4. Controlling the Gigatron over USB from a PC/laptop
// 5. Passing through of game controller signals
// 6. Receive data from Gigatron and store it in the EEPROM area
//
// Select one of the supported platforms in the Tools->Board menu.
//
// Supported:
//      - Arduino/Genuino Uno
//      - Arduino Nano
//      - Arduino/Genuino Micro
//      - ATtiny85 (8 MHz)
//
// (not every microcontroller platform supports all functions)

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

#if defined(ARDUINO_AVR_UNO)
 #define platform "ArduinoUno"
 #define maxStorage 32256

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

#if defined(ARDUINO_AVR_NANO)
 // at67's setup
 #define platform "ArduinoNano"
 #define maxStorage 30720

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

#if defined(ARDUINO_AVR_MICRO)
 // WattSekunde's setup
 #define platform "ArduinoMicro"
 #define maxStorage 28672

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

#if defined(ARDUINO_attiny)
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
  #include "TinyBASIC_v3.h"
};
const byte WozMon_gt1[]    PROGMEM = {
  #include "WozMon.h"
};
const byte Terminal_gt1[]  PROGMEM = {
  #include "Terminal.h"
};
const byte Blinky_gt1[]    PROGMEM = {
  #include "Blinky.h"
};
const byte bricks_gt1[]    PROGMEM = {
  #include "bricks.h"
};
const byte lines_gt1[]     PROGMEM = {
  #include "lines.h"
};
const byte life3_gt1[]     PROGMEM = {
  #include "life3.h"
};
const byte starfield_gt1[] PROGMEM = {
  #include "starfield.h"
};
const byte tetris_gt1[]    PROGMEM = {
  #include "tetris.h"
};

const struct { const byte *gt1; const char *name; } gt1Files[] = {
  { TinyBASIC_gt1, "BASIC"                    }, // 3073 bytes
#if maxStorage >= 10000
  { WozMon_gt1,    "WozMon"                   }, // 595 bytes
  { Terminal_gt1,  "Terminal"                 }, // 256 bytes
  { Blinky_gt1,    "Blinky"                   }, // 17 bytes
  { lines_gt1,     "Lines demo [at67]"        }, // 304 bytes
  { life3_gt1,     "Game of Life demo [at67]" }, // 441 bytes
  { starfield_gt1, "Starfield demo [at67]"    }, // 817 bytes
#endif
#if maxStorage >= 30000
  { bricks_gt1,    "Bricks game [xbx]"        }, // 1607 bytes
  { tetris_gt1,    "Tetris game [at67]"       }, // 9868 bytes
#endif
  { NULL,          "-SAVED-"                  },
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
#define N 60         // Payload bytes per transmission frame
byte checksum;       // Global is simplest
byte outBuffer[256]; // sendFrame() will read up to index 299 but that's ok.
                     // outBuffer[] is global, because having it on the stack
                     // can cause trouble on the ATtiny85 (not fully clear why)
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
// Note: The kit's game controller gives inverted signals.


/*
 *  Emulator control
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
 *  Font data
 */
const int tinyfont[96] PROGMEM = {
  #include "tinyfont.h"
};

/*
 *  Terminal mode for upstream host
 */
static bool echo = false;

/*
 *  Non-volatile memory
 */
#include <EEPROM.h>

struct EEPROMlayout {
  byte keymapIndex;
  byte savedFile[];
};

#define fileStart offsetof(struct EEPROMlayout, savedFile)
static word saveIndex = fileStart; // Write pointer into EEPROM for file (BASIC)
static word EEPROM_length;

#define arrayLen(a) ((int) (sizeof(a) / sizeof((a)[0])))
extern const byte nrKeymaps; // From in PS2.ino

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
    doVersion();
  #endif

  // Cache for speed
  EEPROM_length = EEPROM.length();

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
  // Check Gigatron's vPulse for incoming data
  static byte inByte, inBit, inLen;

  critical();
  byte newValue = waitVSync();
  nonCritical();

  switch (newValue) {

    case 9:                            // Received one bit
      inByte |= inBit;
      // !!! FALL THROUGH !!!

    case 7:                            // Received zero bit
      inBit <<= 1;

      if (saveIndex >= EEPROM_length)  // EEPROM overflow
        if (inLen > 0)                 // Only break if this can't be a new program
          sendController(3, 10);       // Send long Ctrl-C back

      if (inBit != 0)
        break;

      EEPROM.write(saveIndex++, inByte);// Store every full byte

      #if hasSerial
        if (inByte == 10)
          Serial.print('\r');
        Serial.print((char)inByte);    // Forward to host
        noInterrupts();
      #endif

      if (inByte == 10) {              // End of line?
        EEPROM.write(saveIndex, 255);  // Terminate, in case this was the last line
        if (inLen == 0)                // An empty line clears the old program
          saveIndex = fileStart;
        inLen = 0;
      } else
        inLen++;

      inByte = 0;                      // Prepare for next byte
      inBit = 1;
      break;

    default:
      inByte = 0;                      // Reset incoming data state
      inBit = 1;
      inLen = 0;
      break;
  }

  // Game controller pass through
  #if gameControllerDataPin >= 0
    digitalWrite(gigatronDataPin, digitalRead(gameControllerDataPin));
  #endif

  // PS/2 keyboard events
  delay(15);                           // Allow PS/2 interrupts for a reasonable window
  byte key = keyboard_getState();
  if (key != 255) {
    byte f = fnKey(key ^ 64);          // Ctrl+Fn key?
    if (f) {
      if (f == 1)
        doMapping();                   // Ctrl-F1 is help
      else if (f-2 < arrayLen(gt1Files)) {
        if (gt1Files[f-2].gt1)
          doTransfer(gt1Files[f-2].gt1); // Send built-in GT1 file to Gigatron
        else
          sendSavedFile();
      }
    }
    for (;;) {                         // Focus all attention on PS/2 until state is idle again
      if (!fnKey(key ^ 64)) {          // Filter away the Ctrl+Fn combinations here
        critical();
        sendFirstByte(key);            // Synchronize with vPulse and send ASCII code
        nonCritical();
      }
      if (key == 255)                  // Break after returning to the idle state
        break;
      delay(15);                       // Allow PS/2 interrupts, so we can receive break codes
      key = keyboard_getState();       // This typically returns the same key for a couple of frames
    }
  }

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
        (emulatorControl) ? doEmulator(line) : doCommand(line);
        lineIndex = 0;
      }
    }
  #endif
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
  return 0.95 <= vSync && vSync <= 1.20 && 0.90 <= hSync && hSync <= 1.10;
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
  case 'M': doMapping();                      break;
  case 'P': if (0 <= arg && arg < arrayLen(gt1Files))
              doTransfer(gt1Files[arg].gt1);  break;
  case 'U': doTransfer(NULL);                 break;
  case '.': doLine(&line[1]);                 break;
  case 'C': doEcho(!echo);                    break;
  case 'T': doTerminal();                     break;
  case 'W': sendController(~buttonUp,     2); break;
  case 'A': sendController(~buttonLeft,   2); break;
  case 'S': sendController(~buttonDown,   2); break;
  case 'D': sendController(~buttonRight,  2); break;
  case 'Z': sendController(~buttonA & 255,2); break;
  case 'X': sendController(~buttonB,      2); break;
  case 'Q': sendController(~buttonSelect, 2); break;
  case 'E': sendController(~buttonStart,  2); break;
  case 0: /* Empty line */                    break;
  
  case EMU_PS2_ENABLE: emulatorControl = true; break;
  
  default:
    #if hasSerial
      Serial.println("!Unknown command (type 'H' for help)");
    #endif
    ;
  }
  prompt();
}

void doEmulator(char line[])
{
  switch (line[0]) {
    case EMU_PS2_LEFT:    sendController(~buttonLeft,   2);        break;
    case EMU_PS2_RIGHT:   sendController(~buttonRight,  2);        break;
    case EMU_PS2_UP:      sendController(~buttonUp,     2);        break;
    case EMU_PS2_DOWN:    sendController(~buttonDown,   2);        break;
    case EMU_PS2_START:   sendController(~buttonStart,  128 + 32); break;
    case EMU_PS2_SELECT:  sendController(~buttonSelect, 2);        break;
    case EMU_PS2_INPUT_A: sendController((byte)~buttonA,2);        break;
    case EMU_PS2_INPUT_B: sendController(~buttonB,      2);        break;
    case EMU_PS2_CR:      sendController('\n',          2);        break;
    case EMU_PS2_DEL:     sendController(127,           2);        break;

    case EMU_PS2_DISABLE: emulatorControl = false; break;

    default: sendController(line[0], 2); break;
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
    Serial.println(":EEPROM:");
    Serial.print(": size=");
    Serial.print(EEPROM.length());
    Serial.print(" mapping=");
    Serial.println(getKeymapName());
    Serial.println(":PROGMEM slots:");
    for (byte i=0; i<arrayLen(gt1Files); i++) {
      Serial.print(": P");
      Serial.print(i);
      Serial.print(") ");
      Serial.println(gt1Files[i].name);
    }
    doEcho(echo);
    Serial.println(":Type 'H' for help");
  #endif
}

// Show keymapping in Loader screen (Loader must be running)
void doMapping()
{
  word pos = 0x800;
  char text[] = "Ctrl-F1  This help";
  //             0123456789
  pos = renderLine(pos, text);
  text[9] = 0;
  for (byte i=0; i<arrayLen(gt1Files); i++) {
    byte f = i + 2;
    // To save space avoid itoa() or sprintf()
    text[6]      = '0' + f / 10;
    text[7]      = ' ';
    text[6+f/10] = '0' + f % 10;
    pos = renderString(pos, text);
    pos = renderLine(pos, gt1Files[i].name);
  }
  pos = renderString(pos, "Keymap: ");
  pos = renderString(pos, getKeymapName());
  pos = renderLine(pos, " (Change with Ctrl-Alt-Fxx)");
  pos = renderString(pos, "Available:");
  for (byte i=0; i<nrKeymaps; i++) {
    pos = renderString(pos, " ");
    pos = renderString(pos, getKeymapName(i));
  }
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
    Serial.println(": M        Show key mapping or menu in Loader screen");
    Serial.println(": P[<n>]   Transfer object file from PROGMEM slot <n> [1..12]");
    Serial.println(": U        Transfer object file from USB");
    Serial.println(": .<text>  Send text line as ASCII key strokes");
    Serial.println(": C        Toggle echo mode (default off)");
    Serial.println(": T        Enter terminal mode");
    Serial.println(": W/A/S/D  Up/left/down/right arrow");
    Serial.println(": Z/X      A/B button");
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

  for (byte i=0; i<10; i++) {
    sendController(~buttonDown, 2);
    delay(50);
  }

  // Start 'Loader' application on Gigatron
  sendController(~buttonA & 255, 2);

  // Wait for Loader to be running
  delay(1000);
}

void doLine(char *line)
{
  // Pass through the line of text
  for (byte i=0; line[i]; i++) {
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

// Render line in Loader screen
word renderLine(word pos, const char *text)
{
  pos = renderString(pos, text);
  return (pos & 0xff00) + 0x600; // Goes to new line
}

// Render string in Loader screen
word renderString(word pos, const char text[])
{
  // Send 6 pixel lines to Gigatron
  // The ATtiny85 doesn't have sufficient RAM for separate bitmap[] and
  // pixelLine[] arrays. Therefore the rendering must be redone with each
  // iteration, followed by an in-place conversion to pixel colors

  word p = pos;
  byte x;
  for (byte b=32; b; b>>=1) {

    // (Re-)render line of text in bitmap
    x = 0;
    for (byte i=0; text[i]!=0; i++) {

      // Get pixel data for character
      int pixels = pgm_read_word(&tinyfont[text[i]-32]);

      // Render character in bitmap
      if (pixels >= 0) {
        outBuffer[x++] = 0;                   // Regular position
        outBuffer[x++] = (pixels >> 9)  & 62;
        outBuffer[x++] = (pixels >> 4)  & 62;
        outBuffer[x++] = (pixels << 1)  & 62;
      } else {
        outBuffer[x++] = 0;                   // Shift down for g, j, p, q, y
        outBuffer[x++] = (pixels >> 10) & 31;
        outBuffer[x++] = (pixels >> 5)  & 31;
        outBuffer[x++] =  pixels        & 31;
        if (text[i] == 'j')                   // Special case to dot the j
          outBuffer[x-1] = '.';
      }
    }

    // Convert bitmap to pixels
    const byte bgColor = 32; // Blue
    const byte fgColor = 63; // White
    for (byte i=0; i<x; i++)
      outBuffer[i] = (outBuffer[i] & b) ? fgColor : bgColor;

    // Send line of pixels to Gigatron
    sendGt1Segment(p, x);

    // To next scanline
    p += 256;
  }

  return pos + x;
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
    if (!waitVSync()) {
      Serial.print("!Failed");
      return;
    }
  #endif

  #if hasSerial
    #define readNext()\
      if (gt1)\
        nextByte = pgm_read_byte(gt1++);\
      else {\
        nextByte = nextSerial();\
        if (nextByte < 0) return;\
      }
    #define ask(n)\
      if (!gt1) {\
        Serial.print(n);\
        Serial.println("?");\
      }
  #else
    #define readNext() (nextByte = pgm_read_byte(gt1++))
    #define ask(n)
  #endif

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
      outBuffer[i] = nextByte;
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
    sendGt1Segment(address, len);

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
    sendGt1Execute(address, outBuffer+240);
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
    // Leonardo, etcetera) in Arduino's USBCore.cpp. These boards don't have a
    // support processor for USB handling but use an on-chip USB controller
    // and a different software stack for that.
    // From Atmel-7766J-USB-ATmega16U4/32U4-Datasheet_04/2016:
    //
    //   "RXOUTI shall always be cleared before clearing FIFOCON."
    //
    // (An identical remark is in the datasheets for ATmega32U6/AT90USB64/128)
    //
    // However:
    //   Serial.read() ->
    //   CDC.cpp/Serial_::read ->
    //   USBCore.cpp/USB_Recv() ->
    //   USBCore.cpp/ReleaseRX() ->
    //   UEINTX = 0x6B;  // FIFOCON=0 NAKINI=1 RWAL=1 NAKOUTI=0 RXSTPI=1 RXOUTI=0 STALLEDI=1 TXINI=1
    //
    // This last statement attempts to clear both bits AT ONCE. This fails to
    // clear FIFOCON when host data arrives in exact multiples of 64,128,192,...
    // bytes and when using double buffering with two banks of bytes, as
    // USBCore.cpp does. A hangup situation occurs after reading the first
    // transmitted 64 bytes. This can then only be solved by resetting the board,
    // because no further host data reaches the sketch.
    //
    // A better fix would be to repair Arduino's USB_Recv and ReleaseRX.
    // See for follow-up https://github.com/arduino/Arduino/issues/7838
    // and https://github.com/kervinck/gigatron-rom/issues/36
    #if defined(USBCON) && defined(UEINTX) && defined(UEBCLX)
      if (!UEBCLX)                 // If bank empty
        UEINTX &= ~(1 << FIFOCON); // Clear FIFOCON bit
    #endif

    return nextByte;
  #endif
}

/*----------------------------------------------------------------------+
 |                                                                      |
 |      Gigatron communication                                          |
 |                                                                      |
 +----------------------------------------------------------------------*/

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
void sendGt1Segment(word address, int len)
{
  byte n = min(N, len);
  resetChecksum();

  // Send segment data
  critical();
  for (int i=0; i<len; i+=n) {
    n = min(N, len-i);
    sendFrame('L', n, address+i, outBuffer+i);
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
// Send the same byte a few frames, just like a human user
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
  PORTB |= gigatronDataBit;    // Send 1 when idle
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
  // must be disabled on the microcontroller (and why 1 MHz isn't enough).
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

// Count number of hSync pulses during vPulse
// This is a way for the Gigatron to send information out
byte waitVSync()
{
  word timeout = 0; // 2^16 cycles must give at least 17 ms

  // Wait vertical sync NEGATIVE edge

  while (~PINB & gigatronLatchBit) // Ensure vSync is HIGH first
    if (!--timeout)
      return 0;

  while (PINB & gigatronLatchBit) // Then wait for vSync to drop
    if (!--timeout)
      return 0;

  // Now count horizontal sync POSITIVE edges
  byte count = 0;
  for (;;) {
    while (PINB & gigatronPulseBit)  // Ensure hSync is LOW first
      ;
    if (PINB & gigatronLatchBit)     // Not in vPulse anymore
      break;
    while (~PINB & gigatronPulseBit) // Then wait for hSync to rise
      ;
    count += 1;
  }
  return count;
}

/*----------------------------------------------------------------------+
 |                                                                      |
 |      EEPROM functions                                                |
 |                                                                      |
 +----------------------------------------------------------------------*/

// Send a saved file as keystrokes to the Gigatron
void sendSavedFile()
{
    word i = fileStart;
    do {
      byte nextByte = EEPROM.read(i);
      if (nextByte == 255)
        break;
      sendController(nextByte, 1);
      delay(nextByte == 10 ? 70 : 20);  // Allow Gigatron software to process byte
    } while (++i < EEPROM.length());
}
