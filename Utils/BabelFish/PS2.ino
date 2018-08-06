
/*
 *  PS/2 keyboard handling for Babelfish talking to Gigatron
 *  Complete rewrite of PS2Keyboard for game controller mapping
 *  and keymap compression to reduce footprint
 *
 *  References:
 *
 *      https://www.avrfreaks.net/sites/default/files/PS2%20Keyboard.pdf
 *       The PS/2 Mouse/Keyboard Protocol
 *
 *      http://www.quadibloc.com/comp/scan.htm
 *       Scan Codes Demytified
 *
 *      https://retired.beyondlogic.org/keyboard/keybrd.htm
 *       Interfacing the AT keyboard
 *
 *      https://web.archive.org/web/20180101224739/http://www.computer-engineering.org/ps2keyboard/scancodes2.html
 *       Keyboard Scan Codes: Set 2
 *
 *      https://github.com/PaulStoffregen/PS2Keyboard
 *       PS/2 Keyboard Library for Arduino
 *
 *      https://github.com/techpaul/PS2KeyAdvanced
 *       Arduino PS2 Keyboard FULL keyboard protocol support and full keys to integer coding
 *
 *      https://github.com/techpaul/PS2KeyMap/
 *       Arduino PS2 keyboard International Keyboard mapping from PS2KeyAdvanced and return as UTF-8
 *
 *      https://www.terena.org/activities/multiling/ml-mua/test/kbd-all.html
 *       This page provides information about layouts of different national
 *       keyboard their mapping to used Character Sets and UCS
 *
 *      https://github.com/ilpianista/itlinux-win-keyboard
 *       An Italian keyboard layout for Windows, customized with shortcuts from Linux systems
 */

// TODO
// [Basic features]
// XXX Switching between keymaps
// XXX Test keymap for DE
// XXX Test keymap for FR
// XXX Test keymap for GB
// XXX Test keymap for IT
// XXX Test keymap for ES
// XXX Issue: Sometimes keyboard starts injecting break codes
//     when pressing [buttonA] + arrow keys, making it impossible
//     to set the time in Mandelbrot??
// XXX (Left)Alt for buttonB?
// XXX Pause/Break as alias buttonA? (To break BASIC programs)
// XXX How about keyboards without PageUp/PageDown?
//
// [Advanced features, for consideration]
// XXX Caps-Lock and setting LEDs
// XXX Do something with NumLock?
// XXX Key combination to reset keyboard?
// XXX Allow Gigatron to control the keyboard lights
// XXX Let keyboard lights run in a pattern (bad idea :-)

#define PS2_ENTER      '\n'
#define PS2_TAB        9
#define PS2_BACKSPACE  127

#define PS2_ESC        27
#define PS2_F1         (0xc0 + 1)
#define PS2_F2         (0xc0 + 2)
#define PS2_F3         (0xc0 + 3)
#define PS2_F4         (0xc0 + 4)
#define PS2_F5         (0xc0 + 5)
#define PS2_F6         (0xc0 + 6)
#define PS2_F7         (0xc0 + 7)
#define PS2_F8         (0xc0 + 8)
#define PS2_F9         (0xc0 + 9)
#define PS2_F10        (0xc0 + 10)
#define PS2_F11        (0xc0 + 11)
#define PS2_F12        (0xc0 + 12)

/*
 *  Keyboard layout mapping
 */

#define nrKeymaps 6
const char keymapNames[nrKeymaps][3] = {
  "US", "GB", "DE", "FR", "IT", "ES",
};

int getKeymapIndex()
{
  #define arrayLen(a) ((int) (sizeof(a) / sizeof((a)[0])))
  int index = EEPROM.read(offsetof(struct EEPROMlayout, keymapIndex));
  return (index > arrayLen(keymapNames)) ? 0 : index;
}

char *getKeymapName()
{
  return keymapNames[getKeymapIndex()];
}

enum {
  US = 1 << 0,
  GB = 1 << 1,
  DE = 1 << 2,
  FR = 1 << 3,
  IT = 1 << 4,
  ES = 1 << 5,
  NOMOD = 0 << nrKeymaps,
  SHIFT = 1 << nrKeymaps,
  ALTGR = 2 << nrKeymaps,
  EVERY = 3 << nrKeymaps,
};

typedef struct {
  byte flags; // Change from byte to word if we add more keymaps
  byte code;
  byte ascii; // XXX Remove this if we add flags. Value can be inferred by lookup()
} keyTuple_t;

// Keymaps courtesy of
//    Arduino PS2Keyboard library (US,DE,FR)
//    https://playground.arduino.cc/Main/PS2Keyboard
// and
//    Teensy PS2Keyboard library (GB,IT,ES)
//    http://www.pjrc.com/teensy/td_libs_PS2Keyboard.html
//
// 238 * 3 bytes = 714 bytes for holding 6 keyboard layouts
// To save space this table excludes codes that aren't in US-ASCII
// because these aren't in the Gigatron font either (yet).
// So accented letters and such are all absent. This is needed
// to make it all fit in the ATtiny85 configuration.

const PROGMEM keyTuple_t keymaps[] = {
  { +US+GB+DE+FR+IT+ES +EVERY,  13,   9 },
  { +US+GB+DE+FR+IT+ES +EVERY,  90,  10 },
  { +US+GB+DE+FR+IT+ES +EVERY, 118,  27 },
  { +US+GB+DE+FR+IT+ES +EVERY,  41,  32 },
  { +US+GB+DE   +IT+ES +SHIFT,  22,  33 }, // !
  {          +FR       +NOMOD,  74,  33 },
  {    +GB+DE   +IT+ES +SHIFT,  30,  34 }, // "
  {          +FR       +NOMOD,  38,  34 },
  { +US                +SHIFT,  82,  34 },
  { +US                +SHIFT,  38,  35 }, // #
  {          +FR   +ES +ALTGR,  38,  35 },
  {             +IT    +ALTGR,  82,  35 },
  {    +GB+DE          +NOMOD,  93,  35 },
  {       +DE+FR       +ALTGR,  93,  35 },
  { +US+GB+DE   +IT+ES +SHIFT,  37,  36 }, // $
  {          +FR       +NOMOD,  91,  36 },
  { +US+GB+DE   +IT+ES +SHIFT,  46,  37 }, // %
  {          +FR       +SHIFT,  82,  37 },
  {          +FR       +NOMOD,  22,  38 }, // &
  {       +DE   +IT+ES +SHIFT,  54,  38 },
  { +US+GB             +SHIFT,  61,  38 },
  {          +FR       +NOMOD,  37,  39 }, // '
  {             +IT+ES +NOMOD,  78,  39 },
  {                +ES +ALTGR,  78,  39 },
  { +US+GB             +NOMOD,  82,  39 },
  {       +DE          +NOMOD,  85,  39 },
  {       +DE          +SHIFT,  93,  39 },
  {          +FR       +NOMOD,  46,  40 }, // (
  {       +DE   +IT+ES +SHIFT,  62,  40 },
  { +US+GB             +SHIFT,  70,  40 },
  { +US+GB             +SHIFT,  69,  41 }, // )
  {       +DE   +IT+ES +SHIFT,  70,  41 },
  {          +FR       +NOMOD,  78,  41 },
  { +US+GB             +SHIFT,  62,  42 }, // *
  {       +DE   +IT+ES +SHIFT,  91,  42 },
  {          +FR       +NOMOD,  93,  42 },
  { +US+GB+DE+FR+IT+ES +EVERY, 124,  42 },
  { +US+GB   +FR       +SHIFT,  85,  43 }, // +
  {       +DE   +IT+ES +NOMOD,  91,  43 },
  { +US+GB+DE+FR+IT+ES +EVERY, 121,  43 },
  {          +FR       +NOMOD,  58,  44 }, // ,
  { +US+GB+DE   +IT+ES +NOMOD,  65,  44 },
  {             +IT+ES +ALTGR,  65,  44 },
  {          +FR       +NOMOD,  54,  45 }, // -
  {       +DE   +IT+ES +NOMOD,  74,  45 },
  {             +IT+ES +ALTGR,  74,  45 },
  { +US+GB             +NOMOD,  78,  45 },
  { +US+GB+DE+FR+IT+ES +EVERY, 123,  45 },
  {          +FR       +SHIFT,  65,  46 }, // .
  { +US+GB+DE   +IT+ES +NOMOD,  73,  46 },
  {             +IT+ES +ALTGR,  73,  46 },
  { +US+GB+DE+FR+IT+ES +EVERY, 113,  46 },
  {       +DE   +IT+ES +SHIFT,  61,  47 }, // /
  {          +FR       +SHIFT,  73,  47 },
  { +US+GB             +NOMOD,  74,  47 },
  { +US+GB+DE   +IT+ES +NOMOD,  69,  48 }, // 0
  {          +FR       +SHIFT,  69,  48 },
  {                +ES +ALTGR,  69,  48 },
  { +US+GB+DE+FR+IT+ES +EVERY, 112,  48 },
  { +US+GB+DE   +IT+ES +NOMOD,  22,  49 }, // 1
  {          +FR       +SHIFT,  22,  49 },
  { +US+GB+DE+FR+IT+ES +EVERY, 105,  49 },
  { +US+GB+DE   +IT+ES +NOMOD,  30,  50 }, // 2
  {          +FR       +SHIFT,  30,  50 },
  { +US+GB+DE+FR+IT+ES +EVERY, 114,  50 },
  { +US+GB+DE   +IT+ES +NOMOD,  38,  51 }, // 3
  {          +FR       +SHIFT,  38,  51 },
  { +US+GB+DE+FR+IT+ES +EVERY, 122,  51 },
  { +US+GB+DE   +IT+ES +NOMOD,  37,  52 }, // 4
  {          +FR       +SHIFT,  37,  52 },
  { +US+GB+DE+FR+IT+ES +EVERY, 107,  52 },
  { +US+GB+DE   +IT+ES +NOMOD,  46,  53 }, // 5
  {          +FR       +SHIFT,  46,  53 },
  {                +ES +ALTGR,  46,  53 },
  { +US+GB+DE+FR+IT+ES +EVERY, 115,  53 },
  { +US+GB+DE   +IT+ES +NOMOD,  54,  54 }, // 6
  {          +FR       +SHIFT,  54,  54 },
  { +US+GB+DE+FR+IT+ES +EVERY, 116,  54 },
  { +US+GB+DE   +IT+ES +NOMOD,  61,  55 }, // 7
  {          +FR       +SHIFT,  61,  55 },
  {                +ES +ALTGR,  61,  55 },
  { +US+GB+DE+FR+IT+ES +EVERY, 108,  55 },
  { +US+GB+DE   +IT+ES +NOMOD,  62,  56 }, // 8
  {          +FR       +SHIFT,  62,  56 },
  {                +ES +ALTGR,  62,  56 },
  { +US+GB+DE+FR+IT+ES +EVERY, 117,  56 },
  { +US+GB+DE   +IT+ES +NOMOD,  70,  57 }, // 9
  {          +FR       +SHIFT,  70,  57 },
  {                +ES +ALTGR,  70,  57 },
  { +US+GB+DE+FR+IT+ES +EVERY, 125,  57 },
  {          +FR       +NOMOD,  73,  58 }, // :
  {       +DE   +IT+ES +SHIFT,  73,  58 },
  { +US+GB             +SHIFT,  76,  58 },
  {          +FR       +NOMOD,  65,  59 }, // ;
  {       +DE   +IT+ES +SHIFT,  65,  59 },
  { +US+GB             +NOMOD,  76,  59 },
  { +US+GB             +SHIFT,  65,  60 }, // <
  {       +DE+FR+IT+ES +NOMOD,  97,  60 },
  {       +DE   +IT+ES +SHIFT,  69,  61 }, // =
  { +US+GB   +FR       +NOMOD,  85,  61 },
  { +US+GB             +SHIFT,  73,  62 }, // >
  {       +DE+FR+IT+ES +SHIFT,  97,  62 },
  {          +FR       +SHIFT,  58,  63 }, // ?
  { +US+GB             +SHIFT,  74,  63 },
  {       +DE   +IT+ES +SHIFT,  78,  63 },
  {       +DE+FR       +ALTGR,  21,  64 }, // @
  { +US                +SHIFT,  30,  64 },
  {                +ES +ALTGR,  30,  64 },
  {          +FR       +ALTGR,  69,  64 },
  {             +IT    +ALTGR,  76,  64 },
  {    +GB             +SHIFT,  82,  64 },
  {          +FR       +SHIFT,  21,  65 }, // A
  { +US+GB+DE   +IT+ES +SHIFT,  28,  65 },
  { +US+GB+DE+FR+IT+ES +SHIFT,  50,  66 }, // B
  { +US+GB+DE+FR+IT+ES +SHIFT,  33,  67 }, // C
  { +US+GB+DE+FR+IT+ES +SHIFT,  35,  68 }, // D
  { +US+GB+DE+FR+IT+ES +SHIFT,  36,  69 }, // E
  { +US+GB+DE+FR+IT+ES +SHIFT,  43,  70 }, // F
  { +US+GB+DE+FR+IT+ES +SHIFT,  52,  71 }, // G
  { +US+GB+DE+FR+IT+ES +SHIFT,  51,  72 }, // H
  { +US+GB+DE+FR+IT+ES +SHIFT,  67,  73 }, // I
  { +US+GB+DE+FR+IT+ES +SHIFT,  59,  74 }, // J
  { +US+GB+DE+FR+IT+ES +SHIFT,  66,  75 }, // K
  { +US+GB+DE+FR+IT+ES +SHIFT,  75,  76 }, // L
  { +US+GB+DE   +IT+ES +SHIFT,  58,  77 }, // M
  {          +FR       +SHIFT,  76,  77 },
  { +US+GB+DE+FR+IT+ES +SHIFT,  49,  78 }, // N
  { +US+GB+DE+FR+IT+ES +SHIFT,  68,  79 }, // O
  { +US+GB+DE+FR+IT+ES +SHIFT,  77,  80 }, // P
  { +US+GB+DE   +IT+ES +SHIFT,  21,  81 }, // Q
  {          +FR       +SHIFT,  28,  81 },
  { +US+GB+DE+FR+IT+ES +SHIFT,  45,  82 }, // R
  { +US+GB+DE+FR+IT+ES +SHIFT,  27,  83 }, // S
  { +US+GB+DE+FR+IT+ES +SHIFT,  44,  84 }, // T
  { +US+GB+DE+FR+IT+ES +SHIFT,  60,  85 }, // U
  { +US+GB+DE+FR+IT+ES +SHIFT,  42,  86 }, // V
  {          +FR       +SHIFT,  26,  87 }, // W
  { +US+GB+DE   +IT+ES +SHIFT,  29,  87 },
  { +US+GB+DE+FR+IT+ES +SHIFT,  34,  88 }, // X
  {       +DE          +SHIFT,  26,  89 }, // Y
  { +US+GB   +FR+IT+ES +SHIFT,  53,  89 },
  { +US+GB      +IT+ES +SHIFT,  26,  90 }, // Z
  {          +FR       +SHIFT,  29,  90 },
  {       +DE          +SHIFT,  53,  90 },
  {          +FR       +ALTGR,  46,  91 }, // [
  {       +DE   +IT    +ALTGR,  62,  91 },
  { +US+GB             +NOMOD,  84,  91 },
  {             +IT+ES +ALTGR,  84,  91 },
  {             +IT    +NOMOD,  14,  92 }, // \
  {                +ES +ALTGR,  14,  92 },
  {          +FR       +ALTGR,  62,  92 },
  {       +DE          +ALTGR,  78,  92 },
  { +US                +NOMOD,  93,  92 },
  {    +GB             +NOMOD,  97,  92 },
  {       +DE   +IT    +ALTGR,  70,  93 }, // ]
  {          +FR       +ALTGR,  78,  93 },
  { +US+GB             +NOMOD,  91,  93 },
  {             +IT+ES +ALTGR,  91,  93 },
  {       +DE          +NOMOD,  14,  94 }, // ^
  { +US+GB             +SHIFT,  54,  94 },
  {          +FR       +ALTGR,  70,  94 },
  {          +FR       +NOMOD,  84,  94 },
  {                +ES +SHIFT,  84,  94 },
  {             +IT    +SHIFT,  85,  94 },
  {          +FR       +NOMOD,  62,  95 }, // _
  {       +DE   +IT+ES +SHIFT,  74,  95 },
  { +US+GB             +SHIFT,  78,  95 },
  { +US+GB             +NOMOD,  14,  96 }, // `
  {          +FR       +ALTGR,  61,  96 },
  {             +IT    +ALTGR,  78,  96 },
  {                +ES +NOMOD,  84,  96 },
  {       +DE          +SHIFT,  85,  96 },
  {          +FR       +NOMOD,  21,  97 }, // a
  { +US+GB+DE   +IT+ES +NOMOD,  28,  97 },
  { +US+GB+DE+FR+IT+ES +NOMOD,  50,  98 }, // b
  { +US+GB+DE+FR+IT+ES +NOMOD,  33,  99 }, // c
  { +US+GB+DE+FR+IT+ES +NOMOD,  35, 100 }, // d
  { +US+GB+DE+FR+IT+ES +NOMOD,  36, 101 }, // e
  { +US+GB+DE+FR+IT+ES +NOMOD,  43, 102 }, // f
  { +US+GB+DE+FR+IT+ES +NOMOD,  52, 103 }, // g
  { +US+GB+DE+FR+IT+ES +NOMOD,  51, 104 }, // h
  { +US+GB+DE+FR+IT+ES +NOMOD,  67, 105 }, // i
  { +US+GB+DE+FR+IT+ES +NOMOD,  59, 106 }, // j
  { +US+GB+DE+FR+IT+ES +NOMOD,  66, 107 }, // k
  { +US+GB+DE+FR+IT+ES +NOMOD,  75, 108 }, // l
  { +US+GB+DE   +IT+ES +NOMOD,  58, 109 }, // m
  {          +FR       +NOMOD,  76, 109 },
  { +US+GB+DE+FR+IT+ES +NOMOD,  49, 110 }, // n
  { +US+GB+DE+FR+IT+ES +NOMOD,  68, 111 }, // o
  { +US+GB+DE+FR+IT+ES +NOMOD,  77, 112 }, // p
  { +US+GB+DE   +IT+ES +NOMOD,  21, 113 }, // q
  {          +FR       +NOMOD,  28, 113 },
  { +US+GB+DE+FR+IT+ES +NOMOD,  45, 114 }, // r
  { +US+GB+DE+FR+IT+ES +NOMOD,  27, 115 }, // s
  { +US+GB+DE+FR+IT+ES +NOMOD,  44, 116 }, // t
  { +US+GB+DE+FR+IT+ES +NOMOD,  60, 117 }, // u
  { +US+GB+DE+FR+IT+ES +NOMOD,  42, 118 }, // v
  {          +FR       +NOMOD,  26, 119 }, // w
  { +US+GB+DE   +IT+ES +NOMOD,  29, 119 },
  { +US+GB+DE+FR+IT+ES +NOMOD,  34, 120 }, // x
  {       +DE          +NOMOD,  26, 121 }, // y
  { +US+GB   +FR+IT+ES +NOMOD,  53, 121 },
  { +US+GB      +IT+ES +NOMOD,  26, 122 }, // z
  {          +FR       +NOMOD,  29, 122 },
  {       +DE          +NOMOD,  53, 122 },
  {          +FR       +ALTGR,  37, 123 }, // {
  {       +DE   +IT    +ALTGR,  61, 123 },
  {                +ES +ALTGR,  82, 123 },
  { +US+GB             +SHIFT,  84, 123 },
  {             +IT    +SHIFT,  14, 124 }, // |
  {                +ES +ALTGR,  22, 124 },
  {          +FR       +ALTGR,  54, 124 },
  { +US                +SHIFT,  93, 124 },
  {    +GB             +SHIFT,  97, 124 },
  {       +DE+FR   +ES +ALTGR,  97, 124 },
  {       +DE   +IT    +ALTGR,  69, 125 }, // }
  {          +FR       +ALTGR,  86, 125 },
  { +US+GB             +SHIFT,  91, 125 },
  {                +ES +ALTGR,  93, 125 },
  { +US                +SHIFT,  14, 126 }, // ~
  {          +FR       +ALTGR,  30, 126 },
  {                +ES +ALTGR,  37, 126 },
  {             +IT    +ALTGR,  85, 126 },
  {       +DE          +ALTGR,  91, 126 },
  {    +GB             +SHIFT,  93, 126 },
  { +US+GB+DE+FR+IT+ES +EVERY, 102, 127 },
  { +US+GB+DE+FR+IT+ES +EVERY,   5, 193 },
  { +US+GB+DE+FR+IT+ES +EVERY,   6, 194 },
  { +US+GB+DE+FR+IT+ES +EVERY,   4, 195 },
  { +US+GB+DE+FR+IT+ES +EVERY,  12, 196 },
  { +US+GB+DE+FR+IT+ES +EVERY,   3, 197 },
  { +US+GB+DE+FR+IT+ES +EVERY,  11, 198 },
  { +US+GB+DE+FR+IT+ES +EVERY, 131, 199 },
  { +US+GB+DE+FR+IT+ES +EVERY,  10, 200 },
  { +US+GB+DE+FR+IT+ES +EVERY,   1, 201 },
  { +US+GB+DE+FR+IT+ES +EVERY,   9, 202 },
  { +US+GB+DE+FR+IT+ES +EVERY, 120, 203 },
  { +US+GB+DE+FR+IT+ES +EVERY,   7, 204 },
};

/*
 *  Find ASCII code corresponding to key code and active modifiers
 *  for the current keymap
 */
static inline byte lookup(byte mods, word code)
{
  byte keymapFlag = 1 << getKeymapIndex();

  for (int i=0; i<arrayLen(keymaps); i++) {
    keyTuple_t *t = &keymaps[i];

    byte f = pgm_read_byte(&t->flags);
    if (~f & keymapFlag)
      continue; // Different keymap

    if ((f & EVERY) != EVERY && (f & EVERY) != mods)
      continue; // Different modifiers

    byte c = pgm_read_byte(&t->code);
    if (c != code)
      continue; // Different scan code

    return pgm_read_byte(&t->ascii);
  }
  return 0;
}

/*
 * Output state
 */
enum {
  // bit 0:7   Equivalent game controller buttons (positive)
  // buttonRight .. buttonA
  // bit 8:13  State of modifier keys
  leftShiftFlag  = 1<<8,
  rightShiftFlag = 1<<9,
  leftCtrlFlag   = 1<<10,
  rightCtrlFlag  = 1<<11,
  altFlag        = 1<<12,
  altGrFlag      = 1<<13,
  shiftFlags     = leftShiftFlag | rightShiftFlag,
  ctrlFlags      = leftCtrlFlag | rightCtrlFlag,
  altFlags       = altFlag | altGrFlag,
  // bit 14:15 Protocol state
  breakFlag      = 1<<14,       // Release of physical key
  extendedFlag   = 1<<15,       // Extended PS/2 keyboard codes
};

#define nextPs2()\
 flags &= ~(breakFlag | extendedFlag)

static word flags = 0;
static word ascii = 0;
static long lastChange;

// XXX Put variables in a class for better scoping

/*
 * PS/2 sequences
 */
static int bitBuffer;
static byte _n;
static long lastClock;

/*
 * Input buffering
 */
static volatile byte ps2Buffer[15+1];
static volatile byte head = 0, tail = 0;

#if defined(ARDUINO_attiny)
  // attachInterrupt() doesn't work on the ATtiny85
  static byte keyboardClockBit;
  ISR(PCINT0_vect)
  {
    if (~PINB & keyboardClockBit) // FALLING edge of PS/2 clock
      ps2interrupt();
  }
#endif

void keyboard_setup()
{
  // Initialize the pins
  #ifdef INPUT_PULLUP
    pinMode(keyboardDataPin, INPUT_PULLUP);
  #else
    pinMode(keyboardDataPin, INPUT);
    digitalWrite(keyboardDataPin, HIGH);
  #endif
  allowPs2();

  #if defined(ARDUINO_attiny)
    keyboardClockBit = digitalPinToBitMask(keyboardClockPin);
    GIMSK |= 1<<PCIE;          // Pin change interrupt enable
    PCMSK |= keyboardClockBit; // ... for keyboard clock
  #else
    attachInterrupt(
      digitalPinToInterrupt(keyboardClockPin),
      ps2interrupt, FALLING);
  #endif
}

// Handle one bit from PS/2 keyboard for the next byte
//  bit 0    : start bit (0)
//  bit 1..8 : data bits 0..7 (lowest bit first)
//  bit 9    : parity (odd)
//  bit 10   : stop bit (1)
void ps2interrupt()
{

  int nextBit = digitalRead(keyboardDataPin);

  long now = (long) millis();   // millis() stops when interrupts are disabled,
                                // but that is ok, because we will reset n
                                // after every vPulse (through allowPs2())

  // At least 10 kHz required for PS/2, self-reset after time out
  if (now - lastClock > 5 || _n >= 11) {
    bitBuffer = 1;              // Clear bits and setup for odd parity
    _n = 0;
  }
  lastClock = now;

  bitBuffer |= nextBit << _n;
  bitBuffer ^= nextBit;         // Track parity in bit 0

  if (_n == 10) {
    ps2Buffer[head] = (bitBuffer >> 1) & 255;

    byte nextHead = (head + 1) % sizeof ps2Buffer;
    if (nextHead != tail        // Buffer not (almost) full
     && (bitBuffer & 1))        // Parity check, with bonus check of start/stop bits
      head = nextHead;
  }
  _n++;
}

// Ready to receive
void allowPs2()
{
  #ifdef INPUT_PULLUP
    pinMode(keyboardClockPin, INPUT_PULLUP);
  #else
    pinMode(keyboardClockPin, INPUT);
    digitalWrite(keyboardClockPin, HIGH);
  #endif

  _n = 11;                       // Consider previous bits lost
}

// Pull clock line LOW to signal that we're not ready to receive
void forbidPs2()
{
  digitalWrite(keyboardClockPin, LOW);
  pinMode(keyboardClockPin, OUTPUT);
}

// Read next byte from incoming PS/2 buffer, or 0 when empty
static byte readPs2Buffer()
{
  byte value = 0;
  if (head != tail) {
    value = ps2Buffer[tail];
    tail = (tail + 1) % sizeof ps2Buffer;
  }
  return value;
}

byte keyboard_getState()
{
  long now = (long) millis();           // Note: without interrupts millis() stops counting
  for (;;) {
    word value = readPs2Buffer();
    if (value == 0) {                   // Buffer is empty
      long hold = 35;                   // Hold ASCII keys for 3 frames (less than repetition rate)
      if (ascii == (255^buttonStart))   // [Ctrl-Alt-Del] maps to [Start] for 2 seconds
        hold = 2500;
      if (now - lastChange > hold) 
        ascii = 0;
      if (now - lastChange > 1000)      // Hold buttons longer than the repeat delay
        flags &= ~255;
      break;
    }

    switch (value) {
      // The BREAK or EXTENDED bytes are part of a longer sequence
      // Here we don't mind the order they appear in
      case 0xe0: flags |= extendedFlag; continue;
      case 0xf0: flags |= breakFlag;    continue;
    }

    if (flags & extendedFlag)
      value |= 0xe0 << 8; // Simplify our switch statements

    // Recognize modifier keys
    word modifier = 0;
    switch (value) {
      case 0x11:   modifier = altFlag;        break;
      case 0x12:   modifier = leftShiftFlag;  break;
      case 0x14:   modifier = leftCtrlFlag;   break;
      case 0x59:   modifier = rightShiftFlag; break;
      case 0xe011: modifier = altGrFlag;      break;
      case 0xe014: modifier = rightCtrlFlag;  break;
    }

    // Case 1: Handle modifier keys
    if (modifier) {
      if (flags & breakFlag)
        flags &= ~modifier;     // Modifier key released
      else
        flags |= modifier;      // Modifier key pressed
      nextPs2();                // Reset PS/2 sequence
      continue;                 // Back to reading from buffer
    }

    // Recognize character and control keys and map them to ASCII
    // or game controller buttons
    // Always return an updated state to the caller from this point on,
    // even if there are still bytes waiting in the buffer.
    byte button = 0;
    byte newAscii = 0;
    lastChange = now;

    switch (value) {
      // Extended codes in "Set 2"
      case 0xe075: button = buttonUp;     break; // [UpArrow]
      case 0xe06b: button = buttonLeft;   break; // [LeftArrow]
      case 0xe072: button = buttonDown;   break; // [DownArrow]
      case 0xe074: button = buttonRight;  break; // [RightArrow]
      case 0x66:                                 // [BackSpace]
      case 0xe071: if ((flags & ctrlFlags)       // [Delete]
                    && (flags & altFlags))
                     newAscii = 255^buttonStart; // [Ctrl-Alt-Del] for special reset
                   else
                     button = buttonA;    break; // ASCII DEL is 127 is ~buttonA
      case 0xe069: button = buttonA;      break; // [End]
      case 0xe070:                               // [Insert]
      case 0xe06c: button = buttonB;      break; // [Home]
      case 0xe07a: button = buttonSelect; break; // [PageDown]
      case 0xe07d: button = buttonStart;  break; // [PageUp]
      case 0xe04a: newAscii = '/';        break; // [/] on numeric island
      case 0xe05a: newAscii = '\n';       break; // [Enter] on numeric island
      default:
        // Apply keyboard mapping to ASCII
        if ((flags & altGrFlag) ||
           ((flags & ctrlFlags) && (flags & altFlag))) // Ctrl+Alt is AltGr
          newAscii = lookup(ALTGR, value);
        else if (flags & shiftFlags)
          newAscii = lookup(SHIFT, value);
        else
          newAscii = lookup(NOMOD, value);

        // Handle control key combinations
        if (flags & ctrlFlags)
          switch (newAscii) {
            case PS2_ESC: newAscii = 0xc0^64; break; // Ctrl-Escape is a BabelFish command
            case '?':     newAscii = 127;     break; // Traditional mapping of Ctrl-? to DEL
            case ' ':     button   = 255;     break; // Make it send a 0 byte
            default:
              if (fnKey(newAscii)) // Ctrl+Fxx are BabelFish commands
                newAscii ^= 64;
              else
                newAscii &= 31;    // Make control codes (what the key is for...)
          }
    }

    if (button) {               // Case 2: Simulated game controller buttons
      if (flags & breakFlag)
        flags &= ~button;       // Button released
      else
        flags |= button;        // Button pressed
      ascii = 0;
    } else {                    // Case 3: ASCII keys
      if (~flags & breakFlag)
        ascii = newAscii;       // ASCII pressed
      else
        ;                       // ASCII release (ignored)
      flags &= ~255;
    }
    nextPs2();                  // Reset PS/2 sequence
    break;                      // Always return, leave anything in buffer behind
  }

  // Return the newly determined state to caller
  return ascii ? ascii : (255 & ~flags);
}

// Return 0 if not a function key, or its ordinal otherwise
byte fnKey(byte key)
{
  return (PS2_F1 <= key && key <= PS2_F12) ? (key & 15) : 0;
}

