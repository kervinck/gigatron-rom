
// PS/2 keyboard handling for Babelfish talking to Gigatron
// (Almost) Complete rewrite of PS2Keyboard

// References:
//
// https://www.avrfreaks.net/sites/default/files/PS2%20Keyboard.pdf
//  The PS/2 Mouse/Keyboard Protocol
//
// http://www.quadibloc.com/comp/scan.htm
//  Scan Codes Demytified
//
// https://retired.beyondlogic.org/keyboard/keybrd.htm
//  Interfacing the AT keyboard
//
// https://web.archive.org/web/20180101224739/http://www.computer-engineering.org/ps2keyboard/scancodes2.html
//  Keyboard Scan Codes: Set 2
///
// https://github.com/PaulStoffregen/PS2Keyboard
//    PS/2 Keyboard Library for Arduino
//
// https://github.com/techpaul/PS2KeyAdvanced
//  Arduino PS2 Keyboard FULL keyboard protocol support and full keys to integer coding
//
// https://github.com/techpaul/PS2KeyMap/
//  Arduino PS2 keyboard International Keyboard mapping from PS2KeyAdvanced and return as UTF-8

// TODO
// [Basic features]
// XXX Switching between keymaps
// XXX Simplify keymap tables (and check licensing)
// XXX Test keymap for DE
// XXX Test keymap for FR
// XXX Test keymap for UK
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
#define PS2_SCROLL     0

#define ps2keymapSize  136

typedef struct keymap {
  byte noshift[ps2keymapSize];
  byte shift[ps2keymapSize];
  byte hasAltGr;
  byte altgr[ps2keymapSize];
} PS2Keymap_t;

static inline byte lookup(byte submap[], word value)
{
  return (value < ps2keymapSize)
    ? pgm_read_byte(submap + value)
    : 0;
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
static const PS2Keymap_t *_keymap;

/*
 * Mapping
 */
extern const PROGMEM PS2Keymap_t PS2Keymap_US;
extern const PROGMEM PS2Keymap_t PS2Keymap_German;
extern const PROGMEM PS2Keymap_t PS2Keymap_French;

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
  _keymap = &PS2Keymap_US;

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
        if ((flags & altGrFlag) && _keymap->hasAltGr)
          newAscii = lookup(_keymap->altgr, value);
        else if (flags & shiftFlags)
          newAscii = lookup(_keymap->shift, value);
        else
          newAscii = lookup(_keymap->noshift, value);

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

// XXX Remove all of this and get from elsewhere
#define PS2_INVERTED_EXCLAMATION        161 // ¡
#define PS2_CENT_SIGN                   162 // ¢
#define PS2_POUND_SIGN                  163 // £
#define PS2_CURRENCY_SIGN               164 // ¤
#define PS2_YEN_SIGN                    165 // ¥
#define PS2_BROKEN_BAR                  166 // ¦
#define PS2_SECTION_SIGN                167 // §
#define PS2_DIAERESIS                   168 // ¨
#define PS2_COPYRIGHT_SIGN              169 // ©
#define PS2_FEMININE_ORDINAL            170 // ª
#define PS2_LEFT_DOUBLE_ANGLE_QUOTE     171 // «
#define PS2_NOT_SIGN                    172 // ¬
#define PS2_HYPHEN                      173
#define PS2_REGISTERED_SIGN             174 // ®
#define PS2_MACRON                      175 // ¯
#define PS2_DEGREE_SIGN                 176 // °
#define PS2_PLUS_MINUS_SIGN             177 // ±
#define PS2_SUPERSCRIPT_TWO             178 // ²
#define PS2_SUPERSCRIPT_THREE           179 // ³
#define PS2_ACUTE_ACCENT                180 // ´
#define PS2_MICRO_SIGN                  181 // µ
#define PS2_PILCROW_SIGN                182 // ¶
#define PS2_MIDDLE_DOT                  183 // ·
#define PS2_CEDILLA                     184 // ¸
#define PS2_SUPERSCRIPT_ONE             185 // ¹
#define PS2_MASCULINE_ORDINAL           186 // º
#define PS2_RIGHT_DOUBLE_ANGLE_QUOTE    187 // »
#define PS2_FRACTION_ONE_QUARTER        188 // ¼
#define PS2_FRACTION_ONE_HALF           189 // ½
#define PS2_FRACTION_THREE_QUARTERS     190 // ¾
#define PS2_INVERTED_QUESTION MARK      191 // ¿
#define PS2_A_GRAVE                     192 // À
#define PS2_A_ACUTE                     193 // Á
#define PS2_A_CIRCUMFLEX                194 // Â
#define PS2_A_TILDE                     195 // Ã
#define PS2_A_DIAERESIS                 196 // Ä
#define PS2_A_RING_ABOVE                197 // Å
#define PS2_AE                          198 // Æ
#define PS2_C_CEDILLA                   199 // Ç
#define PS2_E_GRAVE                     200 // È
#define PS2_E_ACUTE                     201 // É
#define PS2_E_CIRCUMFLEX                202 // Ê
#define PS2_E_DIAERESIS                 203 // Ë
#define PS2_I_GRAVE                     204 // Ì
#define PS2_I_ACUTE                     205 // Í
#define PS2_I_CIRCUMFLEX                206 // Î
#define PS2_I_DIAERESIS                 207 // Ï
#define PS2_ETH                         208 // Ð
#define PS2_N_TILDE                     209 // Ñ
#define PS2_O_GRAVE                     210 // Ò
#define PS2_O_ACUTE                     211 // Ó
#define PS2_O_CIRCUMFLEX                212 // Ô
#define PS2_O_TILDE                     213 // Õ
#define PS2_O_DIAERESIS                 214 // Ö
#define PS2_MULTIPLICATION              215 // ×
#define PS2_O_STROKE                    216 // Ø
#define PS2_U_GRAVE                     217 // Ù
#define PS2_U_ACUTE                     218 // Ú
#define PS2_U_CIRCUMFLEX                219 // Û
#define PS2_U_DIAERESIS                 220 // Ü
#define PS2_Y_ACUTE                     221 // Ý
#define PS2_THORN                       222 // Þ
#define PS2_SHARP_S                     223 // ß
#define PS2_a_GRAVE                     224 // à
#define PS2_a_ACUTE                     225 // á
#define PS2_a_CIRCUMFLEX                226 // â
#define PS2_a_TILDE                     227 // ã
#define PS2_a_DIAERESIS                 228 // ä
#define PS2_a_RING_ABOVE                229 // å
#define PS2_ae                          230 // æ
#define PS2_c_CEDILLA                   231 // ç
#define PS2_e_GRAVE                     232 // è
#define PS2_e_ACUTE                     233 // é
#define PS2_e_CIRCUMFLEX                234 // ê
#define PS2_e_DIAERESIS                 235 // ë
#define PS2_i_GRAVE                     236 // ì
#define PS2_i_ACUTE                     237 // í
#define PS2_i_CIRCUMFLEX                238 // î
#define PS2_i_DIAERESIS                 239 // ï
#define PS2_eth                         240 // ð
#define PS2_n_TILDE                     241 // ñ
#define PS2_o_GRAVE                     242 // ò
#define PS2_o_ACUTE                     243 // ó
#define PS2_o_CIRCUMFLEX                244 // ô
#define PS2_o_TILDE                     245 // õ
#define PS2_o_DIAERESIS                 246 // ö
#define PS2_DIVISION                    247 // ÷
#define PS2_o_STROKE                    248 // ø
#define PS2_u_GRAVE                     249 // ù
#define PS2_u_ACUTE                     250 // ú
#define PS2_u_CIRCUMFLEX                251 // û
#define PS2_u_DIAERESIS                 252 // ü
#define PS2_y_ACUTE                     253 // ý
#define PS2_thorn                       254 // þ
#define PS2_y_DIAERESIS                 255 // ÿ

const PROGMEM PS2Keymap_t PS2Keymap_US = {
  // without shift
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
  0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, '`', 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'q', '1', 0,
  0, 0, 'z', 's', 'a', 'w', '2', 0,
  0, 'c', 'x', 'd', 'e', '4', '3', 0,
  0, ' ', 'v', 'f', 't', 'r', '5', 0,
  0, 'n', 'b', 'h', 'g', 'y', '6', 0,
  0, 0, 'm', 'j', 'u', '7', '8', 0,
  0, ',', 'k', 'i', 'o', '0', '9', 0,
  0, '.', '/', 'l', ';', 'p', '-', 0,
  0, 0, '\'', 0, '[', '=', 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, ']', 0, '\\', 0, 0,
  0, 0, 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 },
  // with shift
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
  0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, '~', 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'Q', '!', 0,
  0, 0, 'Z', 'S', 'A', 'W', '@', 0,
  0, 'C', 'X', 'D', 'E', '$', '#', 0,
  0, ' ', 'V', 'F', 'T', 'R', '%', 0,
  0, 'N', 'B', 'H', 'G', 'Y', '^', 0,
  0, 0, 'M', 'J', 'U', '&', '*', 0,
  0, '<', 'K', 'I', 'O', ')', '(', 0,
  0, '>', '?', 'L', ':', 'P', '_', 0,
  0, 0, '"', 0, '{', '+', 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '}', 0, '|', 0, 0,
  0, 0, 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 },
  0
};

const PROGMEM PS2Keymap_t PS2Keymap_German = {
  // without shift
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
  0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, '^', 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'q', '1', 0,
  0, 0, 'y', 's', 'a', 'w', '2', 0,
  0, 'c', 'x', 'd', 'e', '4', '3', 0,
  0, ' ', 'v', 'f', 't', 'r', '5', 0,
  0, 'n', 'b', 'h', 'g', 'z', '6', 0,
  0, 0, 'm', 'j', 'u', '7', '8', 0,
  0, ',', 'k', 'i', 'o', '0', '9', 0,
  0, '.', '-', 'l', PS2_o_DIAERESIS, 'p', PS2_SHARP_S, 0,
  0, 0, PS2_a_DIAERESIS, 0, PS2_u_DIAERESIS, '\'', 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '+', 0, '#', 0, 0,
  0, '<', 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 },
  // with shift
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
  0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, PS2_DEGREE_SIGN, 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'Q', '!', 0,
  0, 0, 'Y', 'S', 'A', 'W', '"', 0,
  0, 'C', 'X', 'D', 'E', '$', PS2_SECTION_SIGN, 0,
  0, ' ', 'V', 'F', 'T', 'R', '%', 0,
  0, 'N', 'B', 'H', 'G', 'Z', '&', 0,
  0, 0, 'M', 'J', 'U', '/', '(', 0,
  0, ';', 'K', 'I', 'O', '=', ')', 0,
  0, ':', '_', 'L', PS2_O_DIAERESIS, 'P', '?', 0,
  0, 0, PS2_A_DIAERESIS, 0, PS2_U_DIAERESIS, '`', 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '*', 0, '\'', 0, 0,
  0, '>', 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 },
  1,
  // with altgr
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
  0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, 0, 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, '@', 0, 0,
  0, 0, 0, 0, 0, 0, PS2_SUPERSCRIPT_TWO, 0,
  0, 0, 0, 0, PS2_CURRENCY_SIGN, 0, PS2_SUPERSCRIPT_THREE, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, PS2_MICRO_SIGN, 0, 0, '{', '[', 0,
  0, 0, 0, 0, 0, '}', ']', 0,
  0, 0, 0, 0, 0, 0, '\\', 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '~', 0, '#', 0, 0,
  0, '|', 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 }
};

const PROGMEM PS2Keymap_t PS2Keymap_French = {
  // without shift
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
  0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, PS2_SUPERSCRIPT_TWO, 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'a', '&', 0,
  0, 0, 'w', 's', 'q', 'z', PS2_e_ACUTE, 0,
  0, 'c', 'x', 'd', 'e', '\'', '"', 0,
  0, ' ', 'v', 'f', 't', 'r', '(', 0,
  0, 'n', 'b', 'h', 'g', 'y', '-', 0,
  0, 0, ',', 'j', 'u', PS2_e_GRAVE, '_', 0,
  0, ';', 'k', 'i', 'o', PS2_a_GRAVE, PS2_c_CEDILLA, 0,
  0, ':', '!', 'l', 'm', 'p', ')', 0,
  0, 0, PS2_u_GRAVE, 0, '^', '=', 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '$', 0, '*', 0, 0,
  0, '<', 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 },
  // with shift
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
  0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, 0, 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'A', '1', 0,
  0, 0, 'W', 'S', 'Q', 'Z', '2', 0,
  0, 'C', 'X', 'D', 'E', '4', '3', 0,
  0, ' ', 'V', 'F', 'T', 'R', '5', 0,
  0, 'N', 'B', 'H', 'G', 'Y', '6', 0,
  0, 0, '?', 'J', 'U', '7', '8', 0,
  0, '.', 'K', 'I', 'O', '0', '9', 0,
  0, '/', PS2_SECTION_SIGN, 'L', 'M', 'P', PS2_DEGREE_SIGN, 0,
  0, 0, '%', 0, PS2_DIAERESIS, '+', 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, PS2_POUND_SIGN, 0, PS2_MICRO_SIGN, 0, 0,
  0, '>', 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 },
  1,
  // with altgr
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
  0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, 0, 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, '@', 0, 0,
  0, 0, 0, 0, 0, 0, '~', 0,
  0, 0, 0, 0, 0 /*PS2_EURO_SIGN*/, '{', '#', 0,
  0, 0, 0, 0, 0, 0, '[', 0,
  0, 0, 0, 0, 0, 0, '|', 0,
  0, 0, 0, 0, 0, '`', '\\', 0,
  0, 0, 0, 0, 0, '@', '^', 0,
  0, 0, 0, 0, 0, 0, ']', 0,
  0, 0, 0, 0, 0, 0, '}', 0,
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '¤', 0, '#', 0, 0,
  0, '|', 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 }
};

