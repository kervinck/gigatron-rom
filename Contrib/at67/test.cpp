#include "graphics.h"
#include "test.h"


uint8_t getPixelGiga(uint8_t x, uint8_t y)
{
    x = x % GIGA_WIDTH;
    y = y % GIGA_HEIGHT;
    uint16_t address = GIGA_VRAM + x + (y <<8);
    return Cpu::getRAM(address);
}

void drawPixelGiga(uint8_t x, uint8_t y, uint8_t colour)
{
    x = x % GIGA_WIDTH;
    y = y % GIGA_HEIGHT;
    uint16_t address = GIGA_VRAM + x + (y <<8);
    Cpu::setRAM(address, colour);
}


namespace TestTetris
{
    #define TETRIS_XPOS     60
    #define TETRIS_YPOS     20
    #define TETRIS_XEXT     10
    #define TETRIS_YEXT     20
    #define NUM_TETROMINOES 7
    #define NUM_ROTATIONS   4
    #define TETROMINOE_SIZE 4
    #define MAX_LINES       4
    #define MAX_LEVEL       8


    enum BoardState {Clear=0, Blocked, GameOver};

    struct Tetromino
    {
        uint8_t _colour;
        uint8_t _pattern[NUM_ROTATIONS][4 + TETROMINOE_SIZE*2];
    };

    Tetromino I = 
    {
        0x3C,
        4, 1, 0, 1, 0, 1, 1, 1, 2, 1, 3, 1,
        1, 4, 2, 0, 2, 0, 2, 1, 2, 2, 2, 3,
        4, 1, 0, 2, 0, 2, 1, 2, 2, 2, 3, 2,
        1, 4, 1, 0, 1, 0, 1, 1, 1, 2, 1, 3,
    };
    Tetromino J = 
    {
        0x30,
        3, 2, 0, 0, 0, 0, 0, 1, 1, 1, 2, 1,
        2, 3, 1, 0, 1, 0, 2, 0, 1, 1, 1, 2,
        3, 2, 0, 1, 0, 1, 1, 1, 2, 1, 2, 2,
        2, 3, 0, 0, 1, 0, 1, 1, 0, 2, 1, 2
    };
    Tetromino L = 
    {
        0x0B,
        3, 2, 0, 0, 2, 0, 0, 1, 1, 1, 2, 1,
        2, 3, 1, 0, 1, 0, 1, 1, 1, 2, 2, 2,
        3, 2, 0, 1, 0, 1, 1, 1, 2, 1, 0, 2,
        2, 3, 0, 0, 0, 0, 1, 0, 1, 1, 1, 2
    };
    Tetromino O = 
    {
        0x0F,
        2, 2, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1,
        2, 2, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1,
        2, 2, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1,
        2, 2, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1
    };
    Tetromino S = 
    {
        0x0C,
        3, 2, 0, 0, 1, 0, 2, 0, 0, 1, 1, 1,
        2, 3, 1, 0, 1, 0, 1, 1, 2, 1, 2, 2,
        3, 2, 0, 1, 1, 1, 2, 1, 0, 2, 1, 2,
        2, 3, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2
    };
    Tetromino T = 
    {
        0x33,
        3, 2, 0, 0, 1, 0, 0, 1, 1, 1, 2, 1,
        2, 3, 1, 0, 1, 0, 1, 1, 2, 1, 1, 2,
        3, 2, 0, 1, 0, 1, 1, 1, 2, 1, 1, 2,
        2, 3, 0, 0, 1, 0, 0, 1, 1, 1, 1, 2
    };
    Tetromino Z = 
    {
        0x03,
        3, 2, 0, 0, 0, 0, 1, 0, 1, 1, 2, 1,
        2, 3, 1, 0, 2, 0, 1, 1, 2, 1, 1, 2,
        3, 2, 0, 1, 0, 1, 1, 1, 1, 2, 2, 2,
        2, 3, 0, 0, 1, 0, 0, 1, 1, 1, 0, 2
    };
    Tetromino tetrominoes[NUM_TETROMINOES] = {I, J, L, O, S, T, Z};
    int tetrisScore = 0, tetrisLevel = 0, scoreDelta = 0;
    int frameTickLevel = 60;
    int frameTick = frameTickLevel;
    int frameCount = 0;
    int x=TETRIS_XEXT/2, y = -1;
    int index = 0, rotation = 0;
    int w, h, u, v;
    int ox, oy, ov;
    int oindex, orotation;

    uint8_t getTetrisPixel(int tx, int ty)
    {
        tx *= 4;
        ty *= 4;
        return getPixelGiga(uint8_t(TETRIS_XPOS + tx), uint8_t(TETRIS_YPOS + ty));
    }

    void setTetrisPixel(int tx, int ty, uint8_t colour)
    {
        tx *= 4;
        ty *= 4;
        drawPixelGiga(uint8_t(TETRIS_XPOS + tx + 0), uint8_t(TETRIS_YPOS + ty + 0), colour);
        drawPixelGiga(uint8_t(TETRIS_XPOS + tx + 0), uint8_t(TETRIS_YPOS + ty + 1), colour);
        drawPixelGiga(uint8_t(TETRIS_XPOS + tx + 0), uint8_t(TETRIS_YPOS + ty + 2), colour);
        drawPixelGiga(uint8_t(TETRIS_XPOS + tx + 0), uint8_t(TETRIS_YPOS + ty + 3), colour);
        drawPixelGiga(uint8_t(TETRIS_XPOS + tx + 1), uint8_t(TETRIS_YPOS + ty + 0), colour);
        drawPixelGiga(uint8_t(TETRIS_XPOS + tx + 1), uint8_t(TETRIS_YPOS + ty + 1), colour);
        drawPixelGiga(uint8_t(TETRIS_XPOS + tx + 1), uint8_t(TETRIS_YPOS + ty + 2), colour);
        drawPixelGiga(uint8_t(TETRIS_XPOS + tx + 1), uint8_t(TETRIS_YPOS + ty + 3), colour);
        drawPixelGiga(uint8_t(TETRIS_XPOS + tx + 2), uint8_t(TETRIS_YPOS + ty + 0), colour);
        drawPixelGiga(uint8_t(TETRIS_XPOS + tx + 2), uint8_t(TETRIS_YPOS + ty + 1), colour);
        drawPixelGiga(uint8_t(TETRIS_XPOS + tx + 2), uint8_t(TETRIS_YPOS + ty + 2), colour);
        drawPixelGiga(uint8_t(TETRIS_XPOS + tx + 2), uint8_t(TETRIS_YPOS + ty + 3), colour);
        drawPixelGiga(uint8_t(TETRIS_XPOS + tx + 3), uint8_t(TETRIS_YPOS + ty + 0), colour);
        drawPixelGiga(uint8_t(TETRIS_XPOS + tx + 3), uint8_t(TETRIS_YPOS + ty + 1), colour);
        drawPixelGiga(uint8_t(TETRIS_XPOS + tx + 3), uint8_t(TETRIS_YPOS + ty + 2), colour);
        drawPixelGiga(uint8_t(TETRIS_XPOS + tx + 3), uint8_t(TETRIS_YPOS + ty + 3), colour);
    }

    void drawTetromino(int idx, int rot, int tx, int ty, uint8_t colour)
    {
        for(int i=0; i<TETROMINOE_SIZE; i++)
        {
            int xx = tx + tetrominoes[idx]._pattern[rot][4 + i*2];
            int yy = ty + tetrominoes[idx]._pattern[rot][5 + i*2];
            if(xx < 0  ||  xx >= TETRIS_XEXT) continue;
            if(yy < 0  ||  yy >= TETRIS_YEXT) continue;

            setTetrisPixel(uint8_t(xx), uint8_t(yy), colour);
        }
    }

    BoardState checkTetromino(int idx, int rot, int tx, int ty)
    {
        for(int i=0; i<TETROMINOE_SIZE; i++)
        {
            int xx = tx + tetrominoes[idx]._pattern[rot][4 + i*2];
            int yy = ty + tetrominoes[idx]._pattern[rot][5 + i*2];
            if(xx < 0  ||  xx >= TETRIS_XEXT) continue;
            if(yy < 0  ||  yy >= TETRIS_YEXT) continue;

            if(getTetrisPixel(uint8_t(xx), uint8_t(yy)))
            {
                if(y == 0) return GameOver;
                return Blocked;
            }
        }

        return Clear;
    }

    void updateScore(int lines)
    {
        static int scoring[MAX_LINES] = {10, 25, 50, 100};
        static int levelling[MAX_LEVEL] = {500, 500, 1000, 2000, 4000, 8000, 16000, 32000};

        scoreDelta += scoring[(lines - 1) & (MAX_LINES - 1)] * (tetrisLevel + 1);

        // Level up
        if(scoreDelta >= levelling[tetrisLevel & (MAX_LEVEL - 1)])
        {
            tetrisScore += scoreDelta;
            tetrisLevel++;
            frameTickLevel = std::max(60 - 5*tetrisLevel, 15);
            frameTick = frameTickLevel;
            scoreDelta = 0;
        }

        fprintf(stderr, "Tetris: score : %06d  level : %d  frameTickLevel : %d  lines : %d\n", tetrisScore + scoreDelta, tetrisLevel, frameTickLevel, lines);
    }

    int checkLines(void)
    {
        int lines = 0;

        for(int ii=0; ii<MAX_LINES; ii++)
        {
            for(int ll=TETRIS_YEXT-1; ll>=0; ll--)
            {
                bool line = true;
                for(int kk=0; kk<TETRIS_XEXT; kk++) line &= getTetrisPixel(kk, ll) > 0;
                if(line)
                {
                    lines++;

                    for(int nn=ll; nn>0; nn--)
                    {
                        for(int mm=0; mm<TETRIS_XEXT; mm++) setTetrisPixel(mm, nn, getTetrisPixel(mm, nn-1));
                    }
                    for(int kk=0; kk<TETRIS_XEXT; kk++) setTetrisPixel(kk, 0, 0x00);
                }
            }
        }

        if(lines) updateScore(lines);

        return lines;
    }

    void saveTetrominoState(void)
    {
        ox = x;
        oy = y;
        ov = tetrominoes[index]._pattern[rotation][3];
        oindex = index;
        orotation = rotation;
    }

    void updateTetromino(void)
    {
        w = tetrominoes[index]._pattern[rotation][0];
        h = tetrominoes[index]._pattern[rotation][1];
        u = tetrominoes[index]._pattern[rotation][2];
        v = tetrominoes[index]._pattern[rotation][3];
    }

    int spawnTetromino(void)
    {
        index = rand() % NUM_TETROMINOES;
        rotation = rand() & (NUM_ROTATIONS-1);
        x = rand() % (TETRIS_XEXT - (w - 1)) - u;

        updateTetromino();
        saveTetrominoState();
        return checkLines();
    }
 
    void shakeScreen(int lines)
    {
        static int frameCnt = 0;
        static int strength = 0;

        if(lines)
        {
            frameCnt = 1;
            strength = lines;
        }

        if(frameCnt)
        {
            int screenShake = rand() % 4;
            switch(screenShake)
            {
                case 0: Cpu::setRAM(0x0101, uint8_t(strength));                                                                           break;
                case 1: Cpu::setRAM(0x0101, uint8_t(0 - strength));                                                                       break;
                case 2: for(int i=0x0100; i<0x01EE; i+=2) Cpu::setRAM(uint16_t(i), uint8_t(0x08 + (i-0x0100)/2 + strength));              break;
                case 3: for(int i=0x0100; i<0x01EE; i+=2) Cpu::setRAM(uint16_t(i), uint8_t(0x08 + (i-0x0100)/2 + uint8_t(0 - strength))); break;

                default: break;
            }
            
            if(++frameCnt >= 20) //strength * 10)
            {
                frameCnt = 0;
                Cpu::setRAM(0x0101, 0x00);
                for(int i=0x0100; i<0x01EF; i+=2) Cpu::setRAM(uint16_t(i), uint8_t(0x08 + (i-0x0101)/2));
            }
        }
    }

    void tetris(void)
    {
        static bool firstTime = true;
        if(firstTime  &&  Cpu::getClock() > 10000000)
        {
            firstTime = false;
            for(int l=0; l<TETRIS_YEXT; l++)
                for(int k=0; k<TETRIS_XEXT; k++) setTetrisPixel(k, l, 0x00);
        }

        bool refresh = false;

        saveTetrominoState();

        SDL_Event event;
        SDL_PollEvent(&event);

        switch(event.type)
        {
            case SDL_KEYDOWN:
            {
                if(y == -1) break;

                switch(event.key.keysym.sym)
                {
                    case SDLK_LEFT:  x--; refresh = true;                                           break;
                    case SDLK_RIGHT: x++; refresh = true;                                           break;
                    case SDLK_DOWN:  frameTick = 2;                                                 break;

                    case SDLK_UP:
                    {
                        static int rotation_old = rotation;
                        rotation = (rotation + 1) & (NUM_ROTATIONS-1);
                        updateTetromino();
                        if(y > TETRIS_YEXT - h - v)
                        {
                            rotation = rotation_old;
                            updateTetromino();
                        }
                        else
                        {
                            refresh = true;
                        }
                    }
                    break;

                    case SDLK_ESCAPE:
                    {
                        Cpu::shutdown();
                        exit(0);
                    }

                    default: break;
                }
            }
            break;

            case SDL_KEYUP:
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_DOWN: frameTick = frameTickLevel; break;

                    default: break;
                }
            }
            break;

            default: break;
        }

        int lines = 0;
        if(refresh || ++frameCount >= frameTick)
        {
            // Erase old tetromino
            drawTetromino(oindex, orotation, ox, oy-ov, 0x00); 

            updateTetromino();

            if(x < 0 - u) x = 0 - u;
            if(x > TETRIS_XEXT - w - u) x = TETRIS_XEXT - w - u;

            // Update tetromino
            if(frameCount >= frameTick)
            {
                frameCount = 0;

                // Gravity
                if(++y > TETRIS_YEXT - h)
                {
                    // Hit ground
                    drawTetromino(index, rotation, x, y-v-1, tetrominoes[index]._colour);
                    y = -1;
                    lines = spawnTetromino();
                }
            }

            BoardState boardState = checkTetromino(index, rotation, x, y-v);
            switch(boardState)
            {
                case Clear:   drawTetromino(index, rotation, x, y-v, tetrominoes[index]._colour); break;

                case Blocked:
                {
                    if(!refresh)
                    {
                        drawTetromino(index, rotation, x, y-v-1, tetrominoes[index]._colour);
                        y = -1;
                        lines = spawnTetromino();
                    }
                    else
                    {
                        drawTetromino(oindex, orotation, ox, oy-ov, tetrominoes[oindex]._colour);
                        x = ox, y = oy, v = ov, rotation = orotation, index = oindex;
                    }
                }
                break;

                case GameOver:
                {
                    // Game over
                    scoreDelta = 0;
                    tetrisScore = 0;
                    tetrisLevel = 0;
                    frameTickLevel = 60;
                    frameTick = frameTickLevel;
                    for(int l=0; l<TETRIS_YEXT; l++)
                        for(int k=0; k<TETRIS_XEXT; k++) setTetrisPixel(k, l, 0x00);
                    fprintf(stderr, "Tetris GAME OVER...\n");
                }
                break;

                default: break;
            }
        }

        shakeScreen(lines);
    }
}


namespace TestLife
{
    void lifePixel(uint8_t x, uint8_t y, uint32_t colour)
    {
        uint32_t screen = x + y*SCREEN_WIDTH;
        Graphics::getPixels()[screen] = colour*0xFFFFFFFF;
    }

#define LIFE_WIDTH  2000
#define LIFE_HEIGHT 2000
    void life(bool initialise)
    {
        static uint8_t buffers[2][LIFE_HEIGHT][LIFE_WIDTH];
        static uint8_t lut[9] = {0, 0, 0, 1, 0, 0, 0, 0, 0};
        static int index = 0;
        static bool initialised = false;

        if(initialise)
        {
            initialised = true;

            for(int j=0; j<LIFE_HEIGHT; j++)
                for(int i=0; i<LIFE_WIDTH; i++)
                    lifePixel(uint8_t(i), uint8_t(j), 0);

            for(int k=0; k<2; k++)
                for(int j=0; j<LIFE_HEIGHT; j++)
                    for(int i=0; i<LIFE_WIDTH; i++)
                        buffers[k][j][i] = 0;
            
            // Gliders            
            for(int i=0; i<8; i+=4)
            {
                buffers[0][100][100+i] = 1; buffers[0][101][100+i] = 1; buffers[0][102][100+i] = 1; buffers[0][102][99+i] = 1; buffers[0][101][98+i] = 1;
                lifePixel(uint8_t(100+i), 100, 1); lifePixel(uint8_t(100+i), 101, 1); lifePixel(uint8_t(100+i), 102, 1); lifePixel(uint8_t(99+i), 102, 1); lifePixel(uint8_t(98+i), 101, 1);
            }

            index = 0;
        }

        if(initialised)
        {      
            for(int j=1; j<LIFE_HEIGHT-1; j++)
            {
                for(int i=1; i<LIFE_WIDTH-1; i++)
                {
                    lut[2] = buffers[index][j][i];
                    int count = buffers[index][j-1][i-1] + buffers[index][j-1][i] + buffers[index][j-1][i+1] + buffers[index][j][i+1] + buffers[index][j+1][i+1] + buffers[index][j+1][i] + buffers[index][j+1][i-1] + buffers[index][j][i-1];
                    buffers[index ^ 1][j][i] = lut[count];
                    if(i < 256  &&  j < 256) lifePixel(uint8_t(i), uint8_t(j), lut[count]);
                }
            }

            index ^= 1;
        }
    }

    void life1(bool initialise)
    {
        static uint8_t buffers[2][LIFE_HEIGHT][LIFE_WIDTH];
        static uint8_t lut[9] = {0, 0, 0, 1, 0, 0, 0, 0, 0};
        static bool initialised = false;

        if(initialise)
        {
            initialised = true;

            for(int j=0; j<LIFE_HEIGHT; j++)
                for(int i=0; i<LIFE_WIDTH; i++)
                    lifePixel(uint8_t(i), uint8_t(j), 0);

            for(int k=0; k<2; k++)
                for(int j=0; j<LIFE_HEIGHT; j++)
                    for(int i=0; i<LIFE_WIDTH; i++)
                        buffers[k][j][i] = 0;

            // Gliders            
            for(int i=0; i<8; i+=4)
            {
                buffers[0][100][100+i] = 1; buffers[0][101][100+i] = 1; buffers[0][102][100+i] = 1; buffers[0][102][99+i] = 1; buffers[0][101][98+i] = 1;
                lifePixel(uint8_t(100+i), 100, 1); lifePixel(uint8_t(100+i), 101, 1); lifePixel(uint8_t(100+i), 102, 1); lifePixel(uint8_t(99+i), 102, 1); lifePixel(uint8_t(98+i), 101, 1);
            }
        }

        if(initialised)
        {        
            for(int j=1; j<LIFE_HEIGHT-1; j++)
            {
                for(int i=1; i<LIFE_WIDTH-1; i++)
                {
                    // Increment neighbour counts
                    if(buffers[0][j][i] == 1)
                    {
                        buffers[1][j-1][i-1]++; buffers[1][j-1][i]++; buffers[1][j-1][i+1]++; buffers[1][j][i+1]++; buffers[1][j+1][i+1]++; buffers[1][j+1][i]++; buffers[1][j+1][i-1]++; buffers[1][j][i-1]++;
                    }
                }
            }

            for(int j=1; j<LIFE_HEIGHT-1; j++)
            {
                for(int i=1; i<LIFE_WIDTH-1; i++)
                {
                    lut[2] = buffers[0][j][i];
                    int cell = lut[buffers[1][j][i]];
                    buffers[1][j][i] = 0;
                    buffers[0][j][i] = uint8_t(cell);
                    if(i < 256  &&  j < 256) lifePixel(uint8_t(i), uint8_t(j), cell);
                }
            }
        }
    }
}


namespace TestMandelbrot
{
    void mandelbrot(void)
    {
        const uint8_t colours[16] = {0x01, 0x02, 0x03, 0x07, 0x0b, 0x0f, 0x0e, 0x0d, 0x0c, 0x3c, 0x38, 0x34, 0x30, 0x20, 0x10, 0x00};
        const int16_t xmin = -100;
        const int16_t xmax =  60;
        const int16_t ymin = -60;
        const int16_t ymax =  60;
        const int16_t dx = (xmax-xmin)/160;
        const int16_t dy = (ymax-ymin)/120;
 
        int16_t cy = ymin;
        for(int16_t py=8; py<128; py++)
        {
            int16_t cx = xmin;
            for(int16_t px=0; px<160; px++)
            {
                int16_t x=0, y=x, x2=y, y2=x2;
        
                int colour = 0;
                for(int16_t c=0; c<=15; c++)
                {
                    colour = c;

                    x2 = int16_t(x*x) >> 5;
                    y2 = int16_t(y*y) >> 5;
                    if(int16_t(x2+y2) > 128) break;
            
                    y = (int16_t(x*y) >> 4) + cy;
                    x = int16_t(x2 - y2 + cx);
                }

                Cpu::setRAM((py <<8) + px, colours[colour]);
                cx = cx + dx;
            }

            cy = cy + dy;
        }
    }
}


namespace TestDrawLine
{
    void drawLineGiga(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t colour)
    {
        int16_t dx = int16_t(abs(x2 - x1));
        int16_t sx = (x1 < x2) ? 1 : -1;
        int16_t dy = int16_t(abs(y2 - y1));
        int16_t sy = (y1 < y2) ? 1 : -1;
        int16_t err = ((dx > dy) ? dx : -dy) >>1;
        int16_t e2;

        for(;;)
        {
            drawPixelGiga(uint8_t(x1), uint8_t(y1), colour);
            if(x1 == x2  &&  y1 == y2) break;
            e2 = err;
            if(e2 >-dx)
            {
                err -= dy;
                x1 += sx;
            }
            if(e2 < dy)
            {
                err += dx;
                y1 += sy;
            }
        }
    }

    void drawLineGiga1(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t colour)
    {
        int16_t sx = x2 - x1;
        int16_t sy = y2 - y1;
        int h = sy;
        int16_t dx1 = 1, dx2 = 1, dy1 = 1, dy2 = 0;

        if(sx & 0x8000)
        {
            dx1 = -1;
            dx2 = -1;
            sx = 0 - sx;
        }

        if(sy & 0x8000)
        {
            dy1 = -1;
            sy = 0 - sy;
            if(sx < sy) dy2 = -1;
        }

        if(sx < sy) 
        {
            dx2 = 0;
            std::swap(sx, sy);
            if(h > 0) dy2 = 1;
        }

        int16_t numerator = sx >> 1;
        int16_t xy1 = x1 | (y1<<8);
        int16_t xy2 = x2 | (y2<<8);
        int16_t dxy1 = dx1 + (dy1<<8);
        int16_t dxy2 = dx2 + (dy2<<8);

        for(uint16_t i=0; i<=sx/2; i++) 
        {
            drawPixelGiga(uint8_t(xy1), uint8_t(xy1>>8), colour);
            drawPixelGiga(uint8_t(xy2), uint8_t(xy2>>8), colour);
            numerator += sy;
            if(numerator > sx) 
            {
                numerator -= sx;
                xy1 += dxy1;
                xy2 -= dxy1;
            }
            else
            {
                xy1 += dxy2;
                xy2 -= dxy2;
            }
        }     
    }
}


namespace TestMath
{
    void fastMath(void)
    {
        for(int i=0; i<32768; i++)
        {
#if 1
            // Mod5
            int16_t j = int16_t(i); //int16_t(rand() % 32767);
            int16_t a = (j >> 8) + (j & 0xFF);   /* sum base 2**8 digits */
            a = (a >>  4) + (a & 0xF);    /* sum base 2**4 digits */
            if (a > 14) a = a - 15;
            if (a > 9) a = a - 10;
            if (a > 4) a = a - 5;
            //(a == j%5) ? fprintf(stderr, "%d : %d %d\n", j, a, j%5) : fprintf(stderr, "%d : %d %d ***\n", j, a, j%5);
            if(a != j%5) fprintf(stderr, "%d : %d %d ***\n", j, a, j%5);
#else
            // Div5
            int16_t j = int16_t(i); //int16_t(rand() % 255);
            int16_t a = j - j/64; // error term
            a = (a<<5) + (a<<4) + (a<<2); // * 52
            a = (a>>8); // / 256
            (a == j/5) ? fprintf(stderr, "%d : %d %d\n", j, a, j/5) : fprintf(stderr, "%d : %d %d ***\n", j, a, j/5);
#endif
        }
    }
}
