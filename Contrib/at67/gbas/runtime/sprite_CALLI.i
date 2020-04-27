; do *NOT* use register4 to register7 during time slicing if you use realTimeProc
spriteId            EQU     register0
spriteXY            EQU     register1
spritesLut          EQU     register2
spriteAddrs         EQU     register2

    
%SUB                drawSprite
drawSprite          LDWI    SYS_Sprite6_v3_64
                    STW     giga_sysFn
                    LDW     spritesLut
                    ADDW    spriteId
                    ADDW    spriteId
                    DEEK
                    STW     spriteAddrs                     ; get sprite address table
                    
drawSprite_loop     DEEK
                    BEQ     drawSprite_exit
                    STW     giga_sysArg0
                    LDW     spriteXY
                    SYS     64
                    INC     spriteAddrs
                    INC     spriteAddrs
                    BRA     drawSprite_loop
                    
drawSprite_exit     RET
%ENDS

%SUB                drawSpriteFlipX
drawSpriteFlipX     RET
%ENDS

%SUB                drawSpriteFlipY
drawSpriteFlipY     RET
%ENDS

%SUB                drawSpriteFlipXY
drawSpriteFlipXY    RET
%ENDS
