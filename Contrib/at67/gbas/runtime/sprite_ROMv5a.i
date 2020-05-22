; do *NOT* use register4 to register7 during time slicing
spriteId            EQU     register0
spriteXY            EQU     register1
spriteAddrs         EQU     register2

    
%SUB                draw_sprite
draw_sprite         LDWI    _spritesLut_
                    ADDW    spriteId
                    ADDW    spriteId
                    DEEK
                    STW     spriteAddrs                     ; get sprite address table
                    
draw_s_loop         LDW     spriteAddrs
                    DEEK
                    BEQ     draw_s_exit
                    STW     giga_sysArg0
                    INC     spriteAddrs
                    INC     spriteAddrs
                    LDW     spriteAddrs
                    DEEK
                    ADDW    spriteXY
                    SYS     64
                    INC     spriteAddrs
                    INC     spriteAddrs
                    BRA     draw_s_loop
                    
draw_s_exit         RET
%ENDS

%SUB                drawSprite
drawSprite          PUSH
                    LDWI    SYS_Sprite6_v3_64
                    STW     giga_sysFn
                    CALLI   draw_sprite
                    POP
                    RET
%ENDS

%SUB                drawSpriteX
drawSpriteX         PUSH
                    LDWI    SYS_Sprite6x_v3_64
                    STW     giga_sysFn
                    CALLI   draw_sprite
                    POP
                    RET
%ENDS

%SUB                drawSpriteY
drawSpriteY         PUSH
                    LDWI    SYS_Sprite6y_v3_64
                    STW     giga_sysFn
                    CALLI   draw_sprite
                    POP
                    RET
%ENDS

%SUB                drawSpriteXY
drawSpriteXY        PUSH
                    LDWI    SYS_Sprite6xy_v3_64
                    STW     giga_sysFn
                    CALLI   draw_sprite
                    POP
                    RET
%ENDS

%SUB                getSpriteLUT
getSpriteLUT        LDWI    _spritesLut_
                    ADDW    spriteId
                    ADDW    spriteId
                    DEEK
                    DEEK
                    RET
%ENDS
