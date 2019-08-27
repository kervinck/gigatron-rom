// Standard includes
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>

// System include
#include <Gigatron.h>

const int tinyfont[96] = {
  #include "../Utils/BabelFish/tinyfont.h"
};

#define romType                 (*(byte*)0x0021)
#define romTypeValue_ROMv4      0x38
#define romTypeValue_DEVROM     0xf8

#define SYS_Draw4_30                    0x04d4
#define SYS_SetMemory_v2_54             0x0b03
#define SYS_ExpanderControl_v4_40       0x0b09
#define SYS_SpiExchangeBytes_v4_134     0x0b15
#define SYS_VDrawWord_137               0x00cb

#define CHAR_LEADING 0
#define CHAR_WIDTH 4
#define CHAR_HEIGHT 6

// TODO: make sure we can write text the 'old fashion' way
// Hijack original functions
#define Newline() Newline_ex()
#define PutChar(c) PutChar_ex(c)
#define puts(s) ClearScreen_ex()
#define ClearScreen() ClearScreen_ex()

// Forward declarations
void Newline_ex(void);
void drawChar( int* _char );
void PutChar_ex(int c);
int puts_ex(const char *s);
byte SPIExchangeKey( byte _key );
void drawCursor( byte _copy );
void setCursor( byte _r, byte _c );
void ClearScreen_ex();
void cursorUp();
void cursorDown();
void cursorLeft();
void cursorRight();


byte oldchar;
byte keyrepeat = 0;
int cursor[ 12 ];


int main(void)
{
    int i;
    byte keypress,spiData,row,column;

    keypress = 0;
    ScreenPos = (int)screenMemory;

    if ( romType < romTypeValue_DEVROM )
    {
        //puts( "Terminal is only supported\non ROMv4 or higher." );
        puts( "Terminal is only supported\non xopr's DEVROM." );
        while(1);
    }

    ClearScreen();
    drawCursor( 1 );
    setCursor( 1, 1 );

    while ( 1 )
    {
        oldchar = keypress;
        keypress = serialRaw;
        spiData = keypress;

        if ( oldchar == keypress )
            spiData = 0;

        if ( !keypress )
            keyrepeat = 60;
        else if ( oldchar == keypress && keyrepeat && keyrepeat-- )
            spiData = 0;

        // Send key to terminal server
        spiData = SPIExchangeKey( spiData );

        // https://en.wikipedia.org/wiki/VT52
        // In escape mode?
        switch ( oldchar )
        {
            case 27:
                switch ( spiData )
                {
                    case 'A': // cursor up (no scrolling)
                        cursorUp();
                        break;
                    case 'B': // cursor down
                        cursorDown();
                        break;
                    case 'C': // cursor right
                        cursorRight();
                        break;
                    case 'D': // cursor left
                        cursorLeft();
                        break;
                    case 'E': // clear screen
                        ClearScreen();
                        break;
                    case 'F': // enter graphics mode
                        // TODO
                        break;
                    case 'G': // exit graphics mode
                        // TODO
                        break;
                    case 'H': // cursor home (1,1)
                        setCursor( 1, 1 );
                        break;
                    case 'I': // reverse line feed (insert above cursor, may scroll)
                        // TODO
                        break;
                    case 'J': // clear to end of screen
                        // TODO
                        break;
                    case 'K': // clear to end of line
                        // TODO
                        break;
                    case 'L': // insert line
                        // TODO
                        break;
                    case 'M': // delete line
                        // TODO
                        case 'Y': // set cursor  position,  add 31
                        keypress = 28;
                        break;
                    case 'Z': // ident
                        // reply: ESC/K (VT52)
                        spiData = 27;
                        spiData = SPIExchangeKey( spiData );
                        spiData = '/';
                        spiData = SPIExchangeKey( spiData );
                        spiData = 'K';
                        spiData = SPIExchangeKey( spiData );
                        break;
                    case '=': // enter alternate keypad
                        // TODO
                        break;
                    case '>': // exit alternate keypad
                        // TODO
                        break;
                    case '<': // enter/exit ANSI mode
                        // TODO
                        break;
                    case 0:
                        keypress = 27;
                        break;

                    default:
                    PutChar( spiData );
                    break;
                }

                break;
            case 28: // hack: row
                //Color = Red;PutChar( spiData );Color = White;
                if ( !spiData )
                {
                    keypress = 28;
                }
                else
                {
                    row = spiData - 31;
                    keypress = 29;
                }
                break;
            case 29: // hack: column
                //Color = Green;PutChar( spiData );Color = White;
                if ( !spiData )
                {
                    keypress = 28;
                }
                else
                {
                    column = spiData - 31;
                    setCursor( row, column );
                }
                break;

            default:
                switch ( spiData )
                {
                    // Handle arrow keys and/or buttons
                    case buttonLeft:  cursorLeft(); break;
                    case buttonRight: cursorRight(); break;
                    case buttonUp:    cursorUp(); break;
                    case buttonDown:  cursorDown(); break;
                    case 127:                   // Delete key (also the [A] button...)
                    case 8:                     // Backspace
                        if ((ScreenPos & 255) >= CHAR_WIDTH)
                        {
                            // Remove cursor
                            drawChar( cursor );
                            // Backspace
                            ScreenPos -= CHAR_WIDTH;

                            // draw cursor
                            drawCursor( 0 );
                        }
                        break;
                    case 0: break; // Key up
                    case 255: break; // No data
                    case 27:
                        // Set escape code start sequence
                        oldchar = 27;
                    break; 
                    //case 13:
                    case 13: // carriage return
                        break;
                    default:
                        drawChar( cursor );
                        PutChar( spiData );               // Put character on screen
                        drawCursor( 1 );
                        break;
                }
        }
    }
    return 0;
}


void Newline_ex(void)
{
    int Y;

    Y = (ScreenPos >> 8) - videoTable[0];
    if ( Y < 0 )
        Y += 120;

    // carriage return
    ScreenPos &= 0xff00;

    // line feed
    ScreenPos += ( ( CHAR_HEIGHT + CHAR_LEADING ) * 256 );

    if ( ScreenPos > 0x7fff )
        ScreenPos -= 0x7800;

    // Wrap around screen memory
    if ( (Y + ( CHAR_HEIGHT + CHAR_LEADING )) >= 120 )
    {
        int i;

        // Clear this line
        sysFn = SYS_SetMemory_v2_54;
        sysArgs[1] = BgColor;       // Set background color

        i = ScreenPos + 1280;
        while ( i >= ScreenPos )
        {
            *(int*)(sysArgs+2) = i; // Destination
            sysArgs[0] = 160;               // Count
            __syscall(243); // 270-54/2
            i -= 256;
        }

        // Scroll up by manipulating video indirection table
        i = 240;
        do
        {
            byte page;
            i -= 2;
            page = videoTable[i] + CHAR_HEIGHT + CHAR_LEADING;
            videoTable[i] = page & 128 ? page - 120 : page;
        } while (i);
    }
}

void drawChar( int* _char )
{
    *(int*)ScreenPos = _char[0];
    *(int*)(ScreenPos + 2) = _char[1];

    *(int*)(ScreenPos + 256) = _char[2];
    *(int*)(ScreenPos + 258) = _char[3];

    *(int*)(ScreenPos + 512) = _char[4];
    *(int*)(ScreenPos + 514) = _char[5];

    *(int*)(ScreenPos + 768) = _char[6];
    *(int*)(ScreenPos + 770) = _char[7];

    *(int*)(ScreenPos + 1024) = _char[8];
    *(int*)(ScreenPos + 1026) = _char[9];

    *(int*)(ScreenPos + 1280) = _char[10];
    *(int*)(ScreenPos + 1282) = _char[11];
}

void PutChar_ex(int c)
{
    int i;
    int pixels;
    byte p = 0;;

    // Accept newlines
    if (c == '\n')
    {
        Newline();
        return;
    }

    // Ignore non-printable characters
    // XXX This still can print rubish?
    i = c - 32;
    if ((unsigned)i >= 128-32)
        return;

    sysFn = SYS_VDrawWord_137;
    sysArgs[0] = BgColor;
    sysArgs[1] = Color;
    *(int*)(sysArgs+2) = tinyfont[ i ];
    *(int*)(sysArgs+4) = ScreenPos;

    __syscall(202); // 270-135/2

    // Advance position
    ScreenPos += CHAR_WIDTH;

    // Automatic line wrapping
    if (((byte*)&ScreenPos)[0] > 160-CHAR_WIDTH)
        Newline();
}

int puts_ex(const char *s)
{
    // Note that this fuction is dumbed down: no stream support
    for (; *s; s++)
    {
        PutChar(*s);
    }
    return EOF;
}

byte SPIExchangeKey( byte _key )
{
    // Enable SPI0
    sysFn = SYS_ExpanderControl_v4_40;
    vAC = 0x78;
    __syscall(250); // 270-40/2

    *(int*)(sysArgs) = (int)&_key; // Start address
    *(int*)(sysArgs+2) = (int)(&_key) + 1; // End address
    sysFn = SYS_SpiExchangeBytes_v4_134;
    __syscall(203); // 270-134/2

    // Disable SPI0
    sysFn = SYS_ExpanderControl_v4_40;
    vAC = 0x7c;
    __syscall(250); // 270-40/2

    return _key;
}

void drawCursor( byte _copy )
{
    byte i;

    if ( _copy )
    {
        cursor[0] = *(int*)ScreenPos;
        cursor[1] = *(int*)(ScreenPos + 2);

        cursor[2] = *(int*)(ScreenPos + 256);
        cursor[3] = *(int*)(ScreenPos + 258);

        cursor[4] = *(int*)(ScreenPos + 512);
        cursor[5] = *(int*)(ScreenPos + 514);

        cursor[6] = *(int*)(ScreenPos + 768);
        cursor[7] = *(int*)(ScreenPos + 770);

        cursor[8] = *(int*)(ScreenPos + 1024);
        cursor[9] = *(int*)(ScreenPos + 1026);

        cursor[10] = *(int*)(ScreenPos + 1280);
        cursor[11] = *(int*)(ScreenPos + 1282);
    }
    else
    {
        for ( i = 0; i < 12; i++ )
            cursor[i] = BgColor << 8 | BgColor;
    }

    // Prepare SYS call
    sysFn = SYS_Draw4_30;//SYS_VDrawBits_134;
    sysArgs[0] = Color;
    sysArgs[1] = Color;
    sysArgs[2] = Color;
    sysArgs[3] = BgColor;
    *(int*)(sysArgs+4) = ScreenPos;

    // Draw bitmap to screen as 'n' vertical slices
    for (i=CHAR_HEIGHT; i>0; --i)
    {
        __syscall(255); // 270-30/2
        sysArgs[5]++;
    }
}

void setCursor( byte _r, byte _c )
{
    // Restore character behind cursor
    drawChar( cursor );

    // Update position; Adjust for scrolling
    _r = ( ( _r - 1 ) * CHAR_HEIGHT ) + videoTable[0];
    _c = ( _c - 1 ) * CHAR_WIDTH;

    ScreenPos = ( _r << 8 ) + _c;

    // Draw cursor
    drawCursor( 1 );
}

void ClearScreen_ex()
{
    int i;

    // Clear this line
    sysFn = SYS_SetMemory_v2_54;
    sysArgs[1] = BgColor;       // Set background color

    ScreenPos = (int)screenMemory + 0x7700;
    while ( ScreenPos >= (int)screenMemory )
    {
        *(int*)(sysArgs+2) = ScreenPos; // Destination
        sysArgs[0] = 160;               // Count
        __syscall(243); // 270-54/2
        ScreenPos -= 256;
    }

    // Reset scroll offsets
    i = 240;
    do
    {
        i -= 2;
        videoTable[i] = i / 2 + 0x08;
    } while (i);
}

void cursorUp()
{
    int Y;

    Y = (ScreenPos >> 8);
    Y -= videoTable[0];

    if ( Y == 0 /*|| Y > (  120 - CHAR_HEIGHT )*/ )
        return;

    drawChar( cursor );

    if ( (ScreenPos >> 8) < ( 8 + CHAR_HEIGHT ) )
        ScreenPos = ( (int)screenMemory + 0x7200 ) | ( ScreenPos & 0xff );
    else
        ScreenPos -= (CHAR_HEIGHT << 8);

    drawCursor( 1 );
}

void cursorDown()
{
    int Y;

    Y = (ScreenPos >> 8) - videoTable[0];
    if ( Y < 0 )
        Y += 120;

    // Wrap around screen memory
    if ( (Y + ( CHAR_HEIGHT + CHAR_LEADING )) >= 120 )
        return;

    drawChar( cursor );

    if ( (ScreenPos >> 8) > 120 )
        ScreenPos = ( (int)screenMemory ) | ( ScreenPos & 0xff );
    else
        ScreenPos += (CHAR_HEIGHT << 8);

    drawCursor( 1 );
}

void cursorLeft()
{
    if ( (ScreenPos & 0xff) < CHAR_WIDTH )
        return;

    drawChar( cursor );
    ScreenPos -= CHAR_WIDTH;
    drawCursor( 1 );    
}

void cursorRight()
{
    if ( (ScreenPos & 0xff) > (160 - CHAR_WIDTH - CHAR_WIDTH) )
        return;

    drawChar( cursor );
    ScreenPos += CHAR_WIDTH;
    drawCursor( 1 );    
}

