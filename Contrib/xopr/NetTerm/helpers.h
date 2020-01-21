#ifdef TINY_FONT
    #define ROWS 20
    #define COLS 40
    #define CHAR_LEADING 0
    #define CHAR_WIDTH 4
    #define CHAR_HEIGHT 6
#else
    // NOTE: Normal chars have a horizontal offset of 2, 26 chars
    #define ROWS 15
    #define COLS 26
    #define CHAR_LEADING 0
    #define CHAR_WIDTH 6
    #define CHAR_HEIGHT 8
#endif

#define MAX_X (COLS*CHAR_WIDTH)
#define MAX_Y (ROWS*(CHAR_HEIGHT+CHAR_LEADING))

// Standard includes
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>

// System include
#include <Gigatron.h>

// SPI ports (active low)
#define SS0 0x04
#define SS1 0x08
#define SS2 0x10
#define SS3 0x20
#define BANK0 0x40
#define BANK1 0x80
#define SPI_MASK 0x3c
#define BANK_MASK 0xc0

#define DarkYellow              0x05
#define Yellow                  0x0a
#define LightYellow             0x0f

#define DarkMagenta             0x11
#define Magenta                 0x22
#define LightMagenta            0x33

#define DarkCyan                0x14
#define Cyan                    0x28
#define LightCyan               0x3c

#define DarkGray                0x15
#define LightGray               0x2a

#define romType                 (*(byte*)0x0021)
#define romTypeValue_ROMv4      0x38
#define romTypeValue_DEVROM     0xf8

#define xoutMask                (*(byte*)0x0014)
#define ledState                (*(byte*)0x002E)
#define ledTempo                (*(byte*)0x002F)

#define SYS_VDrawWord_134               0x00cb
#define SYS_Draw4_30                    0x04d4
#define SYS_SetMode_v2_80               0x0b00
#define SYS_SetMemory_v2_54             0x0b03
#define SYS_ExpanderControl_v4_40       0x0b09
#define SYS_SpiExchangeBytes_v4_134     0x0b15

// Forward declaration
void cursorDown( byte _amount, byte _allowScroll );
#define NEWLINE_EX() {cursorDown(1,1);ScreenPos &= 0xff00;}
#define NEWLINE_EX2() {cursorDown(1,1);ScreenPos &= 0xff00;ScreenPos |= 0x0002;}
#define ROW_TO_SCREEN(_r,_s){\
    ((byte*)&_s)[1] = _r * ( CHAR_HEIGHT + CHAR_LEADING ) + videoTable[0];\
    if ( ((byte*)&_s)[1] > 0x7f )\
        ((byte*)&_s)[1] -= 120;\
}

#ifdef TINY_FONT
// Hijack original functions
#define PutChar(c) PutChar_ex(c)
//#define puts(s) puts_ex(s)
#define Newline() NEWLINE_EX()
#else
#define Newline() NEWLINE_EX2()
#endif

int cursor[ 12 ];

const int tinyfont[96] = {
  #include "../Utils/BabelFish/tinyfont.h"
};

void EraseInLine( byte _mode, int _screenCoord )
{
    byte size;
    char line;

    switch ( _mode )
    {
        case 0:
            // Erase from the active position to the end of the line, inclusive (default)
            //size = 160 - (_screenCoord & 0x00ff);
            size = 160 - ((byte*)&_screenCoord)[0];

            break;

        case 1:
            // Erase from the start of the line to the active position, inclusive
            size = 160 - ((byte*)&_screenCoord)[0] + CHAR_WIDTH;
            _screenCoord = _screenCoord & 0xff00;
            break;

        case 2:
            // Erase all of the line, inclusive
            _screenCoord = _screenCoord & 0xff00;
            size = 160;
            break;
    }

    // Clear this line
    sysFn = SYS_SetMemory_v2_54;
    sysArgs[1] = BgColor;       // Set background color

    for ( line = 0; line < CHAR_HEIGHT; ++line )
    {
        *(int*)(sysArgs+2) = _screenCoord; // Destination
        sysArgs[0] = size;          // Count
        __syscall(243);             // 270-54/2
        _screenCoord += 256;
    }
}

void PutChar_ex( byte c )
{
    // DECAWM->autowrap to newline (allow scroll) or overwrite last char
    byte i;

#ifdef TERMINAL_MODES
    byte wrap = (terminalModes & DECAWM) ? 1 : 0;
#else
    byte wrap = 1;
#endif

    // Accept newlines
    if (c == '\n')
    {
        Newline();
        return;
    }
    
    if ( c == 9 )
    {
        // TODO: make tab work for big font as well
        ScreenPos += 8 * CHAR_WIDTH;
        ScreenPos &= ~(8*CHAR_WIDTH - 1);
    }
    else if ( c < 32 || c > 127 )
        return;

    // Wrap to new line if we're outside the view
    if (((byte*)&ScreenPos)[0] >= MAX_X )
    {
        if ( wrap )
        {
            Newline();
        }
        else
        {
            ScreenPos -= CHAR_WIDTH;
        }
    }

    if ( c == 9 )
        return;

    i = c - 32;
    sysFn = SYS_VDrawWord_134;
    sysArgs[0] = BgColor;
    sysArgs[1] = Color;
    *(int*)(sysArgs+2) = tinyfont[ i ];
    *(int*)(sysArgs+4) = ScreenPos;

    __syscall(202); // 270-135/2

    // Note that the column can be out of the display area
    // (to prevent automatic newlines at the end of the screen)
    //column++;
    ScreenPos += CHAR_WIDTH;
}

signed char puts_ex(const char *s)
{
    // Note that this fuction is dumbed down: no stream support
    for (; *s; s++)
    {
        PutChar(*s);
    }
    return EOF;
}

void ScrollUp()
{
    int screenCoord;
    byte i;

    // Clear first line since video rolls over
    ROW_TO_SCREEN( 0, screenCoord );
    EraseInLine( 2, screenCoord );

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

void PutGlyph( unsigned long _g )
{
    // Wrap to new line if we're outside the view
    if (((byte*)&ScreenPos)[0] >= MAX_X )
        Newline();

    sysFn = SYS_VDrawWord_134;
    sysArgs[0] = BgColor;
    sysArgs[1] = Color;
    *(int*)(sysArgs+2) = _g;
    *(int*)(sysArgs+4) = ScreenPos;

    __syscall(202); // 270-135/2

    // Note that the column can be out of the display area
    // (to prevent automatic newlines at the end of the screen)
    //column++;
    ScreenPos += CHAR_WIDTH;
}

void drawChar( int* _char )
{
    if (((byte*)&ScreenPos)[0] >= MAX_X )
        return;

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

void drawCursor( byte _copy )
{
    byte i;

    // TODO: maybe try and draw on the next line
    if (((byte*)&ScreenPos)[0] >= MAX_X )
        return;

    // TODO: MAX_Y

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
    sysFn = SYS_Draw4_30;
    sysArgs[0] = Color;
    sysArgs[1] = Color;
    sysArgs[2] = Color;
    sysArgs[3] = BgColor;
    *(int*)(sysArgs+4) = ScreenPos;

    // Draw bitmap to screen as 'n' vertical slices
    // TODO: there are not enough cycles to include the 6th line when drawing a character; dont draw any pixels in it for now to avoid clutter
    //for (i=CHAR_HEIGHT; i>0; --i)
    for (i=5; i>0; --i)
    {
        __syscall(255); // 270-30/2
        sysArgs[5]++;
    }
}

void setCursor( byte _r, byte _c )
{
    // Restore character behind cursor
    //drawChar( cursor );

    if ( _r >= ROWS )
        _r = ROWS - 1;

    if ( _c >= COLS )
        _c = COLS - 1;

    // Calculate row with scroll offset in mind
    ROW_TO_SCREEN( _r, ScreenPos );
    ScreenPos &= 0xff00;

#ifdef TINY_FONT
    ScreenPos |= (( _c * CHAR_WIDTH + videoTable[1] ) & 0x00ff );
#else
    ScreenPos |= (( _c * CHAR_WIDTH + videoTable[1] + 2) & 0x00ff );
#endif

    // Draw cursor
    //drawCursor( 1 );
}

void EraseInDisplay( byte _mode, int _screenCoord )
{
    byte line;
    byte row = ((byte*)&_screenCoord)[1];
    if ( videoTable[0] > row )
        row += 120;
    row -= videoTable[0];
    row /= ( CHAR_HEIGHT + CHAR_LEADING );

    // Erase current line
    EraseInLine( _mode, _screenCoord );

    // 0: end
    // 1: begin
    // 2: all
    if ( _mode && row )
    {
        // Erase begin or all of screen
        for ( line = 0; line < row; ++line )
        {
#if defined( SLOW ) && defined( TESTS )
            BusyWait( 10 );
#endif
            ROW_TO_SCREEN( line, _screenCoord );
            EraseInLine( 2, _screenCoord );
        }
    }
    if ( _mode != 1 )
    {
        // Erase end or all of screen
        //for ( ++row; row < ROWS - 1; ++row )
        for ( ++row; row < ROWS; ++row )
        {
#if defined( SLOW ) && defined( TESTS )
            BusyWait( 10 );
#endif
            ROW_TO_SCREEN( row, _screenCoord );
            EraseInLine( 2, _screenCoord );
        }
    }

}

void cursorUp( byte _amount, byte _allowScroll )
{
    // TODO: avoid usage of int -> (byte*)&_screenCoord)[1];
/*
    byte row = ((byte*)&_screenCoord)[1];
    if ( videoTable[0] > row )
        row += 120;
    row -= videoTable[0];
    row /= ( CHAR_HEIGHT + CHAR_LEADING );
*/
    int y = (ScreenPos >> 8) - videoTable[0];

    if ( y < 0 )
        y += 120;

    while ( _amount-- )
    {
        if ( y >= ( CHAR_HEIGHT + CHAR_LEADING ) )
        {
            ScreenPos -= (( CHAR_HEIGHT + CHAR_LEADING ) << 8 );
            y -= CHAR_HEIGHT + CHAR_LEADING;
        }
        else if ( _allowScroll )
        {
            // TODO: ScrollDown();
        }
        else
        {
            break;
        }
    }
}

void cursorDown( byte _amount, byte _allowScroll )
{
    // Determine y offset (avoid usage of int)
    byte row = ((byte*)&ScreenPos)[1];
    if ( videoTable[0] > row )
        row += 120;
    row -= videoTable[0];
    row /= ( CHAR_HEIGHT + CHAR_LEADING );

    while ( _amount-- )
    {
        if ( row < (ROWS - 1) )
        {
            row++;
            ROW_TO_SCREEN( row, ScreenPos );
        }
        else if ( _allowScroll )
        {
            ScrollUp();
            ROW_TO_SCREEN( row, ScreenPos );
        }
        else
        {
            break;
        }
    }
}

void cursorLeft( byte _amount )
{
    // TODO: avoid usage of int
    int x = (ScreenPos & 0xff );
    while ( _amount-- )
    {
        if ( x >= CHAR_WIDTH )
        {
            ScreenPos -= CHAR_WIDTH;
            x -= CHAR_WIDTH;
        }
        else
        {
            break;
        }
    }
}

void cursorRight( byte _amount )
{
    // TODO: avoid usage of int
    int x = (ScreenPos & 0xff );
    while ( _amount-- )
    {
        if ( x < (MAX_X - CHAR_WIDTH) )
        {
            ScreenPos += CHAR_WIDTH;
            x += CHAR_WIDTH;
        }
        else
        {
            break;
        }
    }
}

#ifdef TESTS
void test()
{
    reset();

    setCursor( 0, 0 );
    puts_ex( "Tests:" );

    reset();
    test_cursor();
    reset();
    test_clear();
    reset();
    test_colors();
    reset();
    test_linewrap();
    reset();
    test_scroll();
    reset();
    test_cursorscroll();
    reset();
    test_newline();
    reset();
    test_e3();
    // Done
    reset();
    ledState = 0;

    setCursor( 2, 2 );
    puts_ex( "Done." );
}

void test_cursor()
{
    // test setCursor, drawCursor and cursor up/down/left/right
    setCursor( 2, 2 );
    puts_ex( "Cursor" );
    // bottom right corner: '#'
    drawChar( cursor );
    setCursor( 100, 100 );
    PutChar( '#' );
    drawCursor( 1 );
    xoutMask++;
    BusyWait( 60 );

    // top right corner: 'B'
    drawChar( cursor );
    cursorLeft( 1 ); // TODO: fix cursor
    cursorUp( 100, 0 );
    PutChar( 'B' );
    drawCursor( 1 );
    xoutMask++;
    BusyWait( 60 );

    // top left corner: 'A'
    drawChar( cursor );
    cursorLeft( 100 );
    PutChar( 'A' );
    drawCursor( 1 );
    xoutMask++;
    BusyWait( 60 );

    // bottom left corner: 'D'
    drawChar( cursor );
    cursorLeft( 1 ); // TODO: fix cursor
    cursorDown( 100, 0 );
    PutChar( 'D' );
    drawCursor( 1 );
    xoutMask++;
    BusyWait( 60 );

    // bottom right corner: 'C'
    drawChar( cursor );
    cursorRight( 100 );
    PutChar( 'C' );
    drawCursor( 1 );
    xoutMask++;
    BusyWait( 60 );
}

void test_clear()
{
    // Test EraseInDisplay and inherently, EraseInLine in all 3 variations
    setCursor( 2, 2 );
    puts_ex( "Clear" );

    // Erase lower half with red
    BgColor = DarkRed;
    setCursor( ROWS / 2, COLS / 2 );
    EraseInDisplay( 0, ScreenPos );    
    xoutMask++;
    BusyWait( 60 );

    // Erase upper half with green
    BgColor = DarkGreen;
    EraseInDisplay( 1, ScreenPos );    
    xoutMask++;
    BusyWait( 60 );

    // Erase all with black
    BgColor = Black;
    EraseInDisplay( 2, ScreenPos );    
    xoutMask++;
    BusyWait( 60 );
}

void test_colors()
{
    // Test all 64 foreground and background colors
    byte r;
    byte c;

    setCursor( 2, 2 );
    puts_ex( "Colors" );

    BgColor = Black;
    for ( r = 0; r < 8; ++r )
    {
        for ( c = 0; c < 8; ++c )
        {
            setCursor( r + 5, c + 8 );
            Color = 8 * r + c;
            PutChar( '#' );
        }
    }
    xoutMask++;

    Color = White;
    for ( r = 0; r < 8; ++r )
    {
        for ( c = 0; c < 8; ++c )
        {
            setCursor( r + 5, c + 24 );
            BgColor = 8 * r + c;
            PutChar( '.' );
        }
    }
    xoutMask++;

    BusyWait( 60 );
}

void test_linewrap()
{
    // Test line wrapping
    byte count;
    char linewrap[] = "Line wrap";
    setCursor( 2, 2 );
    puts_ex( linewrap );

    BusyWait( 60 );

    setCursor( 0, 0 );
    count = 20;
    Color = LightGreen;
    while ( count-- )
    {
        puts_ex( linewrap );
        xoutMask++;
        BusyWait( 5 );
    }

    // Force DECAWM (automatic wrap mode) off
#ifdef TERMINAL_MODES
    terminalModes &= ~DECAWM;

    count = 5;
    Color = LightRed;
    while ( count-- )
    {
        puts_ex( linewrap );
        xoutMask++;
        BusyWait( 5 );
    }
#endif
}

void test_scroll()
{
    // Test screen scrolling
    byte count = 21;

    setCursor( 2, 2 );
    puts_ex( "Scroll" );

    while ( count-- )
    {
        ScrollUp();
        setCursor( 0, 10 );
        puts_ex( "line 1 will be erased" );
        setCursor( 19, 16 );
        puts_ex( "line 20" );
        xoutMask++;
        BusyWait( 10 );
    }

    // TODO: scroll down/left/up
}

void test_cursorscroll()
{
    // Test cursor scrolling
    byte count;

    setCursor( 2, 2 );
    puts_ex( "CursorScroll" );

    // Center
    setCursor( ROWS / 2, COLS / 2 );
    count = 21;
    while ( count-- )
    {
        cursorDown( 1, 1 );
        PutChar( '^' );
        xoutMask++;
        BusyWait( 30 );
    }

/*
    // Center
    setCursor( ROWS / 2, COLS / 2 );
    count = 21;
    while ( count-- )
    {
        cursorUp( 1, 1 );
        PutChar( 'v' );
        xoutMask++;
        BusyWait( 30 );
    }
*/
}

void test_newline()
{
    // Test newline scrolling
    byte count = 21;
    char newline[] = "Newline";

    setCursor( 2, 2 );
    puts_ex( newline );

    setCursor( 8, 0 );
    drawCursor( 0 );
    while ( count-- )
    {
        drawChar( cursor );
        Newline();
        puts_ex( newline );
        drawCursor( 1 );
        xoutMask++;
        BusyWait( 30 );
    }
}

void test_e3()
{
    // Fill screen with 'E', wait a second and fill the next 3 lines with '3'
    byte c;
    byte r = ROWS;

    setCursor( 2, 2 );
    puts_ex( "E3" );
    drawCursor( 1 );

    BusyWait( 60 );
    drawChar( cursor );
    setCursor( 0, 0 );
    drawCursor( 1 );

    // Fill screen with 'E'
    while ( r-- )
    {
        c = COLS;
        while ( c-- )
        {
            PutChar( 'E' );
            drawCursor( 0 );
#ifdef SLOW
            BusyWait( 1 );
#endif
            xoutMask++;
        }
    }
    BusyWait( 60 );
    r = 3;
    while ( r-- )
    {
        c = COLS;
        while ( c-- )
        {
            PutChar( '3' );
            drawCursor( 0 );
#ifdef SLOW
            BusyWait( 3 );
#endif
            xoutMask++;
        }
    }
}

void reset()
{
    byte i = 240;
    byte t = 127;

    ledState = 0;
    BusyWait( 60 );

    // LEDs static on
    ledState = 1;
    xoutMask = 0xf;

    //Reset cursor
    ScreenPos = (int)screenMemory;


    // Clear display
    BgColor = Blue;
    Color = White;
    EraseInDisplay( 2, ScreenPos );    

    // Reset scroll
    // Scroll up by manipulating video indirection table
    do
    {
        i -= 2;
        videoTable[i] = t--;
    } while (i);

    // Store and draw (empty) cursor
    drawCursor( 0 );
    drawChar( cursor );
}


#endif
