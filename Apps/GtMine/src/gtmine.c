#include <stdlib.h>
#include <string.h>
#include <gigatron/console.h>
#include <gigatron/sys.h>
#include <gigatron/libc.h>
#include <stdarg.h>

#ifndef MEM32
# define MEM32 0 // set to 1 for 64 bits
#endif

// Game controller bits (actual controllers in kit have negative output)
// +----------------------------------------+
// |       Up                        B*     |
// |  Left    Right               B     A*  |
// |      Down      Select Start     A      |
// +----------------------------------------+ *=Auto fire
#define BUTTON_RIGHT  0xfe
#define BUTTON_LEFT   0xfd
#define BUTTON_DOWN   0xfb
#define BUTTON_UP     0xf7
#define BUTTON_START  0xef
#define BUTTON_SELECT 0xdf
#define BUTTON_B      0xbf
#define BUTTON_A      0x7f

// length of the queue for automatic uncovering of game fields.
// It is an alternative to a recursive function.
// I am afraid of stack problems with recursive functions.
#define MAXQ 40
#define REPETITION 6 // speed for automatic cursor movement

#if MEM32
    #define MAXX 26 // max 26
    #define MAXY 17 // max 17
    #define TOP 2
#else
    #define MAXX 26 // max 26
    #define MAXY 17 // max 17
    #define TOP 0
#endif

#define SFREE 0
#define S1 1
#define S2 2
#define S3 3
#define S4 4
#define S5 5
#define S6 6
#define S7 7
#define S8 8
#define SBOMB 9
#define SBOMBTRIGGERED 10
#define SCURSOR 11
#define SHIDDEN 12
#define SMARKER 13

#define BHIDDEN 0x10
#define BMARKER 0x20

__nohop const char sfree[]={
	44,44,44,44,44,46,
	44,44,44,44,44,46,
	44,44,44,44,44,46,
	44,44,44,44,44,46,
	44,44,44,44,44,46,
	46,46,46,46,46,46,
	250};           // 0
__nohop const char s1[]={
	44,44,48,48,44,46,
	44,44,44,48,44,46,
	44,44,44,48,44,46,
	44,44,44,48,44,46,
	44,44,44,48,44,46,
	46,46,46,46,46,46,
	250};              // 1
__nohop const char s2[]={
	44,8,8,8,44,46,
	44,44,44,44,8,46,
	44,44,8,8,44,46,
	44,8,44,44,44,46,
	44,8,8,8,8,46,
	46,46,46,46,46,46,
	250};                         // 2
__nohop const char s3[]={
	44,35,35,35,44,46,
	44,44,44,44,35,46,
	44,44,35,35,44,46,
	44,44,44,44,35,46,
	44,35,35,35,44,46,
	46,46,46,46,46,46,
	250};              // 3
__nohop const char s4[]={
	44,33,44,44,44,46,
	44,33,44,44,44,46,
	44,33,44,33,44,46,
	44,33,33,33,33,46,
	44,44,44,33,44,46,
	46,46,46,46,46,46,
	250};              // 4
__nohop const char s5[]={
	44,6,6,6,6,46,
	44,6,44,44,44,46,
	44,44,6,6,44,46,
	44,44,44,44,6,46,
	44,6,6,6,44,46,
	46,46,46,46,46,46,
	250};                         // 5
__nohop const char s6[]={
	44,44,57,57,44,46,
	44,57,44,44,44,46,
	44,57,57,57,44,46,
	44,57,44,44,57,46,
	44,44,57,57,44,46,
	46,46,46,46,46,46,
	250};              // 6
__nohop const char s7[]={
	44,16,16,16,16,46,
	44,44,44,44,16,46,
	44,44,44,16,44,46,
	44,44,16,44,44,46,
	44,44,16,44,44,46,
	46,46,46,46,46,46,
	250};              // 7
__nohop const char s8[]={
	44,44,37,37,44,46,
	44,37,44,44,37,46,
	44,44,37,37,44,46,
	44,37,44,44,37,46,
	44,44,37,37,44,46,
	46,46,46,46,46,46,
	250};              // 8
__nohop const char sbomb[]={
	16,44,16,44,16,46,
	44,61,16,16,44,46,
	16,16,16,16,16,46,
	44,16,16,16,44,46,
	16,44,16,44,16,46,
	46,46,46,46,46,46,
	250};           // 9
__nohop const char sbombtriggered[]={
	16,19,16,19,16,19,
	19,62,16,16,19,19,
	16,16,16,16,16,19,
	19,16,16,16,19,19,
	16,19,16,19,16,19,
	19,19,19,19,19,19,
	250};  // 10 [0x0a]
__nohop const char shidden[]={
	58,58,58,58,58,50,
	58,58,58,58,58,50,
	58,58,58,58,58,50,
	58,58,58,58,58,50,
	58,58,58,58,58,50,
	50,50,50,50,50,50,
	250};         // 12 [0x0c]
__nohop const char smarker[]={
	58,58,19,19,58,50,58,
	19,19,19,58,50,58,58,
	19,19,58,50,58,58,58,
	1,58,50,58,58,1,1,1,
	50,50,50,50,50,50,50,
	250};             // 13 [0x0d]
__nohop const char bigcursor[]={
	35,35,35,35,35,35,35,35,
	35, 0, 0, 0, 0, 0, 0,35,
	35, 0, 0, 0, 0, 0, 0,35,
	35, 0, 0, 0, 0, 0, 0,35,
	35, 0, 0, 0, 0, 0, 0,35,
	35, 0, 0, 0, 0, 0, 0,35,
	35, 0, 0, 0, 0, 0, 0,35,
	35,35,35,35,35,35,35,35,250};

typedef enum {
    BEGINNER, ADVANCED, EXPERT
} levels;

struct game_level_s {
    char fieldsX, fieldsY;
    int fields;
    char numberBomb, topMargin;
} game_level;

__near char leftMargin;
__near char markerCount;            // counter for marked fields
__near int  revealedFields;         // counter for revealed fields
__near char queuePointer;           // pointer to queue
__near char gameOver;               // flag, end of game reached
__near char newGame;                // Flag, start new game without closing the old one
__near char firstClick;             // Flag for start of the clock
__near unsigned int colors;
__near char bottonLevel;
__near unsigned int ticks;
__near unsigned int seconds;        // elapsed seconds

unsigned int queue[MAXQ];    // queue for automatic uncovering of game fields
char field[MAXY][MAXX];      // byte array for playing field, lower nibble sprite id, upper nibble flags
char backup[64];

void setLevel(struct game_level_s *data, levels level)
{
    static struct game_level_s
        advanced = { 16, 16, 256, 40, 20 },
        expert = { MAXX, MAXY, MAXX*MAXY, MAXX*MAXY/5, 17+(MEM32?1:0) },
        beginner = { 9, 9, 81, 10, 27};

    struct game_level_s *lvl = &beginner;
    if (level == ADVANCED)
        lvl = &advanced;
    else if (level == EXPERT)
        lvl = &expert;
    *data = *lvl;
}

void printCursor(char *addr, char *dest){ //
    int ii, zz, vv;
    zz = vv = ii = 0;
    while(addr[vv] < 128){
        if(addr[vv] > 0){
            backup[vv] = dest[zz + ii];
            dest[zz + ii] = addr[vv];
        }
        vv++; ii++;
        if(ii > 7){
            ii = 0;
            zz += 256;
        }
    }
}

void restoreCursor(char *addr, char *dest){ //
    int ii, zz, vv;
    zz = vv = ii = 0;
    while(addr[vv] < 128){
        if(addr[vv] > 0){
            dest[zz + ii] = backup[vv];
        }
        vv++; ii++;
        if(ii > 7){
            ii = 0;
            zz += 256;
        }
    }
}

void printSprite(int val, int xx, int yy)
// val is the id of the sprite, xx,yy is the x,y position in the playfield
{
    static const char* ptrChars[] = {
        sfree, s1, s2, s3, s4, s5, s6, s7, s8,
        sbomb, sbombtriggered, sfree, shidden, smarker };
    const char* ptrChar;
    int sprnum;
    
    sprnum = val & 0x0f;
    if(val >= BHIDDEN)
        sprnum = SHIDDEN;
    if(val >= BMARKER)
        sprnum = SMARKER;
    ptrChar = sfree;
    if (sprnum >= 0 && sprnum <= SMARKER) 
        ptrChar = ptrChars[sprnum];
    SYS_Sprite6(ptrChar, (char*)(yy*6+game_level.topMargin<<8)+6*xx+leftMargin);
}

void initialize()
{
    int i,x,y;

    for( y=0; y<game_level.fieldsY; y++ )
        for( x=0; x<game_level.fieldsX; x++ )
            field[y][x] = BHIDDEN;
    
    i = 0; // bomb counter temp
    // setting the bombs in the field
    while(i < game_level.numberBomb){
        x = rand() % (game_level.fieldsX);
        y = rand() % (game_level.fieldsY);
        if(field[y][x] != (SBOMB | BHIDDEN)){ // field is not a bomb, bomb set
            i++;                              // add bomb
            field[y][x] = SBOMB | BHIDDEN;    // set marker for bomb
        }
    }
    
    for( y=0; y<game_level.fieldsY; y++ ) {
        char *fieldRow = field[y];
        char *fieldRowAbove = (y > 0) ? field[y-1] : 0;
        char *fieldRowBelow = (y < game_level.fieldsY-1) ? field[y+1] : 0;
        for( x=0; x<game_level.fieldsX; x++ ){
            // count neighboring bombs
            if(field[y][x] != (SBOMB | BHIDDEN)) {
                // observe edges
                if(x < game_level.fieldsX-1 ){ // right edge
                    if(fieldRow[x+1] == (SBOMB | BHIDDEN)) fieldRow[x]++;                       // right
                    if(fieldRowBelow && fieldRowBelow[x+1] == (SBOMB | BHIDDEN)) fieldRow[x]++; // bottom right
                    if(fieldRowAbove && fieldRowAbove[x+1] == (SBOMB | BHIDDEN)) fieldRow[x]++; // top right
                }
                if(x > 0 ){ // left edge
                    if(fieldRow[x-1] == (SBOMB | BHIDDEN)) fieldRow[x]++;                       // right
                    if(fieldRowBelow && fieldRowBelow[x-1] == (SBOMB | BHIDDEN)) fieldRow[x]++; // bottom right
                    if(fieldRowAbove && fieldRowAbove[x-1] == (SBOMB | BHIDDEN)) fieldRow[x]++; // top right
                }
                if(fieldRowBelow && fieldRowBelow[x] == (SBOMB | BHIDDEN)) fieldRow[x]++; // bottom
                if(fieldRowAbove && fieldRowAbove[x] == (SBOMB | BHIDDEN)) fieldRow[x]++; // top
            }
        }
    }
}


int getInput(void)
{
    static __near char fc;
    static __near char last = 0xff;
    register int c, b;

    if ((b = buttonState ^ 0xff)) {
        c = serialRaw;
        fc = frameCount + 16;
        if (last == 0xff) {           /* clean press */
            if (b == 0x10)            /* - report buttonStart like a key */
                return last = 0xef;
            if (c < 127) {            /* - looks like a key */
                if ((c+1) & c)        /* - also report type b codes as buttons */
                    buttonState = 0xff;
                return last = c;
            }
        }
        b &= 0xef;                   /* - ignore buttonStart */
        b = (-b) & b;                /* - pick one button only */
        if (b) {
            buttonState |= b;        /* - mark button as processed */
            return last = b ^ 0xff;  /* - return */
        }
    }
    if (serialRaw == 0xff) {         /* nothing pressed. */
        last = 0xff;                 /* - next is a clean press */
        return -1;
    }
    if (last != 0xff && (signed char)(frameCount - fc) >= 0) {
        fc = frameCount + 8;
        return last;                 /* autorepeat */
    }
    return -1;
}

#define ADDR0(cx)    (char*)(((8+TOP)<<8)+cx*6)  // screen address for top line
#define ADDR(cy,cx)  (char*)(((8+cy*8)<<8)+cx*6) // screen address for line cy

void cprint(char *addr, const char *s)
{
    _console_printchars(0x200a, addr, s, -1);
}

void cprintr(char *addr, const char *s)
{
    _console_printchars(0x30a, addr, s, 1);
    _console_printchars(0x200a, addr+6, s+1, -1);
}

void cprintu(register char *addr, register unsigned int x)
{
    char buffer[8];
    register char *s = utoa(x, buffer, 10);
    while (s > buffer+sizeof(buffer)-4)
        *--s = ' ';
    _console_printchars(0x20a, addr, s, 8);
}

int main()
{
    char cursorX, cursorY;       // cursor in the playing field

    char i, x1, y1, tx, ty; // help variables
    int x, y; // help variables

    bottonLevel = BEGINNER;
    setLevel(&game_level, bottonLevel);

    SYS_SetMode(2);

    for(;;){
        _console_clear(ADDR0(0), 0x3f38, 120-8-TOP);
        videoTop_v5 = 224;
        seconds = 0;
        markerCount = 0;
        gameOver = 0;
        newGame = 0;
        firstClick = 0;
        revealedFields = 0;
        seconds = 0;
        leftMargin = (160 - 6*game_level.fieldsX)/2;

        initialize();

        for( y=0; y<game_level.fieldsY; y++ )
            for( x=0; x<game_level.fieldsX; x++ )
                printSprite(field[y][x], x, y);
        
        // output info line
        _console_clear(ADDR(14,0), 0x030a, 8);
        cprintr(ADDR(14,1),"Beginner");
        cprintr(ADDR(14,10),"Advanced");
        cprintr(ADDR(14,19),"Expert");
        _console_clear(ADDR0(0), 0x030a, 8);
        cprint(ADDR0(1), "Bombs");

        videoTop_v5 = 2 * TOP;
        
        cursorX = 0;
        cursorY = 0;

        printCursor((char*)bigcursor, (char*)(cursorY*6-1+game_level.topMargin<<8) + 6 * cursorX-1 + leftMargin);

        while(!gameOver){

            switch(getInput()) {
                case BUTTON_START:  // blocked software reset
                    bottonLevel++;
                    if(bottonLevel > EXPERT) bottonLevel = BEGINNER;
                    gameOver = 1;
                    newGame = 1;
                    setLevel(&game_level, bottonLevel);
                break;
                case 'n': // start new game
                case 'N':
                    gameOver = 1;
                    newGame = 1;
                break;

                case 'b': // new game beginner
                case 'B':
                    gameOver = 1;
                    newGame = 1;
                    setLevel(&game_level, BEGINNER);
                break;
                case 'a': // new game advanced
                case 'A':
                    gameOver = 1;
                    newGame = 1;
                    setLevel(&game_level, ADVANCED);
                break;

                case 'e': // new game expert
                case 'E':
                    gameOver = 1;
                    newGame = 1;
                    setLevel(&game_level, EXPERT);
                break;
                case BUTTON_B:
                case 0x20: // set,unset marker with space
                    if((field[cursorY][cursorX] & BHIDDEN) == BHIDDEN){      // only on covered fields
                        if((field[cursorY][cursorX] & BMARKER) == BMARKER){
                            field[cursorY][cursorX] = field[cursorY][cursorX] & 0x1f;
                            markerCount--;
                        }else{
                            if(markerCount < game_level.numberBomb){                                // only so many as bombs
                                field[cursorY][cursorX] = field[cursorY][cursorX] | 0x20;
                                markerCount++;
                            }
                        }
                        restoreCursor((char*)bigcursor, (char*)(cursorY*6-1+game_level.topMargin<<8) + 6 * cursorX-1 + leftMargin);
                        printSprite((field[cursorY][cursorX]), cursorX, cursorY);
                        printCursor((char*)bigcursor, (char*)(cursorY*6-1+game_level.topMargin<<8) + 6 * cursorX-1 + leftMargin);
                    }
                break;

                case BUTTON_A:
                case 0x0a: // uncover game field with enter key
                    if(field[cursorY][cursorX] < 0x10) continue;  // is already uncovered
                    if(firstClick == 0){ // first click, start clock
                        firstClick = 1;
                        ticks = _clock();
                    }
                    if(field[cursorY][cursorX] < 0x20){              // marker protects field
                        if((field[cursorY][cursorX] & 0x0f) == SBOMB){
                            // game over
                            gameOver = 1;
                            field[cursorY][cursorX] = SBOMBTRIGGERED;
                            for( y=0; y<game_level.fieldsY; y++ ){              // uncover all hidden fields
                                for( x=0; x<game_level.fieldsX; x++ ){
                                    printSprite((field[y][x] & 0x0f), x, y);
                                }
                            }
                        }else{ // no bomb in the field
                            if(field[cursorY][cursorX] > 0x1f) markerCount--;  // remove incorrect marker ### before bug
                            field[cursorY][cursorX] = field[cursorY][cursorX]& 0x0f;
                            printSprite(field[cursorY][cursorX], cursorX, cursorY);
                            revealedFields++;
                            restoreCursor((char*)bigcursor, (char*)(cursorY*6-1+game_level.topMargin<<8) + 6 * cursorX-1 + leftMargin);
                            if(field[cursorY][cursorX] == SFREE) {
                                // field is empty, adjacent fields can be uncovered
                                queuePointer = 0;
                                queue[queuePointer] = (cursorY<<8) + cursorX;
                                queuePointer++;
                                while(queuePointer>0){
                                    queuePointer--;
                                    ty = queue[queuePointer]>>8;
                                    tx = queue[queuePointer] & 0xff;
                                    if(field[ty][tx] > 0x0f){
                                        if(field[ty][tx] > 0x1f) markerCount--;  // remove incorrect marker

                                        revealedFields++;
                                        field[ty][tx] = field[ty][tx] & 0x0f;
                                        printSprite(field[ty][tx], tx, ty);
                                    }
                                    // search neighboring fields
                                    for(y = -1; y < 2; y++){
                                        for(x = -1; x < 2; x++){
                                            // loop adjacent fields
                                            x1 = tx + x; y1 = ty + y;
                                            if((x1 < game_level.fieldsX) && (x1 >= 0) && (y1 < game_level.fieldsY) && (y1 >= 0) && (field[y1][x1]>0x0f)){
                                                // field lies in the array and is not uncovered
                                                if(field[y1][x1] > 0x1f) markerCount--;  // remove incorrect marker
                                                field[y1][x1] = field[y1][x1] & 0x0f;    // uncover field
                                                printSprite(field[y1][x1], x1, y1);      // draw revealed field
                                                revealedFields++;
                                                if(field[y1][x1] == SFREE){              // field has no neighbor bombs, add to queue
                                                    queue[queuePointer] = (y1<<8) + x1;
                                                    queuePointer++;
                                                    if(queuePointer > MAXQ) queuePointer = MAXQ; // prevent overflow
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        printCursor((char*)bigcursor, (char*)(cursorY*6-1+game_level.topMargin<<8) + 6 * cursorX-1 + leftMargin);
                    }
                break;

            case BUTTON_DOWN:
                if(cursorY < game_level.fieldsY-1){
                    restoreCursor((char*)bigcursor, (char*)(cursorY*6-1+game_level.topMargin<<8) + 6 * cursorX-1 + leftMargin);
                    cursorY++;
                    printCursor((char*)bigcursor, (char*)(cursorY*6-1+game_level.topMargin<<8) + 6 * cursorX-1 + leftMargin);
                }
                break;

            case BUTTON_UP:
                if(cursorY > 0){
                    restoreCursor((char*)bigcursor, (char*)(cursorY*6-1+game_level.topMargin<<8) + 6 * cursorX-1 + leftMargin);
                    cursorY--;
                    printCursor((char*)bigcursor, (char*)(cursorY*6-1+game_level.topMargin<<8) + 6 * cursorX-1 + leftMargin);
                }
                break;
                
            case BUTTON_LEFT:
                if(cursorX > 0){
                    restoreCursor((char*)bigcursor, (char*)(cursorY*6-1+game_level.topMargin<<8) + 6 * cursorX-1 + leftMargin);
                    cursorX--;
                    printCursor((char*)bigcursor, (char*)(cursorY*6-1+game_level.topMargin<<8) + 6 * cursorX-1 + leftMargin);
                }
                break;

            case BUTTON_RIGHT:
                if(cursorX < game_level.fieldsX-1){
                    restoreCursor((char*)bigcursor, (char*)(cursorY*6-1+game_level.topMargin<<8) + 6 * cursorX-1 + leftMargin);
                    cursorX++;
                    printCursor((char*)bigcursor, (char*)(cursorY*6-1+game_level.topMargin<<8) + 6 * cursorX-1 + leftMargin);
                }
                break;

            default:
                break;
            }

            // output of status line
            if(seconds<999) seconds = (_clock() - ticks) / 60; else seconds = 999;
            if(!firstClick) seconds = 0;
            i = game_level.numberBomb - markerCount;
            cprintu(ADDR0(6), i);
            if(seconds>999)
                seconds = 999;
            cprintu(ADDR0(22), seconds);
            if((revealedFields + game_level.numberBomb) == game_level.fields) gameOver = 1;
        }
        // game end
        if(!newGame) {
            if((revealedFields + game_level.numberBomb) == game_level.fields) {
                colors = 0x031c;
                _console_clear(ADDR0(0), colors, 8);
                _console_printchars(colors, ADDR0(3), "YOU are the winner!", 255);
            }else{
                colors = 0x0f03;
                _console_clear(ADDR0(0), colors, 8);
                _console_printchars(colors, ADDR0(2), ">>> You have lost <<<", 255);
            }
            _console_clear(ADDR(14,0), colors, 8);
            _console_printchars(colors, ADDR(14,1), "Hit any key for new game", 255);
            while (getInput() < 0)
                { }
        }
    }
}

/* Local Variables: */
/* mode: c */
/* c-basic-offset: 4 */
/* indent-tabs-mode: () */
/* End: */
