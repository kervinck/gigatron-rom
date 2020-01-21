// NOTE: disable a couple of defines if you get "IndexError: list index out of range"
//#define DEBUG               // Indicate data on the LEDs as a counter  
//#define TESTS               // Run some test procedures instead of actual terminal, defined in helpers.h
//#define SLOW                // Runs tests slower

#define COLORS              // Enable terminal colors (costs extra memory)
//#define LEDS                // Allow for usage of Gigatron LEDs within the application (costs extra memory)
#define FAST_START          // Quick startup by disabling the screen during initialization
#define ANSI_BLOCK_CHARS    // Allow ANSI block characters (costs extra memory)
//#define TERMINAL_MODES      // Enable terminal modes (costs extra memory)
#define TINY_FONT           // Enables small font (costs extra memory)

// Successful combinations (trial and error), leave out one of the following:
// COLORS
// LEDS and ANSI_BLOCK_CHARS
// LEDS and TERMINAL_MODES
// ANSI_BLOCK_CHARS and TERMINAL_MODES

// We can only define the port after including the helper file
#include "vtactionstates.h"
#include "helpers.h"

// Enable SPI hardware by assigning Slave Select pin (SS0 to SS3). Note that SS0 is reserved for an SD card.
// Comment out to do local loopback
#define IOPORT SS1

byte oldchar;
byte intermediate = 0;

int params[10] = {0};
byte paramCount = 0;

int savedCursor = 0;

char response[10] = {0};
byte responseIndex = 0;

#ifdef TESTS
void main(void)
{
    test();
    while( 1 );
}
#else
// Forward declaration
void vt_parse( byte _byte );
byte SPIExchangeKey( byte _key );

void main(void)
{
    byte spiData;

    ScreenPos = (int)screenMemory;
#ifndef TINY_FONT
    ScreenPos |= 0x02;
#endif

    if ( romType < romTypeValue_DEVROM )
    {
        //puts( "Terminal is only supported\non ROMv4 or higher." );
        puts( "Terminal is only supported\non xopr's DEVROM." );
        while(1);
    }

#ifdef FAST_START
    // Disable video to increase speed
    sysFn = SYS_SetMode_v2_80;
    vAC = 1975;
    __syscall(230); // 270-80/2
#else
    // Set lower video mode to increase speed
    sysFn = SYS_SetMode_v2_80;
    vAC = 3;
    __syscall(230); // 270-80/2
#endif

    EraseInDisplay( 2, ScreenPos );
    drawCursor( 1 );
    setCursor( 0, 0 );

#ifdef IOPORT
    // Enable SPI port
    sysFn = SYS_ExpanderControl_v4_40;
    vAC = ~IOPORT & SPI_MASK | BANK0;
    __syscall(250); // 270-40/2

    // To disable
    //vAC = IOPORT & SPI_MASK | BANK0;
#endif

#ifdef LEDS
    // Disable LED sequencer and set 4th led
    ledState = 1;
    xoutMask = 8;

#endif

#ifdef FAST_START
    // Set lower video mode to increase speed
    sysFn = SYS_SetMode_v2_80;
    vAC = 3;
    __syscall(230); // 270-80/2
#endif

    while ( 1 )
    {
        if ( response[ responseIndex ] )
        {
            spiData = response[ responseIndex ];
            responseIndex++;

            // End of string?
            if ( !response[ responseIndex ] )
            {
                response[ 0 ] = 0;
                responseIndex = 0;
            }
#ifndef IOPORT
            // Color output marking it dummy
            Color = Red;
            PutChar( spiData );
            Color = White;
#endif
        }
        else
        {
            spiData = serialRaw;

#ifndef IOPORT
            if ( spiData == DEL )
                spiData = BS;
#endif

            if ( spiData == oldchar )
                spiData = 0;
            else
                oldchar = spiData;

#if defined(DEBUG) && defined(LEDS)
            // Blinky led on input to detect freezes
            if ( spiData && spiData != 0xff )
                xoutMask++;
#endif

#ifdef IOPORT
            // Translate buttons to escape code
            switch ( spiData )
            {
                case buttonUp:    response[ 0 ] = 0x1B; response[ 1 ] = '['; response[ 2 ] = 'A'; response[ 3 ] = 0;break;
                case buttonDown:  response[ 0 ] = 0x1B; response[ 1 ] = '['; response[ 2 ] = 'B'; response[ 3 ] = 0;break;
                case buttonRight: response[ 0 ] = 0x1B; response[ 1 ] = '['; response[ 2 ] = 'C'; response[ 3 ] = 0;break;
                case buttonLeft:  response[ 0 ] = 0x1B; response[ 1 ] = '['; response[ 2 ] = 'D'; response[ 3 ] = 0;break;
            }            
#else
            // Translate buttons to local action
            switch ( spiData )
            {
                case buttonUp:    cursorUp( 1, 0 );    break;
                case buttonDown:  cursorDown( 1, 0 );  break;
                case buttonRight: cursorRight( 1 ); break;
                case buttonLeft:  cursorLeft( 1 );  break;
            }            
            vt_parse( spiData );
#endif
        }

#ifdef IOPORT
        // Send key to terminal server
        spiData = SPIExchangeKey( spiData );
        vt_parse( spiData );
#endif
    }
}

void vt_action_regular( byte _byte )
{
    if ( _byte >= SP && _byte < DEL )
    {
        // Print character
        PutChar( _byte );
        drawCursor( 1 );    
    }
#ifdef ANSI_BLOCK_CHARS
    else if ( _byte == 176 )
    {
        // Block light
        PutGlyph( 0x280a );
        drawCursor( 1 );    
    }
    else if ( _byte == 177 )
    {
        // Block medium
        PutGlyph( 0x5555 );
        drawCursor( 1 );    
    }
    else if ( _byte == 178 )
    {
        // Block dark
        PutGlyph( 0x7d5f );
        drawCursor( 1 );    
    }
    else if ( _byte == 219 )
    {
        // Block
        PutChar( 127 );
        drawCursor( 1 );    
    }
    else if ( _byte == 220 )
    {
        // Block bottom
        PutGlyph( 0x6318 );
        drawCursor( 1 );    
    }
    else if ( _byte == 221 )
    {
        // Block left
        PutGlyph( 0x03ff );
        drawCursor( 1 );    
    }
    else if ( _byte == 222 )
    {
        // Block right
        PutGlyph( 0xec00 );
        drawCursor( 1 );    
    }
    else if ( _byte == 223 )
    {
        // Block top
        PutGlyph( 0x1ce7 );
        drawCursor( 1 );    
    }
#endif
}

void vt_action_control_sequence_execute( byte _byte )
{
    switch ( _byte )
    {
        case 'A':
            // ESC [ Pn A          CUU -- Cursor Up -- Host to VT100 and VT100 to Host
            if ( params[0] == 0 )
                params[0] = 1;
            cursorUp( params[0], 0 );
            break;

        case 'B':
            // ESC [ Pn B          CUD -- Cursor Down -- Host to VT100 and VT100 to Host
            if ( params[0] == 0 )
                params[0] = 1;
            cursorDown( params[0], 0 );
            break;

        case 'C':
            // ESC [ Pn C          CUF -- Cursor Forward -- Host to VT100 and VT100 to Host
            if ( params[0] == 0 )
                params[0] = 1;
            cursorRight( params[0] );
            break;

        case 'D':
            // ESC [ Pn D          CUB -- Cursor Backward -- Host to VT100 and VT100 to Host (n positions, default 1), stop at margin
            if ( params[0] == 0 )
                params[0] = 1;
            cursorLeft( params[0] );
            break;

        case 'H':
            // ESC [ Pn ; Pn H     CUP -- Cursor Position (line, column)
        case 'f':
            // ESC [ Pn ; Pn f     HVP -- Horizontal and Vertical Position (line, column)

            // The numbering of lines depends on the state of the Origin Mode (DECOM).
            if ( params[0] )
                params[0]--;
            if ( paramCount < 2 )
                params[1] = 0;
            if ( params[1] )
                params[1]--;

            setCursor( params[0], params[1] );
            break;

        case 'J':
            // ESC [ Ps J          ED -- Erase In Display (
            //     0 	Erase from the active position to the end of the screen, inclusive (default)
            //     1 	Erase from start of the screen to the active position, inclusive
            //     2 	Erase all of the display -- all lines are erased, changed to single-width, and the cursor does not move.

            // Erase stored cursor character
            drawCursor( 0 );
            EraseInDisplay( params[0], ScreenPos );

            // Restore cursor
            drawCursor( 1 );
            break;

        case 'K':
            // ESC [ Ps K          EL -- Erase In Line
            //     0 	Erase from the active position to the end of the line, inclusive (default)
            //     1 	Erase from the start of the screen to the active position, inclusive
            //     2 	Erase all of the line, inclusive

            // Erase stored cursor character
            drawCursor( 0 );
            EraseInLine( params[0], ScreenPos );

            // Restore cursor
            drawCursor( 1 );
            break;

        case 'c':
            // ESC [ Pn c          DA -- Device Attributes (empty/0 from host, response from terminal:)
            //     No options 	ESC [?1;0c
            //     Processor option (STP) 	ESC [?1;1c
            //     Advanced video option (AVO) 	ESC [?1;2c  (10 additional lines of 132 column display,bold...)
            //     AVO and STP 	ESC [?1;3c
            //     Graphics option (GPO) 	ESC [?1;4c
            //     GPO and STP 	ESC [?1;5c
            //     GPO and AVO 	ESC [?1;6c
            //     GPO, STP and AVO 	ESC [?1;7c

            // Identify: set response to send back
            snprintf(response, sizeof response, "\x1B[?1;0c");
            //puts_ex( response );

            break;

        case 'g':
            // ESC [ Ps g          TBC -- Tabulation Clear (0=current, 3=all)
            break;
#ifdef TERMINAL_MODES
        case 'h':
            // ESC [ Ps ; . . . ; Ps h     SM -- Set Mode (reset mode counterpart)
            while ( paramCount-- )
            {
                if ( !params[ paramCount ] )
                    continue;
                if ( params[ paramCount ] == 20 )
                    params[ paramCount ] = 0;
                if ( params[ paramCount ] > 9 )
                    continue;
                terminalModes |= (1 << params[ paramCount ]);
            }
            break;

        case 'l':
            // ESC [ Ps ; Ps ; . . . ; Ps l    RM -- Reset Mode (set mode counterpart)
            while ( paramCount-- )
            {
                if ( !params[ paramCount ] )
                    continue;
                if ( params[ paramCount ] == 20 )
                    params[ paramCount ] = 0;
                if ( params[ paramCount ] > 9 )
                    continue;
                terminalModes &= ~(1 << params[ paramCount ]);
            }
            break;
#endif

#ifdef COLORS
        case 'm':
            // ESC [ Ps ; . . . ; Ps m     SGR -- Select Graphic Rendition (attributes)
            // Restore character (attribute under cursor will change)
            drawChar( cursor );

            while ( paramCount-- )
            {
                switch ( params[ paramCount ] )
                {
                    case 0: Color = White; BgColor = Blue; break;

                    case 30: Color = Black; break;
                    case 31: Color = Red; break;
                    case 32: Color = Green; break;
                    case 33: Color = Yellow; break;
                    case 34: Color = Blue; break;
                    case 35: Color = Magenta; break;
                    case 36: Color = Cyan; break;
                    case 37: Color = LightGray; break;

                    case 40: BgColor = Black; break;
                    case 41: BgColor = Red; break;
                    case 42: BgColor = Green; break;
                    case 43: BgColor = Yellow; break;
                    case 44: BgColor = Blue; break;
                    case 45: BgColor = Magenta; break;
                    case 46: BgColor = Cyan; break;
                    case 47: BgColor = LightGray; break;

                    case 90: Color = DarkGray; break;
                    case 91: Color = LightRed; break;
                    case 92: Color = LightGreen; break;
                    case 93: Color = LightYellow; break;
                    case 94: Color = LightBlue; break;
                    case 95: Color = LightMagenta; break;
                    case 96: Color = LightCyan; break;
                    case 97: Color = White; break;

                    case 100: BgColor = DarkGray; break;
                    case 101: BgColor = LightRed; break;
                    case 102: BgColor = LightGreen; break;
                    case 103: BgColor = LightYellow; break;
                    case 104: BgColor = LightBlue; break;
                    case 105: BgColor = LightMagenta; break;
                    case 106: BgColor = LightCyan; break;
                    case 107: BgColor = White; break;
                }
            }
            
            // Restore cursor
            drawCursor( 1 );

            break;
#endif

        case 'n':
            // ESC [ Ps n          DSR -- Device Status Report
            switch ( params[ 0 ] )
            {
                case 5:
                    //     5 	Command from host -- Please report status (using a DSR control sequence)
                    //     0 	Response from VT100 -- Ready, No malfunctions detected (default)
                    //     3 	Response from VT100 -- Malfunction -- retry
                    // Report ready: set response to send back
                    snprintf( response, sizeof response, "\x1B[0n" );
                    break;
                case 6:
                    //     6 	Command from host -- Please report active position (using a CPR control sequence) response 
                    // ESC [ Pn ; Pn R:    CPR -- Cursor Position Report -- VT100 to Host (line,column)
                    // Report ready: set response to send back
                    // Abuse intermediate variable
                    intermediate = (ScreenPos >> 8);

                    if ( videoTable[0] > intermediate )
                        intermediate = intermediate + 120 - videoTable[0];
                    else
                        intermediate -= videoTable[0];

                    intermediate /= ( CHAR_HEIGHT + CHAR_LEADING );
                    snprintf( response, sizeof response, "\x1B[%u,%uR", intermediate + 1, ( ScreenPos & 0xff ) / CHAR_WIDTH + 1 );
                    break;
            }
            break;
#ifdef LEDS
        case 'q':
            // ESC [ Ps q          DECLL -- Load LEDS (Linux supports L1 as scroll lock, L2 NumLock, L3 as Caps lock  , )
            // ESC [ Ps;Ps;...Ps q
            // 0 or None	All LEDs Off
            // 1	L1 On
            // 2	L2 On
            // 3	L3 On
            // 4	L4 On

            // Set LEDs
            while ( paramCount-- )
            {
                if ( !params[ paramCount ] )
                {
                    xoutMask = 0;
                }
                else if ( params[ paramCount ] <= 4 )
                {
                    // Turn number into bit mask
                    params[ paramCount ]--;
                    xoutMask |= ( 1 << params[ paramCount ] );
                }
            }

            break;
#endif
/*
        case 'r':
            // ESC [ Pn; Pn r      DECSTBM -- Set Top and Bottom Margins (startline, endline scroll)
            break;

        case 'x':
            // ESC [ <sol>; <par>; <nbits>; <xspeed>; <rspeed>; <clkmul>; <flags> x    DECREPTPARM -- Report Terminal Parameters
            break;

        case 'y':
            // ESC [ 2 ; Ps y      DECTST -- Invoke Confidence Test
            //     Test 	Weight
            //     0: reset only
            //     1: Power up self-test (ROM check sum, RAM, NVR keyboard and AVO if installed)
            //     2: Data Loop Back 	(loop back connector required)
            //     4: EIA modem control test 	(loop back connector required)
            //     8: Repeat Selected Test(s) indefinitely (until failure or power off)
            //     0 	Attributes off
            //     1 	Bold or increased intensity
            //     4 	Underscore
            //     5 	Blink
            //     7 	Negative (reverse) image
            break;
*/
    }
}

void vt_action_control_sequence( byte _byte )
{
    if ( _byte >= 0x30 && _byte <= 0x39 )
    {
        // Pn: Numbers: param
        params[paramCount] = params[paramCount] * 10 + _byte - 0x30;
    }
    else if ( _byte == ';' )
    {
        // Ps: Next param
        paramCount++;
        params[paramCount] = 0;
    }
    else if ( _byte >= 0x40 && _byte <= 0x7E )
    {
        // Offset param count (0->1)
        paramCount++;

        // Handle Control sequence
        vt_action_control_sequence_execute( _byte );
        paramCount = 0;
        params[0] = 0;
        mode = VTPARSE_MODE_REGULAR;
    }
    else if ( _byte == '?' )
    {
        // Note that in some cases, ESC[? is sent and therefore allowed
    }
}

void vt_action_escape_sequence( byte _byte )
{
    switch ( _byte )
    {
        case '0':
            // SCS -- Select Character Set
            // G0 ( or G1 ) Special graphics
            break;
        case '1':
            // SCS -- Select Character Set
            // G0 ( or G1 ) Alternate ROM standard
            break;
        case '2':
            // SCS -- Select Character Set
            // G0 ( or G1 ) Alternate ROM special graphics
            break;
        case '3':
        case '4':
            // # DECDHL -- Double Height Line
            // top/bottom
            break;
        case '5':
            // # DECSWL -- Single-width Line
            break;
        case '6':
            // # DECDWL -- Double-Width Line
            break;
        case '7':
            // DECSC -- Save Cursor
            // This sequence causes the cursor position, graphic rendition (attributes), and character set to be saved. (See DECRC).
            savedCursor = ScreenPos;
            break;
        case '8':
            // # DECALN -- Screen Alignment Display
            // Fill screen with 'E'

            if ( intermediate != '#' )
            {
                // DECRC -- Restore Cursor
                if ( savedCursor != ScreenPos )
                {
                    // Restore character behind cursor
                    drawChar( cursor );
                    ScreenPos = savedCursor;
                    // Draw cursor
                    drawCursor( 1 );
                }
            }
            break;
        case 'A':
            // SCS -- Select Character Set
            // G0 ( or G1 ) UK

            // VT52: Cursor up (no scroll)
            break;
        case 'B':
            // SCS -- Select Character Set
            // G0 ( or G1 ) ASCII

            // VT52: Cursor down (no scroll)
            break;
        case 'C':
            // VT52: Cursor right (no scroll/newline)
            break;
        case 'D':
            // IND -- Index (cursor down, allow scroll)
            cursorDown( 1, 0 );

            // VT52: Cursor left (no scroll/newline)
            break;
        case 'E':
            // NEL -- Next Line (CRLF, allow scroll)
            Newline();
            drawCursor( 1 );
            break;
        case 'F':
            // VT52: Enter Graphics Mode
            break;
        case 'G':
            // VT52: Select ASCII character set
            break;
        case 'H':
            // HTS -- Horizontal Tabulation Set

            // VT52: Move the cursor to the home position.
            break;
        case 'I':
            // VT52: Reverse Line Feed
            break;
        case 'J':
            // VT52: Erase to End of Screen
            break;
        case 'K':
            // VT52: Erase to End of Line
            break;
        case 'M':
            // RI -- Reverse Index (cursor up, allow scoll)
            cursorUp( 1, 1 );
            break;
        case 'Y':
            // VT52: Direct Cursor Address (needs line, column)
            //mode = VTPARSE_MODE_ESCAPE_CURSOR_LINE;
            break;
        case 'Z':
            // DECID -- Identify Terminal, deprecated->ESC[?1;0c
            // set response to send back
            snprintf(response, sizeof response, "\x1B[?1;0c");
            //puts_ex( response );

            // VT52: Identify, reply with ESC / Z
            break;
#ifdef TERMINAL_MODES
        case '=':
            // DECKPAM -- Keypad Application Mode
            terminalModes |= DECKPAM;
            // VT52: Enter Alternate Keypad Mode
            break;
        case '>':
            // DECKPNM -- Keypad Numeric Mode
            terminalModes &= ~DECKPAM;
#endif
            // VT52: Exit Alternate Keypad Mode
            break;
        case '<':
            // VT52: Enter ANSI Mode (ignore VT52 escape codes)
            break;
    }

    // Note: the previous switch statement is at its max apparently
    switch ( _byte )
    {
        case 'c':
            // RIS -- Reset To Initial State
            break;
        case '[':
            // CSI -- Control Sequence Introducer
            mode = VTPARSE_MODE_CONTROL_SEQUENCE;
            break;
    }

    // Reset mode if it was not altered
    if ( mode == VTPARSE_MODE_ESCAPE_INTERMEDIATE )
        mode = VTPARSE_MODE_REGULAR;

    intermediate = 0;
}

void vt_action_escape_intermediate( byte _byte )
{
    if ( _byte >= 0x30 && _byte <= 0x7E )
    {
        // Handle escape (might turn into control sequence)
        vt_action_escape_sequence( _byte );
    }
    else if ( _byte < 0x20 || _byte >= 0x2F )
    {
        // TODO: Error
    }

    // Store intermediate (assume only 1 character)
    if ( !intermediate )
        intermediate = _byte;
}

byte vt_parse_control_char( byte _byte )
{
    /*
    if ( _byte != NUL )
    {
        snprintf(response, sizeof response, "%d", _byte );
        puts_ex( response );
    }
    */
    // Check if we need to parse a control char in the stream
    if ( _byte == NUL )
    {
        // Ignored on input (not stored in input buffer; see full duplex protocol).
    }
    else if ( _byte == ENQ )
    {
        // Transmit answerback message.
    }
    else if ( _byte == BEL )
    {
        // bell sound
    }
    else if ( _byte == BS )
    {
        // Backspace
        // TODO: rollover
        if ((ScreenPos & 0xff) >= CHAR_WIDTH)
        {
            // Remove cursor
            drawChar( cursor );

            ScreenPos -= CHAR_WIDTH;

            // draw cursor
            drawCursor( 0 );
        }
    }
    else if ( _byte == '`'/*HT*/ )
    {
        // move to next tabstop (align on 8*CHAR_WIDTH pixels)
        drawChar( cursor );

        PutChar_ex( HT );

        // Restore character behind cursor
        drawCursor( 1 );    
    }
    else if ( _byte >= LF && _byte <= FF )
    {
        // Line feed: depends on mode
        // Note: incoming keyboard enter translates to line feed
#ifndef IOPORT
        drawChar( cursor );
        Newline();
        drawCursor( 1 );
#endif
    }
    else if ( _byte == CR )
    {
        // (Carriage) Return
        Newline();
        drawCursor( 1 );
    }
    else if ( _byte == SO )
    {
        // TODO: SO 	    016 0E 	Invoke G1 character set, as designated by SCS control sequence.
    }
    else if ( _byte == SI )
    {
        // TODO: SI 	    017 0F 	Select G0 character set, as selected by ESC ( sequence.
    }
    else if ( _byte == XON )
    {
        // TODO: XON 	021 11 	Causes terminal to resume transmission.
    }
    else if ( _byte == XOFF )
    {
        // TODO: XOFF 	023 13 	Causes terminal to stop transmitted all codes except XOFF and XON.
    }
    else if ( _byte == ESC || _byte == CAN || _byte == SUB )
    {
        // Clear params/intermediates, abort or restart escape sequence
        intermediate = 0;
        paramCount = 0;
        params[0] = 0;

        // Restart or Abort?
        if ( _byte == ESC )
            mode = VTPARSE_MODE_ESCAPE_INTERMEDIATE;
        else
            mode = VTPARSE_MODE_REGULAR;
    }
    else if ( _byte == DEL )
    {
        // Ignored on input (not stored in input buffer).
    }
    else
    {
        // No control character
        return 0;
    }

    // Control character parsed
    return 1;
}

void vt_parse( byte _byte )
{
    // Check if we got a control character (which will be parsed)
    if ( vt_parse_control_char( _byte ) )
        return;

    switch ( mode )
    {
        case VTPARSE_MODE_REGULAR:                // (control+printable characters)
            vt_action_regular( _byte );
            break;

        case VTPARSE_MODE_ESCAPE_INTERMEDIATE:    // 0x20-0x2F (attribute chars)
            vt_action_escape_intermediate( _byte );
            break;

        case VTPARSE_MODE_CONTROL_SEQUENCE:       // 0x40-x07E (characters)
            vt_action_control_sequence( _byte );
            break;
    }
}

byte SPIExchangeKey( byte _key )
{
    *(int*)(sysArgs) = (int)&_key; // Start address
    *(int*)(sysArgs+2) = (int)(&_key) + 1; // End address
    sysFn = SYS_SpiExchangeBytes_v4_134;
    __syscall(203); // 270-134/2

    return _key;
}
#endif
