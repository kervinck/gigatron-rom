'normally sprites must be horizontally divisible by 6, but by specifying an overlap value the last column
'of a sprite can be pushed closer to the rest of the sprite, (works with flipped sprites as well)
const SPRITE_OVERLAP = 3 '12x9 sprites displayed as 9x9

const BlinkyUp = 0
load sprite, ../../../res/image/PucMon//Blinky_u0.tga, BlinkyUp + 0, NoFlip, SPRITE_OVERLAP

const BlinkyDn = 1
load sprite, ../../../res/image/PucMon//Blinky_d0.tga, BlinkyDn + 0, NoFlip, SPRITE_OVERLAP

const BlinkyLt = 2
load sprite, ../../../res/image/PucMon/Blinky_l0.tga, BlinkyLt + 0, NoFlip, SPRITE_OVERLAP

const BlinkyRt = 3 'instance and hardware flip, (native code), in the X direction to save memory
load sprite, ../../../res/image/PucMon/Blinky_l0.tga, BlinkyRt + 0, FlipX, SPRITE_OVERLAP

const PinkyUp = 4
load sprite, ../../../res/image/PucMon/Pinky_u0.tga, PinkyUp + 0, NoFlip, SPRITE_OVERLAP

const PinkyDn = 5
load sprite, ../../../res/image/PucMon/Pinky_d0.tga, PinkyDn + 0, NoFlip, SPRITE_OVERLAP

const PinkyLt = 6
load sprite, ../../../res/image/PucMon/Pinky_l0.tga, PinkyLt + 0, NoFlip, SPRITE_OVERLAP

const PinkyRt = 7 'instance and hardware flip, (native code), in the X direction to save memory
load sprite, ../../../res/image/PucMon/Pinky_l0.tga, PinkyRt + 0, FlipX, SPRITE_OVERLAP

const InkyUp = 8
load sprite, ../../../res/image/PucMon/Inky_u0.tga, InkyUp + 0, NoFlip, SPRITE_OVERLAP

const InkyDn = 9
load sprite, ../../../res/image/PucMon/Inky_d0.tga, InkyDn + 0, NoFlip, SPRITE_OVERLAP

const InkyLt = 10
load sprite, ../../../res/image/PucMon/Inky_l0.tga, InkyLt + 0, NoFlip, SPRITE_OVERLAP

const InkyRt = 11 'instance and hardware flip, (native code), in the X direction to save memory
load sprite, ../../../res/image/PucMon/Inky_l0.tga, InkyRt + 0, FlipX, SPRITE_OVERLAP

const ClydeUp = 12
load sprite, ../../../res/image/PucMon/Clyde_u0.tga, ClydeUp + 0, NoFlip, SPRITE_OVERLAP

const ClydeDn = 13
load sprite, ../../../res/image/PucMon/Clyde_d0.tga, ClydeDn + 0, NoFlip, SPRITE_OVERLAP

const ClydeLt = 14
load sprite, ../../../res/image/PucMon/Clyde_l0.tga, ClydeLt + 0, NoFlip, SPRITE_OVERLAP

const ClydeRt = 15 'instance and hardware flip, (native code), in the X direction to save memory
load sprite, ../../../res/image/PucMon/Clyde_l0.tga, ClydeRt + 0, FlipX, SPRITE_OVERLAP

const PucUp = 16
load sprite, ../../../res/image/PucMon/Puc_u0.tga, PucUp + 0, NoFlip, SPRITE_OVERLAP
load sprite, ../../../res/image/PucMon/Puc_u1.tga, PucUp + 1, NoFlip, SPRITE_OVERLAP
load sprite, ../../../res/image/PucMon/Puc_u2.tga, PucUp + 2, NoFlip, SPRITE_OVERLAP
load sprite, ../../../res/image/PucMon/Puc_u3.tga, PucUp + 3, NoFlip, SPRITE_OVERLAP

const PucDn = 20 'instance and hardware flip, (native code), in the Y direction to save memory
load sprite, ../../../res/image/PucMon/Puc_u0.tga, PucDn + 0, FlipY, SPRITE_OVERLAP
load sprite, ../../../res/image/PucMon/Puc_u1.tga, PucDn + 1, FlipY, SPRITE_OVERLAP
load sprite, ../../../res/image/PucMon/Puc_u2.tga, PucDn + 2, FlipY, SPRITE_OVERLAP
load sprite, ../../../res/image/PucMon/Puc_u3.tga, PucDn + 3, FlipY, SPRITE_OVERLAP

const PucLt = 24
load sprite, ../../../res/image/PucMon/Puc_l0.tga, PucLt + 0, NoFlip, SPRITE_OVERLAP
load sprite, ../../../res/image/PucMon/Puc_l1.tga, PucLt + 1, NoFlip, SPRITE_OVERLAP
load sprite, ../../../res/image/PucMon/Puc_l2.tga, PucLt + 2, NoFlip, SPRITE_OVERLAP
load sprite, ../../../res/image/PucMon/Puc_l3.tga, PucLt + 3, NoFlip, SPRITE_OVERLAP

const PucRt = 28 'instance and hardware flip, (native code), in the X direction to save memory
load sprite, ../../../res/image/PucMon/Puc_l0.tga, PucRt + 0, FlipX, SPRITE_OVERLAP
load sprite, ../../../res/image/PucMon/Puc_l1.tga, PucRt + 1, FlipX, SPRITE_OVERLAP
load sprite, ../../../res/image/PucMon/Puc_l2.tga, PucRt + 2, FlipX, SPRITE_OVERLAP
load sprite, ../../../res/image/PucMon/Puc_l3.tga, PucRt + 3, FlipX, SPRITE_OVERLAP

const ScaredUp = 32
load sprite, ../../../res/image/PucMon/Scared_u0.tga, ScaredUp + 0, NoFlip, SPRITE_OVERLAP

const ScaredDn = 33
load sprite, ../../../res/image/PucMon/Scared_d0.tga, ScaredDn + 0, NoFlip, SPRITE_OVERLAP

const ScaredLt = 34
load sprite, ../../../res/image/PucMon/Scared_u0.tga, ScaredLt + 0, NoFlip, SPRITE_OVERLAP

const ScaredRt = 35
load sprite, ../../../res/image/PucMon/Scared_u0.tga, ScaredRt + 0, FlipX, SPRITE_OVERLAP

const EyesUp = 36
load sprite, ../../../res/image/PucMon/Eyes_d.tga, EyesUp, FlipY, 0

const EyesDn = 37
load sprite, ../../../res/image/PucMon/Eyes_d.tga, EyesDn, NoFlip, 0

const EyesLt = 38
load sprite, ../../../res/image/PucMon/Eyes_l.tga, EyesLt, NoFlip, 0

const EyesRt = 39
load sprite, ../../../res/image/PucMon/Eyes_l.tga, EyesRt, FlipX, 0

const Tunnel = 40
load sprite, ../../../res/image/PucMon/Black12x9.tga, Tunnel, NoFlip, 0

const Pill = 41
const Erase6x6 = Pill + 1
load sprite, ../../../res/image/PucMon/Pill.tga,     Pill + 0, NoFlip, 0
load sprite, ../../../res/image/PucMon/Black6x6.tga, Pill + 1, NoFlip, 0

const Erase12x9 = 43
load sprite, ../../../res/image/PucMon/Black12x9.tga, Erase12x9, NoFlip, SPRITE_OVERLAP

const Life = 44
load sprite, ../../../res/image/PucMon/Life.tga, Life, NoFlip, 0

const Level = 45
load sprite, ../../../res/image/PucMon/Level.tga, Level, NoFlip, 0

const Digit = 46
load sprite, ../../../res/image/PucMon/Zero.tga,  Digit + 0, NoFlip, 0
load sprite, ../../../res/image/PucMon/One.tga,   Digit + 1, NoFlip, 0
load sprite, ../../../res/image/PucMon/Two.tga,   Digit + 2, NoFlip, 0
load sprite, ../../../res/image/PucMon/Three.tga, Digit + 3, NoFlip, 0
load sprite, ../../../res/image/PucMon/Four.tga,  Digit + 4, NoFlip, 0
load sprite, ../../../res/image/PucMon/Five.tga,  Digit + 5, NoFlip, 0
load sprite, ../../../res/image/PucMon/Six.tga,   Digit + 6, NoFlip, 0
load sprite, ../../../res/image/PucMon/Seven.tga, Digit + 7, NoFlip, 0
load sprite, ../../../res/image/PucMon/Eight.tga, Digit + 8, NoFlip, 0
load sprite, ../../../res/image/PucMon/Nine.tga,  Digit + 9, NoFlip, 0

const Blinky_1 = 56
load sprite, ../../../res/image/PucMon/Blinky_1.tga, Blinky_1, NoFlip, SPRITE_OVERLAP

const Pinky_1 = 57
load sprite, ../../../res/image/PucMon/Pinky_1.tga, Pinky_1, NoFlip, SPRITE_OVERLAP

const Inky_1 = 58
load sprite, ../../../res/image/PucMon/Inky_1.tga, Inky_1, NoFlip, SPRITE_OVERLAP

const Clyde_1 = 59
load sprite, ../../../res/image/PucMon/Clyde_1.tga, Clyde_1, NoFlip, SPRITE_OVERLAP

const Scared_1 = 60
load sprite, ../../../res/image/PucMon/Scared_1.tga, Scared_1, NoFlip, SPRITE_OVERLAP


dim maze%(23, 27) = {&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,
                     &hF0,&hF0,&hF0,&hF0,&hF0,&h01,&h01,&h01,&h01,&h01,&hD1,&h01,&h01,&h01,&h01,&h01,&h01,&hF0,&hF0,&h01,&h01,&h01,&h01,&h01,
                     &h01,&hD1,&h01,&h01,&h01,&h01,&h01,&hF0,&hF0,&h02,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,&hF0,&h01,
                     &hF0,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&h02,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&hF0,
                     &h01,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,&hF0,&hD1,&h01,&h01,&h01,&h01,&hD1,&h01,
                     &h01,&hD1,&h01,&h01,&hD1,&h01,&h01,&hD1,&h01,&h01,&hD1,&h01,&h01,&hD1,&h01,&h01,&h01,&h01,&hD1,&hF0,&hF0,&h01,&hF0,&hF0,
                     &hF0,&hF0,&h01,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,
                     &hF0,&h01,&h01,&h01,&h01,&h01,&hD1,&hF0,&hF0,&h01,&h01,&h01,&h01,&hF0,&hF0,&h01,&h01,&h01,&h01,&hF0,&hF0,&hD1,&h01,&h01,
                     &h01,&h01,&h01,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&hF0,&h00,&hF0,&hF0,&h00,&hF0,&hF0,&hF0,&hF0,
                     &hF0,&h01,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,&hF0,&h00,&h00,&h00,&hD0,&hB0,&h00,&hD0,
                     &h00,&h00,&h00,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,&hF0,&h00,&hF0,&hF0,
                     &hF0,&hE0,&hE0,&hF0,&hF0,&hF0,&h00,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,
                     &hF0,&h00,&hF0,&h00,&h00,&hC0,&h00,&h00,&h00,&hF0,&h00,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hA0,&h90,&h90,&h90,
                     &h90,&h00,&hD1,&h00,&h00,&hD0,&hF0,&h00,&h00,&h00,&h00,&h00,&h00,&hF0,&hD0,&h00,&h00,&hD1,&h00,&h90,&h90,&h90,&h90,&hA0,
                     &hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,&hF0,&h00,&hF0,&h00,&h00,&h00,&h00,&h00,&h00,&hF0,&h00,&hF0,&hF0,&h01,&hF0,&hF0,
                     &hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,&hF0,&h00,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&h00,&hF0,
                     &hF0,&h01,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,&hF0,&hD0,&h00,&h00,&h00,&h00,&h00,&h00,
                     &h00,&h00,&hD0,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,&hF0,&h00,&hF0,&hF0,
                     &hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&h00,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&h01,&h01,&h01,&h01,&h01,&hD1,&h01,
                     &h01,&hD1,&h01,&h01,&h01,&hF0,&hF0,&h01,&h01,&h01,&hD1,&h01,&h01,&hD1,&h01,&h01,&h01,&h01,&h01,&hF0,&hF0,&h01,&hF0,&hF0,
                     &hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,
                     &hF0,&h02,&h01,&h01,&hF0,&hF0,&hD1,&h01,&h01,&hD1,&h01,&h01,&hD1,&h00,&h00,&hD1,&h01,&h01,&hD1,&h01,&h01,&hD1,&hF0,&hF0,
                     &h01,&h01,&h02,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,&hF0,&h01,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,
                     &hF0,&h01,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&h01,&h01,&hD1,&h01,&h01,&h01,&hF0,&hF0,&h01,&h01,&h01,&h01,&hF0,&hF0,&h01,
                     &h01,&h01,&h01,&hF0,&hF0,&h01,&h01,&h01,&hD1,&h01,&h01,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,
                     &h01,&hF0,&hF0,&h01,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&h01,&hF0,&hF0,&h01,&h01,&h01,&h01,&h01,&h01,&h01,
                     &h01,&h01,&h01,&h01,&hD1,&h01,&h01,&hD1,&h01,&h01,&h01,&h01,&h01,&h01,&h01,&h01,&h01,&h01,&h01,&hF0,&hF0,&hF0,&hF0,&hF0,
                     &hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0,&hF0}

const MUSIC_INTRO = &h31A0
def byte(&h31a0) =  &h90, &h47, &h91, &h23, &h04, &h80, &h04, &h90, &h53, &h81, &h05, &h80, &h04, &h90, &h4e, &h04,
def byte         =  &h80, &h04, &h90, &h4b, &h91, &h2f, &h05, &h80, &h04, &h90, &h53, &h91, &h23, &h04, &h90, &h4e,
def byte         =  &h04, &h80, &h81, &h09, &h90, &h4b, &h08, &h91, &h2f, &h05, &h80, &h04, &h90, &h48, &h91, &h24,
def byte         =  &h04, &h80, &h04, &h90, &h54, &h81, &h05, &h80, &h04, &h90, &h4f, &h04, &h80, &h04, &h90, &h4c,
def byte         =  &h91, &h30, &h05, &h80, &h04, &h90, &h54, &h91, &h24, &h04, &h90, &h4f, &h04, &h80, &h81, &h09,
def byte         =  &h90, &h4c, &h08, &h91, &h30, &h05, &h80, &h04, &h90, &h47, &h91, &h23, &h04, &hd0, &ha0, &h32,

def byte(&h32a0) =  &h80, &h04, &h90, &h53, &h81, &h05, &h80, &h04, &h90, &h4e, &h04, &h80, &h04, &h90, &h4b, &h91,
def byte         =  &h2f, &h05, &h80, &h04, &h90, &h53, &h91, &h23, &h04, &h90, &h4e, &h04, &h80, &h81, &h09, &h90,
def byte         =  &h4b, &h08, &h91, &h2f, &h04, &h80, &h05, &h90, &h4b, &h91, &h2a, &h04, &h90, &h4c, &h04, &h90,
def byte         =  &h4d, &h81, &h04, &h80, &h05, &h90, &h4d, &h91, &h2c, &h04, &h90, &h4e, &h04, &h90, &h4f, &h81,
def byte         =  &h04, &h80, &h05, &h90, &h4f, &h91, &h2e, &h04, &h90, &h50, &h04, &h90, &h51, &h81, &h04, &h80,
def byte         =  &h05, &h90, &h53, &h91, &h2f, &h08, &h80, &h81, &hd0, &h00, &h00,

'audio waveforms are byte interlaced of size 64 bytes each, wave0=&h0700, wave1=&h0701, wave2=&h0702, wave3=&h0703
'overwrite waveform 0 in audio memory, (chase siren)
def byte(&h0700, x, 0.0, 360.0, 64, 4) = sin(x)*31.0 + 32.0

'overwrite waveform 1 in audio memory, (eat dots)
load wave, ../../../res/audio/PucMon/eat.gtwav, &h0701, 4

'overwrite waveform 3 in audio memory, (puc death)
load wave, ../../../res/audio/PucMon/death.gtwav, &h0703, 4

const ORIGIN_X = 2
const ORIGIN_Y = 2
const SCORE_X = 112
const SCORE_Y = 65
const LEVEL_X = 137
const LEVEL_Y = 53
const HIGH_X = 112
const HIGH_Y = 40
const LIFE_X = 137
const LIFE_Y = 109

const PILL_RT = @maze( 2, 26)
const PILL_LT = @maze( 2,  1)
const PILL_LB = @maze(18,  1)
const PILL_RB = @maze(18, 26)

const WALL   = &hF0
const DOOR   = &hE0
const JUNC   = &hD0
const EXIT   = &hC0
const ENTER  = &hB0
const TUNNEL = &hA0
const SLOW   = &h90
const RPILL  = &h04
const RDOT   = &h03
const PILL   = &h02
const DOT    = &h01

dim MAZE_BORDER(6)   = 0, 115, 255
dim MAZE_OUTER_T(14) = 25, 0, 0, -15, -25, 0, 0, -35, 65, 0, 0, 15, 2, 0, 255
dim MAZE_OUTER_B(16) = -68, 0, 0, -20, 9, 0, -9, 0, 0, -20, 25, 0, 0, -15, -25, 0, 255
dim MAZE_TJUNC_0(10) = 25, 0, 0, -10, 5, 0, 0, 10, 15, 0, 255
dim MAZE_TJUNC_1(6)  = -2, 0, 0, -10, -15, 0, 255
dim MAZE_CAGE(6)     = -10, 0, 0, 20, 17, 0, 255
dim MAZE_VERT_0(8)   = 0, 15, -5, 0, 0, -15, 5, 0, 255
dim MAZE_VERT_1(8)   = 5, 0, 0, 25, -5, 0, 0, -25, 255
dim MAZE_CORNER(8)   = 10, 0, 0, 10, 5, 0, 0, -10, 255
dim MAZE_HORIZ_0(8)  = 15, 0, 0, 5, -15, 0, 0, -5, 255
dim MAZE_HORIZ_1(8)  = 20, 0, 0, 5, -20, 0, 0, -5, 255
dim MAZE_LINE_0(2)   = 15, 0, 255
dim MAZE_LINE_1(2)   = 20, 0, 255

'LUT containing maze piece locations
const NUM_MAZE_PIECES = 36
dim MAZE_PIECES(NUM_MAZE_PIECES - 1) = {0+2*256, 1+2*256, 2+2*256, 157+2*256, 158+2*256, 159+2*256,
                                        ORIGIN_X+((50+ORIGIN_Y)*256), (135+ORIGIN_X)+((50+ORIGIN_Y)*256),
                                        (68+ORIGIN_X)+((115+ORIGIN_Y)*256), (67+ORIGIN_X)+((115+ORIGIN_Y)*256),
                                        (10+ORIGIN_X)+((105+ORIGIN_Y)*256), (125+ORIGIN_X)+((105+ORIGIN_Y)*256),
                                        (67+ORIGIN_X)+((105+ORIGIN_Y)*256), (68+ORIGIN_X)+((105+ORIGIN_Y)*256),
                                        (67+ORIGIN_X)+((85+ORIGIN_Y)*256), (68+ORIGIN_X)+((85+ORIGIN_Y)*256),
                                        (67+ORIGIN_X)+((35+ORIGIN_Y)*256), (68+ORIGIN_X)+((35+ORIGIN_Y)*256),
                                        (60+ORIGIN_X)+((45+ORIGIN_Y)*256), (75+ORIGIN_X)+((45+ORIGIN_Y)*256),
                                        (40+ORIGIN_X)+((60+ORIGIN_Y)*256), (95+ORIGIN_X)+((60+ORIGIN_Y)*256),
                                        (35+ORIGIN_X)+((25+ORIGIN_Y)*256), (100+ORIGIN_X)+((25+ORIGIN_Y)*256),
                                        (10+ORIGIN_X)+((85+ORIGIN_Y)*256), (125+ORIGIN_X)+((85+ORIGIN_Y)*256),
                                        (10+ORIGIN_X)+((10+ORIGIN_Y)*256), (125+ORIGIN_X)+((10+ORIGIN_Y)*256),
                                        (35+ORIGIN_X)+((10+ORIGIN_Y)*256), (100+ORIGIN_X)+((10+ORIGIN_Y)*256),
                                        (40+ORIGIN_X)+((35+ORIGIN_Y)*256), (95+ORIGIN_X)+((35+ORIGIN_Y)*256),
                                        (35+ORIGIN_X)+((85+ORIGIN_Y)*256), (100+ORIGIN_X)+((85+ORIGIN_Y)*256),
                                        (10+ORIGIN_X)+((25+ORIGIN_Y)*256), (125+ORIGIN_X)+((25+ORIGIN_Y)*256)}

'LUT containing maze piece addresses
dim MAZE_ADDRS(NUM_MAZE_PIECES - 1) = {@MAZE_BORDER, @MAZE_BORDER, @MAZE_BORDER, @MAZE_BORDER, @MAZE_BORDER,
                                       @MAZE_BORDER, @MAZE_OUTER_T, @MAZE_OUTER_T, @MAZE_OUTER_B, @MAZE_OUTER_B,
                                       @MAZE_TJUNC_0, @MAZE_TJUNC_0, @MAZE_TJUNC_1, @MAZE_TJUNC_1, @MAZE_TJUNC_1,
                                       @MAZE_TJUNC_1, @MAZE_TJUNC_1, @MAZE_TJUNC_1, @MAZE_CAGE, @MAZE_CAGE,
                                       @MAZE_VERT_0, @MAZE_VERT_0, @MAZE_VERT_1, @MAZE_VERT_1, @MAZE_CORNER,
                                       @MAZE_CORNER, @MAZE_HORIZ_0, @MAZE_HORIZ_0, @MAZE_HORIZ_1, @MAZE_HORIZ_1,
                                       @MAZE_LINE_0, @MAZE_LINE_0, @MAZE_LINE_1, @MAZE_LINE_1, @MAZE_LINE_0, @MAZE_LINE_0}

'LUT containing ghost directions
dim GHOST_DIRS(7) = 1, 0, 0, 1, -1, 0, 0, -1

'LUT containing ghost sprite frames
dim gframes%(NUM_GHOSTS*4 - 1) = BlinkyRt, BlinkyLt, BlinkyDn, BlinkyUp, PinkyRt, PinkyLt, PinkyDn, PinkyUp, InkyRt, InkyLt, InkyDn, InkyUp, ClydeRt, ClydeLt, ClydeDn, ClydeUp

'LUT containing ghost sprite altrenate frames
dim gframes1%(NUM_GHOSTS - 1) = Blinky_1, Pinky_1, Inky_1, Clyde_1

dim ghostsXr(NUM_GHOSTS)    = (64+ORIGIN_X), (52+ORIGIN_X), (52+ORIGIN_X), (76+ORIGIN_X), (76+ORIGIN_X)
dim ghostsYr(NUM_GHOSTS)    = (36+ORIGIN_Y), (46+ORIGIN_Y), (56+ORIGIN_Y), (46+ORIGIN_Y), (56+ORIGIN_Y)
dim ghostsX(NUM_GHOSTS - 1) = (64+ORIGIN_X), (52+ORIGIN_X), (52+ORIGIN_X), (76+ORIGIN_X)
dim ghostsY(NUM_GHOSTS - 1) = (36+ORIGIN_Y), (46+ORIGIN_Y), (56+ORIGIN_Y), (46+ORIGIN_Y)
dim ghostsXd(NUM_GHOSTS - 1) = -1, -1, -1, -1
dim ghostsYd(NUM_GHOSTS - 1) = 0, 0, 0, 0
dim ghostsFlags%(NUM_GHOSTS - 1) = 0, 0, 0, 0

const SCORE_LEN = 7
dim highBCD%(SCORE_LEN - 1) = 0
dim scoreBCD%(SCORE_LEN - 1) = 0
dim pointsBCD%(SCORE_LEN - 1) = 0

const LEVEL_LEN = 5
dim levelBCD%(LEVEL_LEN - 1) = 0
