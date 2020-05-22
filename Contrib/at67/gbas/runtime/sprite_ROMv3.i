; do *NOT* use register4 to register7 during time slicing if you use realTimeStub
spriteId            EQU     register0
spriteXY            EQU     register1
spriteAddrs         EQU     register2

    
%SUB                draw_sprite
draw_sprite         PUSH
                    LDWI    _spritesLut_
                    ADDW    spriteId
                    ADDW    spriteId
                    DEEK
                    STW     spriteAddrs                     ; get sprite address table
                    
draw_s_loop         LDW     spriteAddrs
                    DEEK                                    ; get source address
                    BEQ     draw_s_exit
                    STW     giga_sysArg0
                    INC     spriteAddrs
                    INC     spriteAddrs
                    LDW     spriteAddrs
                    DEEK                                    ; get stripe destination offset
                    ADDW    spriteXY
                    SYS     64
                    INC     spriteAddrs
                    INC     spriteAddrs
                    CALL    realTimeStubAddr
                    BRA     draw_s_loop
                    
draw_s_exit         POP
                    RET
%ENDS

%SUB                drawSprite
drawSprite          PUSH
                    LDWI    SYS_Sprite6_v3_64
                    STW     giga_sysFn
                    LDWI    draw_sprite
                    CALL    giga_vAC
                    POP
                    RET
%ENDS

%SUB                drawSpriteX
drawSpriteX         PUSH
                    LDWI    SYS_Sprite6x_v3_64
                    STW     giga_sysFn
                    LDWI    draw_sprite
                    CALL    giga_vAC
                    POP
                    RET
%ENDS

%SUB                drawSpriteY
drawSpriteY         PUSH
                    LDWI    SYS_Sprite6y_v3_64
                    STW     giga_sysFn
                    LDWI    draw_sprite
                    CALL    giga_vAC
                    POP
                    RET
%ENDS

%SUB                drawSpriteXY
drawSpriteXY        PUSH
                    LDWI    SYS_Sprite6xy_v3_64
                    STW     giga_sysFn
                    LDWI    draw_sprite
                    CALL    giga_vAC
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