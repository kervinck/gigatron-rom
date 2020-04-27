; do *NOT* use register4 to register7 during time slicing if you use realTimeProc
spriteId            EQU     register0
spriteXY            EQU     register1
spriteAddrs         EQU     register2

    
%SUB                drawSprite
drawSprite          LDWI    SYS_Sprite6_v3_64
                    STW     giga_sysFn
                    LDWI    _spritesLut_
                    ADDW    spriteId
                    ADDW    spriteId
                    DEEK
                    STW     spriteAddrs                     ; get sprite address table
                    
drawSprite_loop     LDW     spriteAddrs
                    DEEK
                    BEQ     drawSprite_exit
                    STW     giga_sysArg0
                    INC     spriteAddrs
                    INC     spriteAddrs
                    LDW     spriteAddrs
                    DEEK
                    ADDW    spriteXY
                    SYS     64
                    INC     spriteAddrs
                    INC     spriteAddrs
                    BRA     drawSprite_loop
                    
drawSprite_exit     RET
%ENDS

%SUB                drawSpriteX
drawSpriteX         RET
%ENDS

%SUB                drawSpriteY
drawSpriteY         RET
%ENDS

%SUB                drawSpriteXY
drawSpriteXY        RET
%ENDS
