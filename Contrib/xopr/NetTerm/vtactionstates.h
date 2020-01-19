#ifndef VTACTIONSTATES_DOT_H
#define VTACTIONSTATES_DOT_H

#ifdef TERMINAL_MODES
// Terminal modes
#define LNM     (1 << 0) // !LNM -- Line Feed/New Line Mode
#define DECCKM  (1 << 1) // Cursor Keys Mode
#define DECANM  (1 << 2) // ANSI/VT52 Mode
#define DECCOLM (1 << 3) // Column Mode
#define DECSCLM (1 << 4) // Scrolling Mode
#define DECSCNM (1 << 5) // Screen Mode
#define DECOM   (1 << 6) // Origin Mode
#define DECAWM  (1 << 7) // Autowrap Mode
#define DECARM  (1 << 8) // Auto Repeat Mode
#define DECINLM (1 << 9) // Interlace Mode
#define DECKPAM (1 << 10) // Keypad Application Mode
#endif

// Control characters
#define NUL 0x00
#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define EOT 0x04
#define ENQ 0x05
#define ACK 0x06
#define BEL 0x07
#define BS  0x08
#define HT  0x09
#define LF  0x0A
#define VT  0x0B
#define FF  0x0C
#define CR  0x0D
#define SO  0x0E
#define SI  0x0F
#define DLE 0x10
#define DC1 0x11
#define XON 0x11
#define DC2 0x12
#define DC3 0x13
#define XOFF 0x13
#define DC4 0x14
#define NAK 0x15
#define SYN 0x16
#define ETB 0x17
#define CAN 0x18
#define EM  0x19
#define SUB 0x1A
#define ESC 0x1B
#define FS  0x1C
#define GS  0x1D
#define RS  0x1E
#define US  0x1F
#define SP  0x20
#define DEL 0x7F

typedef enum {
   VTPARSE_MODE_REGULAR = 0,                // (control+printable characters)
   VTPARSE_MODE_ESCAPE_INTERMEDIATE = 1,    // 
   VTPARSE_MODE_CONTROL_SEQUENCE = 2,       // 
   VTPARSE_MODE_ESCAPE_CURSOR_LINE = 3,     // waiting for line value
   VTPARSE_MODE_ESCAPE_CURSOR_COLUMN = 4,   // waiting for column value

} vtparse_mode_t;

vtparse_mode_t mode = VTPARSE_MODE_REGULAR;
#ifdef TERMINAL_MODES
int terminalModes = LNM | DECCKM | DECANM | DECAWM | DECARM;
#endif
#endif // VTACTIONSTATES_DOT_H

