
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
// XXX Test keymap for DE
// XXX Test keymap for FR
// XXX Test keymap for GB
// XXX Test keymap for ES
// XXX Issue: Sometimes keyboard starts injecting break codes
//     when pressing [buttonA] + arrow keys, making it impossible
//     to set the time in Mandelbrot??
// XXX (Left)Alt for buttonB?
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

#define CTRLALTDEL     (255 ^ buttonStart)

/*
 *  Keyboard layout mapping
 */

const char keymapNames[][3] = {
  "US", "GB", "DE", "FR", "IT", "ES",
};
const byte nrKeymaps = arrayLen(keymapNames);

int getKeymapIndex(void)
{
  byte index = EEPROM.read(offsetof(struct EEPROMlayout, keymapIndex));
  return (index >= nrKeymaps) ? 0 : index; // Also handle invalid values
}

const char *getKeymapName()
{
  return getKeymapName(getKeymapIndex());
}

const char *getKeymapName(byte index)
{
  return (index > arrayLen(keymapNames)) ? 0 : keymapNames[index];
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
// 229 * 3 bytes = 687 bytes for holding 6 keyboard layouts
// To save space this table excludes codes that aren't in US-ASCII
// because these aren't in the Gigatron font either (yet).
// So accented letters and such are all absent. This is needed
// to make it all fit in the ATtiny85 configuration.

const PROGMEM keyTuple_t keymaps[] = {
  { +US+GB+DE+FR+IT+ES +EVERY, 0x0d,   9 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x5a,  10 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x76,  27 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x29,  32 },
  { +US+GB+DE   +IT+ES +SHIFT, 0x16,  33 }, // '!'
  {          +FR       +NOMOD, 0x4a,  33 },
  {    +GB+DE   +IT+ES +SHIFT, 0x1e,  34 }, // '"'
  {          +FR       +NOMOD, 0x26,  34 },
  { +US                +SHIFT, 0x52,  34 },
  { +US                +SHIFT, 0x26,  35 }, // '#'
  {          +FR   +ES +ALTGR, 0x26,  35 },
  {             +IT    +ALTGR, 0x52,  35 },
  {    +GB+DE          +NOMOD, 0x5d,  35 },
  {       +DE+FR       +ALTGR, 0x5d,  35 },
  { +US+GB+DE   +IT+ES +SHIFT, 0x25,  36 }, // '$'
  {          +FR       +NOMOD, 0x5b,  36 },
  { +US+GB+DE   +IT+ES +SHIFT, 0x2e,  37 }, // '%'
  {          +FR       +SHIFT, 0x52,  37 },
  {          +FR       +NOMOD, 0x16,  38 }, // '&'
  {       +DE   +IT+ES +SHIFT, 0x36,  38 },
  { +US+GB             +SHIFT, 0x3d,  38 },
  {          +FR       +NOMOD, 0x25,  39 }, // '''
  {             +IT+ES +NOMOD, 0x4e,  39 },
  { +US+GB             +NOMOD, 0x52,  39 },
  {       +DE          +NOMOD, 0x55,  39 },
  {       +DE          +SHIFT, 0x5d,  39 },
  {          +FR       +NOMOD, 0x2e,  40 }, // '('
  {       +DE   +IT+ES +SHIFT, 0x3e,  40 },
  { +US+GB             +SHIFT, 0x46,  40 },
  { +US+GB             +SHIFT, 0x45,  41 }, // ')'
  {       +DE   +IT+ES +SHIFT, 0x46,  41 },
  {          +FR       +NOMOD, 0x4e,  41 },
  { +US+GB             +SHIFT, 0x3e,  42 }, // '*'
  {       +DE   +IT+ES +SHIFT, 0x5b,  42 },
  {          +FR       +NOMOD, 0x5d,  42 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x7c,  42 },
  { +US+GB   +FR       +SHIFT, 0x55,  43 }, // '+'
  {       +DE   +IT+ES +NOMOD, 0x5b,  43 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x79,  43 },
  {          +FR       +NOMOD, 0x3a,  44 }, // ','
  { +US+GB+DE   +IT+ES +NOMOD, 0x41,  44 },
  {          +FR       +NOMOD, 0x36,  45 }, // '-'
  {       +DE   +IT+ES +NOMOD, 0x4a,  45 },
  { +US+GB             +NOMOD, 0x4e,  45 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x7b,  45 },
  {          +FR       +SHIFT, 0x41,  46 }, // '.'
  { +US+GB+DE   +IT+ES +NOMOD, 0x49,  46 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x71,  46 },
  {       +DE   +IT+ES +SHIFT, 0x3d,  47 }, // '/'
  {          +FR       +SHIFT, 0x49,  47 },
  { +US+GB             +NOMOD, 0x4a,  47 },
  { +US+GB+DE   +IT+ES +NOMOD, 0x45,  48 }, // '0'
  {          +FR       +SHIFT, 0x45,  48 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x70,  48 },
  { +US+GB+DE   +IT+ES +NOMOD, 0x16,  49 }, // '1'
  {          +FR       +SHIFT, 0x16,  49 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x69,  49 },
  { +US+GB+DE   +IT+ES +NOMOD, 0x1e,  50 }, // '2'
  {          +FR       +SHIFT, 0x1e,  50 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x72,  50 },
  { +US+GB+DE   +IT+ES +NOMOD, 0x26,  51 }, // '3'
  {          +FR       +SHIFT, 0x26,  51 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x7a,  51 },
  { +US+GB+DE   +IT+ES +NOMOD, 0x25,  52 }, // '4'
  {          +FR       +SHIFT, 0x25,  52 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x6b,  52 },
  { +US+GB+DE   +IT+ES +NOMOD, 0x2e,  53 }, // '5'
  {          +FR       +SHIFT, 0x2e,  53 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x73,  53 },
  { +US+GB+DE   +IT+ES +NOMOD, 0x36,  54 }, // '6'
  {          +FR       +SHIFT, 0x36,  54 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x74,  54 },
  { +US+GB+DE   +IT+ES +NOMOD, 0x3d,  55 }, // '7'
  {          +FR       +SHIFT, 0x3d,  55 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x6c,  55 },
  { +US+GB+DE   +IT+ES +NOMOD, 0x3e,  56 }, // '8'
  {          +FR       +SHIFT, 0x3e,  56 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x75,  56 },
  { +US+GB+DE   +IT+ES +NOMOD, 0x46,  57 }, // '9'
  {          +FR       +SHIFT, 0x46,  57 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x7d,  57 },
  {          +FR       +NOMOD, 0x49,  58 }, // ':'
  {       +DE   +IT+ES +SHIFT, 0x49,  58 },
  { +US+GB             +SHIFT, 0x4c,  58 },
  {          +FR       +NOMOD, 0x41,  59 }, // ';'
  {       +DE   +IT+ES +SHIFT, 0x41,  59 },
  { +US+GB             +NOMOD, 0x4c,  59 },
  { +US+GB             +SHIFT, 0x41,  60 }, // '<'
  {       +DE+FR+IT+ES +NOMOD, 0x61,  60 },
  {       +DE   +IT+ES +SHIFT, 0x45,  61 }, // '='
  { +US+GB   +FR       +NOMOD, 0x55,  61 },
  { +US+GB             +SHIFT, 0x49,  62 }, // '>'
  {       +DE+FR+IT+ES +SHIFT, 0x61,  62 },
  {          +FR       +SHIFT, 0x3a,  63 }, // '?'
  { +US+GB             +SHIFT, 0x4a,  63 },
  {       +DE   +IT+ES +SHIFT, 0x4e,  63 },
  {       +DE+FR       +ALTGR, 0x15,  64 }, // '@'
  { +US                +SHIFT, 0x1e,  64 },
  {                +ES +ALTGR, 0x1e,  64 },
  {          +FR       +ALTGR, 0x45,  64 },
  {             +IT    +ALTGR, 0x4c,  64 },
  {    +GB             +SHIFT, 0x52,  64 },
  {          +FR       +SHIFT, 0x15,  65 }, // 'A'
  { +US+GB+DE   +IT+ES +SHIFT, 0x1c,  65 },
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x32,  66 }, // 'B'
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x21,  67 }, // 'C'
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x23,  68 }, // 'D'
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x24,  69 }, // 'E'
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x2b,  70 }, // 'F'
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x34,  71 }, // 'G'
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x33,  72 }, // 'H'
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x43,  73 }, // 'I'
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x3b,  74 }, // 'J'
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x42,  75 }, // 'K'
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x4b,  76 }, // 'L'
  { +US+GB+DE   +IT+ES +SHIFT, 0x3a,  77 }, // 'M'
  {          +FR       +SHIFT, 0x4c,  77 },
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x31,  78 }, // 'N'
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x44,  79 }, // 'O'
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x4d,  80 }, // 'P'
  { +US+GB+DE   +IT+ES +SHIFT, 0x15,  81 }, // 'Q'
  {          +FR       +SHIFT, 0x1c,  81 },
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x2d,  82 }, // 'R'
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x1b,  83 }, // 'S'
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x2c,  84 }, // 'T'
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x3c,  85 }, // 'U'
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x2a,  86 }, // 'V'
  {          +FR       +SHIFT, 0x1a,  87 }, // 'W'
  { +US+GB+DE   +IT+ES +SHIFT, 0x1d,  87 },
  { +US+GB+DE+FR+IT+ES +SHIFT, 0x22,  88 }, // 'X'
  {       +DE          +SHIFT, 0x1a,  89 }, // 'Y'
  { +US+GB   +FR+IT+ES +SHIFT, 0x35,  89 },
  { +US+GB      +IT+ES +SHIFT, 0x1a,  90 }, // 'Z'
  {          +FR       +SHIFT, 0x1d,  90 },
  {       +DE          +SHIFT, 0x35,  90 },
  {          +FR       +ALTGR, 0x2e,  91 }, // '['
  {       +DE   +IT    +ALTGR, 0x3e,  91 },
  { +US+GB             +NOMOD, 0x54,  91 },
  {             +IT+ES +ALTGR, 0x54,  91 },
  {             +IT    +NOMOD, 0x0e,  92 }, // '\'
  {                +ES +ALTGR, 0x0e,  92 },
  {          +FR       +ALTGR, 0x3e,  92 },
  {       +DE          +ALTGR, 0x4e,  92 },
  { +US                +NOMOD, 0x5d,  92 },
  {    +GB             +NOMOD, 0x61,  92 },
  {       +DE   +IT    +ALTGR, 0x46,  93 }, // ']'
  {          +FR       +ALTGR, 0x4e,  93 },
  { +US+GB             +NOMOD, 0x5b,  93 },
  {             +IT+ES +ALTGR, 0x5b,  93 },
  {       +DE          +NOMOD, 0x0e,  94 }, // '^'
  { +US+GB             +SHIFT, 0x36,  94 },
  {          +FR       +ALTGR, 0x46,  94 },
  {          +FR       +NOMOD, 0x54,  94 },
  {                +ES +SHIFT, 0x54,  94 },
  {             +IT    +SHIFT, 0x55,  94 },
  {          +FR       +NOMOD, 0x3e,  95 }, // '_'
  {       +DE   +IT+ES +SHIFT, 0x4a,  95 },
  { +US+GB             +SHIFT, 0x4e,  95 },
  { +US+GB             +NOMOD, 0x0e,  96 }, // '`'
  {          +FR       +ALTGR, 0x3d,  96 },
  {             +IT    +ALTGR, 0x4e,  96 },
  {                +ES +NOMOD, 0x54,  96 },
  {       +DE          +SHIFT, 0x55,  96 },
  {          +FR       +NOMOD, 0x15,  97 }, // 'a'
  { +US+GB+DE   +IT+ES +NOMOD, 0x1c,  97 },
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x32,  98 }, // 'b'
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x21,  99 }, // 'c'
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x23, 100 }, // 'd'
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x24, 101 }, // 'e'
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x2b, 102 }, // 'f'
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x34, 103 }, // 'g'
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x33, 104 }, // 'h'
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x43, 105 }, // 'i'
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x3b, 106 }, // 'j'
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x42, 107 }, // 'k'
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x4b, 108 }, // 'l'
  { +US+GB+DE   +IT+ES +NOMOD, 0x3a, 109 }, // 'm'
  {          +FR       +NOMOD, 0x4c, 109 },
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x31, 110 }, // 'n'
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x44, 111 }, // 'o'
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x4d, 112 }, // 'p'
  { +US+GB+DE   +IT+ES +NOMOD, 0x15, 113 }, // 'q'
  {          +FR       +NOMOD, 0x1c, 113 },
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x2d, 114 }, // 'r'
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x1b, 115 }, // 's'
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x2c, 116 }, // 't'
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x3c, 117 }, // 'u'
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x2a, 118 }, // 'v'
  {          +FR       +NOMOD, 0x1a, 119 }, // 'w'
  { +US+GB+DE   +IT+ES +NOMOD, 0x1d, 119 },
  { +US+GB+DE+FR+IT+ES +NOMOD, 0x22, 120 }, // 'x'
  {       +DE          +NOMOD, 0x1a, 121 }, // 'y'
  { +US+GB   +FR+IT+ES +NOMOD, 0x35, 121 },
  { +US+GB      +IT+ES +NOMOD, 0x1a, 122 }, // 'z'
  {          +FR       +NOMOD, 0x1d, 122 },
  {       +DE          +NOMOD, 0x35, 122 },
  {          +FR       +ALTGR, 0x25, 123 }, // '{'
  {       +DE   +IT    +ALTGR, 0x3d, 123 },
  {                +ES +ALTGR, 0x52, 123 },
  { +US+GB             +SHIFT, 0x54, 123 },
  {             +IT    +SHIFT, 0x0e, 124 }, // '|'
  {                +ES +ALTGR, 0x16, 124 },
  {          +FR       +ALTGR, 0x36, 124 },
  { +US                +SHIFT, 0x5d, 124 },
  {    +GB             +SHIFT, 0x61, 124 },
  {       +DE+FR   +ES +ALTGR, 0x61, 124 },
  {       +DE   +IT    +ALTGR, 0x45, 125 }, // '}'
  {          +FR       +ALTGR, 0x56, 125 },
  { +US+GB             +SHIFT, 0x5b, 125 },
  {                +ES +ALTGR, 0x5d, 125 },
  { +US                +SHIFT, 0x0e, 126 }, // '~'
  {          +FR       +ALTGR, 0x1e, 126 },
  {                +ES +ALTGR, 0x25, 126 },
  {             +IT    +ALTGR, 0x55, 126 },
  {       +DE          +ALTGR, 0x5b, 126 },
  {    +GB             +SHIFT, 0x5d, 126 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x66, 127 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x05, 193 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x06, 194 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x04, 195 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x0c, 196 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x03, 197 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x0b, 198 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x83, 199 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x0a, 200 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x01, 201 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x09, 202 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x78, 203 },
  { +US+GB+DE+FR+IT+ES +EVERY, 0x07, 204 },
};

/*
 *  Find ASCII code corresponding to key code and active modifiers
 *  for the current keymap
 */
static inline byte lookup(byte mods, word code)
{
  byte keymapFlag = 1 << getKeymapIndex();

  for (int i=0; i<arrayLen(keymaps); i++) {
    const keyTuple_t *t = &keymaps[i];

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

static word flags = 0;          // Modifier keys and button state
static byte ascii = 0;          // Current ASCII key press
static word lastChange, hold;   // Babelfish controls the hold time for keys

// XXX Put variables in a class for better scoping

/*
 * PS/2 sequences
 */
static int bitBuffer;
static byte _n;
static word lastClock;

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
  pinMode(keyboardDataPin, INPUT_PULLUP);
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
  byte nextBit = digitalRead(keyboardDataPin);

  word now = (word) millis();   // millis() stops when interrupts are disabled,
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
  pinMode(keyboardClockPin, INPUT_PULLUP);
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
  word now = (word) millis();           // Note: without interrupts millis() stops counting
  for (;;) {
    word value = readPs2Buffer();
    if (value == 0) {                   // Buffer is empty
      if (now - lastChange > hold)      // Auto-release keys after `hold' milliseconds
        ascii = 0;
      if (now - lastChange > 1000)      // Hold `buttons' a bit longer than the repeat delay. This
        flags &= ~255;                  // is merely a safeguard against missing the break event
                                        // and then ending up with a stuck virtual button press.
      break;
    }

    switch (value) {
      // The BREAK or EXTENDED bytes are part of a longer sequence
      // Here we don't mind the order they appear in
      case 0xe0: flags |= extendedFlag; continue;
      case 0xf0: flags |= breakFlag;    continue;
      case 0xaa: // BAT ok
        #define oddParity(x) ((1 ^ x ^ (x>>1) ^ (x>>2) ^ (x>>3) ^ (x>>4) ^ (x>>5) ^ (x>>6) ^ (x>>7)) & 1)
        #define code(x) ((1 << 10) | (oddParity(x) << 9) | ((x) << 1))
        keyboard_send(code(0xf4)); // Enable (fixes Walter's PERIBOARD-409)
        continue;
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

    switch (value) {
      // Extended codes in "Set 2"
      case 0xe075: button = buttonUp;     break; // [UpArrow]
      case 0xe06b: button = buttonLeft;   break; // [LeftArrow]
      case 0xe072: button = buttonDown;   break; // [DownArrow]
      case 0xe074: button = buttonRight;  break; // [RightArrow]
      case 0x66:                                 // [BackSpace]
      case 0xe071: if ((flags & ctrlFlags)       // [Delete]
                    && (flags & altFlags))
                     newAscii = CTRLALTDEL;      // [Ctrl-Alt-Del] becomes [Start]
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
            case '?':     newAscii = 127; break; // Traditional mapping of Ctrl-? to DEL
            case ' ':     button   = 255; break; // Make it send a 0 byte
            default:
              byte f = fnKey(newAscii);
              if (f) {
                if (flags & altFlags)
                  // Ctrl-Alt-Fxx changes the keymap
                  EEPROM.write(offsetof(struct EEPROMlayout, keymapIndex), f-1);
                else
                  // Ctrl+Fxx are BabelFish commands
                  newAscii ^= 64;
              } else
                // Make control codes (what the key is for...)
                newAscii &= 31;
          }
    }

    // Consolidate the `flags' and `ascii' states
    if (button) {               // Case 2: Simulated game controller buttons
      lastChange = now;
      if (flags & breakFlag)
        flags &= ~button;       // Button released
      else
        flags |= button;        // Button pressed
      ascii = 0;
    } else {                    // Case 3: ASCII keys
      if (newAscii != ascii)
        lastChange = now;
      if (~flags & breakFlag)   // Ignore ASCII release events (use 'hold' instead)
        ascii = newAscii;       // ASCII pressed
      hold = 35;                // Hold for 3 frames (less than the repeat rate)
      if (ascii == CTRLALTDEL)  // At least 2 seconds for the [Start] signal
        hold = max(500 + now - lastChange, 2500); // No accidental Easter Egg trigger (8b guy)
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

// 'code' must include start, data, parity and stop bits
void keyboard_send(word code)
{
  // 1. Bring the Clock line low for at least 100 microseconds
  forbidPs2();
  delayMicroseconds(100);

  // 2. Bring the data line low
  digitalWrite(keyboardDataPin, 0);
  pinMode(keyboardDataPin, OUTPUT);

  // 3. Release the Clock line
  allowPs2();
  // Pull-up resistor will bring clock line up first

  do {
    // 6. Wait for the device to bring Clock high
    while (digitalRead(keyboardClockPin) == 0)
      ;

    // 4. 7. Wait for the device to bring the Clock line low
    // MvK: Falling edge transfers the data
    while (digitalRead(keyboardClockPin) != 0)
      ;

    // 5. Set/reset the Data line to send the next data bit
    code >>= 1;
    if (code & 1)
      pinMode(keyboardDataPin, INPUT_PULLUP); // To send 1
    else {
      digitalWrite(keyboardDataPin, 0);
      pinMode(keyboardDataPin, OUTPUT); // To send 0
    }

    // 8. Repeat steps 5-7 for the other seven data bits and the parity bit
    // 9. Release the Data line
  } while (code > 1);

  pinMode(keyboardDataPin, INPUT_PULLUP);

  // 10. Wait for the device to bring Data low
  while (digitalRead(keyboardDataPin) != 0)
    ;

  // 11. Wait for the device to bring Clock low
  while (digitalRead(keyboardClockPin) != 0)
    ;

  // 12. Wait for the device to release Data and Clock
  while (digitalRead(keyboardClockPin) == 0 || digitalRead(keyboardDataPin) == 0)
    ;
}
