
/*
 *  Convert Arduino keymaps to more compact data
 *  structure for use in BabelFish on ATtiny85
 *
 *  Original encoding:
 *  6 * 409 bytes = 2454 bytes
 *  New encoding
 *  3 * 329 bytes = 658 bytes
 *
 *  Encoding is
 *  6 bits to indicate which subsets apply
 *  2 bits for shift/altGr
 *    00 no modifier
 *    01 modfier must be shift
 *    10 modifier must be AltGr
 *    11 ignore modifier
 *  8 bits keyboard code
 *    1..131
 *  8 bits ascii
 *    9,10,27,32-127,193-204
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define ps2keymapSize  132

typedef unsigned char  byte;
typedef unsigned short word;

typedef struct keymap {
  byte noshift[ps2keymapSize];
  byte shift[ps2keymapSize];
  byte hasAltGr;
  byte altgr[ps2keymapSize];
} PS2Keymap_t;

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
#define PS2_EURO_SIGN  0

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
#define PS2_INVERTED_QUESTION_MARK      191 // ¿
#define PS2_A_GRAVE                     192 // À
// These 12 conflict with the function key codes in Gigatron
#define PS2_A_ACUTE                0 // 193 // Á
#define PS2_A_CIRCUMFLEX           0 // 194 // Â
#define PS2_A_TILDE                0 // 195 // Ã
#define PS2_A_DIAERESIS            0 // 196 // Ä
#define PS2_A_RING_ABOVE           0 // 197 // Å
#define PS2_AE                     0 // 198 // Æ
#define PS2_C_CEDILLA              0 // 199 // Ç
#define PS2_E_GRAVE                0 // 200 // È
#define PS2_E_ACUTE                0 // 201 // É
#define PS2_E_CIRCUMFLEX           0 // 202 // Ê
#define PS2_E_DIAERESIS            0 // 203 // Ë
#define PS2_I_GRAVE                0 // 204 // Ì
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

// Keymaps courtesy of
//    Arduino PS2Keyboard library (US,DE,FR)
//    https://playground.arduino.cc/Main/PS2Keyboard
// and
//    Teensy PS2Keyboard library (GB,IT,ES)
//    http://www.pjrc.com/teensy/td_libs_PS2Keyboard.html
//
// Modifications made for Gigatron project

static const PS2Keymap_t keymaps[] = {
/* US */ {
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
},

/* UK */ {
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
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, ']', 0, '#', 0, 0,
  0, '\\', 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 },
  // with shift
  {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
  0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, 172 /* ¬ */, 0,
  0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'Q', '!', 0,
  0, 0, 'Z', 'S', 'A', 'W', '"', 0,
  0, 'C', 'X', 'D', 'E', '$', 163 /* £ */, 0,
  0, ' ', 'V', 'F', 'T', 'R', '%', 0,
  0, 'N', 'B', 'H', 'G', 'Y', '^', 0,
  0, 0, 'M', 'J', 'U', '&', '*', 0,
  0, '<', 'K', 'I', 'O', ')', '(', 0,
  0, '>', '?', 'L', ':', 'P', '_', 0,
  0, 0, '@', 0, '{', '+', 0, 0,
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '}', 0, '~', 0, 0,
  0, '|', 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 },
  0
},

/* DE */ {
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
},

/* FR */ {
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
  0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, /*'¤'*/0, 0, '#', 0, 0,
  0, '|', 0, 0, 0, 0, PS2_BACKSPACE, 0,
  0, '1', 0, '4', '7', 0, 0, 0,
  '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
  PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
  0, 0, 0, PS2_F7 }
},

/* IT */ {
    // without shift
    {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
     0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, '\\', 0,
     0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'q', '1', 0,
     0, 0, 'z', 's', 'a', 'w', '2', 0,
     0, 'c', 'x', 'd', 'e', '4', '3', 0,
     0, ' ', 'v', 'f', 't', 'r', '5', 0,
     0, 'n', 'b', 'h', 'g', 'y', '6', 0,
     0, 0, 'm', 'j', 'u', '7', '8', 0,
     0, ',', 'k', 'i', 'o', '0', '9', 0,
     0, '.', '-', 'l', PS2_o_GRAVE, 'p', '\'', 0,
     0, 0, PS2_a_GRAVE, 0, PS2_e_GRAVE, PS2_i_GRAVE, 0, 0,
     0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '+', 0, PS2_u_GRAVE, 0, 0,
     0, '<', 0, 0, 0, 0, PS2_BACKSPACE, 0,
     0, '1', 0, '4', '7', 0, 0, 0,
     '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
     PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
     0, 0, 0, PS2_F7 },
    // with shift
    {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
     0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, '|', 0,
     0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'Q', '!', 0,
     0, 0, 'Z', 'S', 'A', 'W', '\"', 0,
     0, 'C', 'X', 'D', 'E', '$', PS2_POUND_SIGN, 0,
     0, ' ', 'V', 'F', 'T', 'R', '%', 0,
     0, 'N', 'B', 'H', 'G', 'Y', '&', 0,
     0, 0, 'M', 'J', 'U', '/', '(', 0,
     0, ';', 'K', 'I', 'O', '=', ')', 0,
     0, ':', '_', 'L', PS2_c_CEDILLA, 'P', '?', 0,
     0, 0, PS2_DEGREE_SIGN, 0, PS2_e_ACUTE, '^', 0, 0,
     0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '*', 0, PS2_SECTION_SIGN, 0, 0,
     0, '>', 0, 0, 0, 0, PS2_BACKSPACE, 0,
     0, '1', 0, '4', '7', 0, 0, 0,
     '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
     PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
     0, 0, 0, PS2_F7 },
         1,
    // with altgr
    {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
     0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, PS2_NOT_SIGN, 0,
     0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 0, PS2_SUPERSCRIPT_ONE, 0,
     0, 0, 0, 0, 0, 0, PS2_SUPERSCRIPT_TWO, 0,
     0, 0, 0, 0, PS2_EURO_SIGN, PS2_FRACTION_ONE_QUARTER, PS2_SUPERSCRIPT_THREE, 0,
     0, ' ', 0, 0, 0, 0, PS2_FRACTION_ONE_HALF, 0,
     0, 0, 0, 0, 0, 0, PS2_NOT_SIGN, 0,
     0, 0, 0, 0, 0, '{', '[', 0,
     0, ',', 0, 0, 0, '}', ']', 0,
     0, '.', '-', 0, '@', 0, '`', 0,
     0, 0, '#', 0, '[', '~', 0, 0,
     0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, ']', 0, 0, 0, 0,
     0, PS2_LEFT_DOUBLE_ANGLE_QUOTE, 0, 0, 0, 0, PS2_BACKSPACE, 0,
     0, '1', 0, '4', '7', 0, 0, 0,
     '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
     PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
     0, 0, 0, PS2_F7 }
},

/* ES */ {
    // without shift
    {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
     0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, PS2_MASCULINE_ORDINAL, 0,
     0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'q', '1', 0,
     0, 0, 'z', 's', 'a', 'w', '2', 0,
     0, 'c', 'x', 'd', 'e', '4', '3', 0,
     0, ' ', 'v', 'f', 't', 'r', '5', 0,
     0, 'n', 'b', 'h', 'g', 'y', '6', 0,
     0, 0, 'm', 'j', 'u', '7', '8', 0,
     0, ',', 'k', 'i', 'o', '0', '9', 0,
     0, '.', '-', 'l', PS2_n_TILDE, 'p', '\'', 0,
     0, 0, PS2_ACUTE_ACCENT, 0, '`', PS2_INVERTED_EXCLAMATION, 0, 0,
     0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '+', 0, PS2_c_CEDILLA, 0, 0,
     0, '<', 0, 0, 0, 0, PS2_BACKSPACE, 0,
     0, '1', 0, '4', '7', 0, 0, 0,
     '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
     PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
     0, 0, 0, PS2_F7 },
    // with shift
    {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
     0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, PS2_FEMININE_ORDINAL, 0,
     0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'Q', '!', 0,
     0, 0, 'Z', 'S', 'A', 'W', '\"', 0,
     0, 'C', 'X', 'D', 'E', '$', PS2_MIDDLE_DOT, 0,
     0, ' ', 'V', 'F', 'T', 'R', '%', 0,
     0, 'N', 'B', 'H', 'G', 'Y', '&', 0,
     0, 0, 'M', 'J', 'U', '/', '(', 0,
     0, ';', 'K', 'I', 'O', '=', ')', 0,
     0, ':', '_', 'L', PS2_N_TILDE, 'P', '?', 0,
     0, 0, PS2_DIAERESIS, 0, '^', PS2_INVERTED_QUESTION_MARK, 0, 0,
     0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '*', 0, PS2_C_CEDILLA, 0, 0,
     0, '>', 0, 0, 0, 0, PS2_BACKSPACE, 0,
     0, '1', 0, '4', '7', 0, 0, 0,
     '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
     PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
     0, 0, 0, PS2_F7 },
         1,
    // with altgr
    {0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
     0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, '\\', 0,
     0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 0, '|', 0,
     0, 0, 0, 0, 0, 0, '@', 0,
     0, 0, 0, 0, PS2_EURO_SIGN, '~', '#', 0,
     0, ' ', 0, 0, 0, 0, '5', 0,
     0, 0, 0, 0, 0, 0, PS2_NOT_SIGN, 0,
     0, 0, 0, 0, 0, '7', '8', 0,
     0, ',', 0, 0, 0, '0', '9', 0,
     0, '.', '-', 0, 0, 0, '\'', 0,
     0, 0, '{', 0, '[', PS2_INVERTED_EXCLAMATION, 0, 0,
     0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, ']', 0, '}', 0, 0,
     0, '|', 0, 0, 0, 0, PS2_BACKSPACE, 0,
     0, '1', 0, '4', '7', 0, 0, 0,
     '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
     PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
     0, 0, 0, PS2_F7 }
}
};

byte gigatronKey(byte ascii)
{
  if (ascii == 9)
    return ascii;
  if (ascii == 10)
    return ascii;
  if (ascii == 27)
    return ascii;
  if (32 <= ascii && ascii <= 127)
    return ascii;
  if (0xc1 <= ascii && ascii <= 0xcc)
    return ascii;
  return 0;
}

int main(void)
{
  for (int map=0; map<6; map++) {
    for (int i=0; i<ps2keymapSize; i++) {
      int j;

      j = keymaps[map].noshift[i];
      if (gigatronKey(j))
        printf("map %d page 0 code %3d ascii %3d\n", map, i, j);

      j = keymaps[map].shift[i];
      if (gigatronKey(j))
        printf("map %d page 1 code %3d ascii %3d\n", map, i, j);

      if (keymaps[map].hasAltGr) {
        j = keymaps[map].altgr[i];
        if (gigatronKey(j))
          printf("map %d page 2 code %3d ascii %3d\n", map, i, j);
      }
    }
  }
}

