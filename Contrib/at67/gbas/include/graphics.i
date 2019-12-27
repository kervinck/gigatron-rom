; do *NOT* use register4 to register7 during time slicing if you call realTimeProc
graphicsMode        EQU     register0
waitVBlankNum       EQU     register0

drawHLine_x1        EQU     register0
drawHLine_y1        EQU     register1
drawHLine_x2        EQU     register2
drawHLine_x4        EQU     register3

drawVLine_x1        EQU     register0
drawVLine_y1        EQU     register1
drawVLine_y2        EQU     register2
drawVLine_y8        EQU     register3

drawLine_x1         EQU     register0
drawLine_y1         EQU     register1
drawLine_x2         EQU     register2
drawLine_y2         EQU     register3
drawLine_xy1        EQU     register0
drawLine_xy2        EQU     register1
drawLine_dxy1       EQU     register2
drawLine_dxy2       EQU     register3
drawLine_dx1        EQU     register4
drawLine_dy1        EQU     register5
drawLine_dx2        EQU     register6
drawLine_dy2        EQU     register7
drawLine_sx         EQU     register8
drawLine_sy         EQU     register9
drawLine_h          EQU     register10
drawLine_num        EQU     register11
drawLine_count      EQU     register12
drawLine_addr       EQU     register13
drawLine_tmp        EQU     register14
    
    
%SUB                scanlineMode
scanlineMode        LDW     giga_romType
                    ANDI    0xF8
                    SUBI    romTypeValue_ROMv2
                    BGE     scanlineM_cont
                    RET
    
scanlineM_cont      LDWI    SYS_SetMode_v2_80
                    STW     giga_sysFn
                    LDW     graphicsMode
                    SYS     0xE6                ; 270 - max(14,80/2)
                    RET
%ENDS   
    
%SUB                waitVBlank
waitVBlank          LDW     waitVBlankNum
                    SUBI    0x01
                    STW     waitVBlankNum
                    BGE     waitVB_start
                    RET
    
waitVB_start        LD      giga_frameCount
                    SUBW    frameCountPrev
                    BEQ     waitVB_start
                    LD      giga_frameCount
                    STW     frameCountPrev
                    PUSH
                    CALL    realTimeProcAddr
                    POP
                    BRA     waitVBlank
%ENDS   

%SUB                drawHLine
drawHLine           PUSH
                    LD      drawHLine_x1
                    ST      giga_sysArg4
                    LD      drawHLine_y1
                    ADDI    8
                    ST      giga_sysArg4 + 1
                    LDW     drawHLine_x2
                    SUBW    drawHLine_x1
                    SUBI    4
                    BLT     drawHL_loop1
                    
                    LD      fgbgColour + 1
                    ST      giga_sysArg0
                    ST      giga_sysArg0 + 1
                    ST      giga_sysArg2
                    ST      giga_sysArg2 + 1    ; 4 pixels of fg colour
                    LDWI    SYS_Draw4_30        ; setup 4 pixel SYS routine
                    STW     giga_sysFn
                    
                    LDW     drawHLine_x2        
                    SUBI    3
                    STW     drawHLine_x4        ; 4 pixel chunks limit

drawHL_loop0        SYS     0xFF                ; SYS_Draw4_30, 270 - 30/2 = 0xFF
                    CALL    realTimeProcAddr
                    LD      giga_sysArg4
                    ADDI    4
                    ST      giga_sysArg4
                    SUBW    drawHLine_x4
                    BLT     drawHL_loop0        ; all 4 pixel chunks
                    
drawHL_loop1        LD      fgbgColour + 1
                    POKE    giga_sysArg4
                    INC     giga_sysArg4
                    LD      giga_sysArg4
                    SUBW    drawHLine_x2
                    BLE     drawHL_loop1        ; remaining pixels
                    POP
                    RET
%ENDS

%SUB                drawVLine
drawVLine           PUSH
                    LD      drawVLine_x1
                    ST      giga_sysArg4
                    LD      drawVLine_y1
                    ADDI    8
                    ST      drawVLine_y1
                    ST      giga_sysArg4 + 1
                    LDW     drawVLine_y2
                    ADDI    8
                    ST      drawVLine_y2
                    SUBW    drawVLine_y1
                    SUBI    8
                    BLT     drawVL_loop1
                    
                    LDW     fgbgColour
                    STW     giga_sysArg0
                    LDWI    SYS_VDrawBits_134
                    STW     giga_sysFn          ; setup 8 pixel SYS routine
                    
                    LDW     drawVLine_y2
                    SUBI    7
                    STW     drawVLine_y8        ; 8 pixel chunks limit

drawVL_loop0        LDI     0xFF
                    ST      giga_sysArg2        ; 8 pixels of fg and bg colour
                    SYS     0xCB                ; SYS_VDrawBits_134, 270 - 134/2 = 0xCB
                    CALL    realTimeProcAddr
                    LD      giga_sysArg4 + 1
                    ADDI    8
                    ST      giga_sysArg4 + 1
                    SUBW    drawVLine_y8
                    BLT     drawVL_loop0        ; all 8 pixel chunks
                    
drawVL_loop1        LD      fgbgColour + 1
                    POKE    giga_sysArg4
                    INC     giga_sysArg4 + 1
                    LD      giga_sysArg4 + 1
                    SUBW    drawVLine_y2
                    BLE     drawVL_loop1        ; remaining pixels
                    POP
                    RET
%ENDS

%SUB                drawLine
drawLine            PUSH                        ; matches drawLineLoop's POP
                    LDI     1
                    STW     drawLine_dx1
                    STW     drawLine_dx2
                    STW     drawLine_dy1
                    LDI     0
                    STW     drawLine_dy2                
    
                    LDWI    0x8000
                    STW     drawLine_tmp
                    
                    LDW     drawLine_x2         ; sx = x2 - x1
                    SUBW    drawLine_x1
                    STW     drawLine_sx
                    ANDW    drawLine_tmp        
                    BEQ     drawL_dy
                    LDWI    -1
                    STW     drawLine_dx1        
                    STW     drawLine_dx2        ; dx1 = dx2 = (sx & 0x8000) ? -1 : 1
                    LDI     0                   ; sx = (sx & 0x8000) ? 0 - sx : sx
                    SUBW    drawLine_sx
                    STW     drawLine_sx                
                    
drawL_dy            LDW     drawLine_y2
                    SUBW    drawLine_y1
                    STW     drawLine_sy
                    STW     drawLine_h          ; h = sy
                    ANDW    drawLine_tmp
                    BEQ     drawL_ext
                    
                    LDWI    -1
                    STW     drawLine_dy1        ; dy1 = (sy & 0x8000) ? -1 : 1
                    LDI     0                   
                    SUBW    drawLine_sy
                    STW     drawLine_sy         ; sy = (sy & 0x8000) ? 0 - sy : sy
                    SUBW    drawLine_sx
                    BLE     drawL_ext           
                    LDW     drawLine_dy1
                    STW     drawLine_dy2        ; if(sx < sy) dy2 = -1
    
drawL_ext           LDWI    drawLineLoadXY
                    CALL    giga_vAC
                    LDWI    drawLineExt
                    CALL    giga_vAC
%ENDS   
                    
%SUB                drawLineExt
drawLineExt         LDW     drawLine_sy
                    SUBW    drawLine_sx
                    BLE     drawL_num
                    LDI     0
                    STW     drawLine_dx2        ; if(sx < sy) dx2 = 0
                    LDW     drawLine_sy       
                    STW     drawLine_tmp
                    LDW     drawLine_sx
                    STW     drawLine_sy
                    LDW     drawLine_tmp
                    STW     drawLine_sx         ; swap sx with sy
                    LDW     drawLine_h
                    BLE     drawL_num
                    LDI     1
                    STW     drawLine_dy2        ; if(h > 0) dy2 = 1
    
drawL_num           LDWI    SYS_LSRW1_48
                    STW     giga_sysFn          
                    LDW     drawLine_sx
                    SYS     0xF6                ; 0xF6 = 270-max(14,48/2)
                    ADDI    1
                    STW     drawLine_num        ; numerator = sx>>1
                    STW     drawLine_count      ; for(count=sx>>1; counti>=0; --i)
                    
                    LDWI    drawLineLoadDXY
                    CALL    giga_vAC
                    LDWI    drawLineLoop
                    CALL    giga_vAC
%ENDS

%SUB                drawLineLoop
drawLineLoop        LD      fgbgColour + 1
                    POKE    drawLine_xy1        ; plot start pixel
                    POKE    drawLine_xy2        ; plot end pixel, (meet in middle)
                    
                    LDW     drawLine_num        ; numerator += sy
                    ADDW    drawLine_sy
                    STW     drawLine_num
                    SUBW    drawLine_sx
                    BLE     drawL_flip          ; if(numerator <= sx) goto flip
                    STW     drawLine_num        ; numerator -= sx
                    
                    LDW     drawLine_xy1
                    ADDW    drawLine_dxy1
                    STW     drawLine_xy1        ; xy1 += dxy1
                    
                    LDW     drawLine_xy2
                    SUBW    drawLine_dxy1
                    STW     drawLine_xy2        ; xy2 -= dxy1
                    BRA     drawL_count
                    
drawL_flip          LDW     drawLine_xy1        
                    ADDW    drawLine_dxy2
                    STW     drawLine_xy1        ; xy1 += dxy2
                    
                    LDW     drawLine_xy2        
                    SUBW    drawLine_dxy2
                    STW     drawLine_xy2        ; xy2 -= dxy2
                    
drawL_count         CALL    realTimeProcAddr
                    LDW     drawLine_count
                    SUBI    0x01
                    STW     drawLine_count
                    BGT     drawLineLoop
                    POP                         ; matches drawLine's PUSH
                    RET
%ENDS   
    
%SUB                drawLineLoadXY
drawLineLoadXY      LD      drawLine_x1
                    ST      drawLine_xy1
                    LD      drawLine_y1
                    ADDI    8
                    ST      drawLine_xy1 + 1    ; xy1 = x1 | ((y1+8)<<8)
                    
                    LD      drawLine_x2
                    ST      drawLine_xy2
                    LD      drawLine_y2
                    ADDI    8
                    ST      drawLine_xy2 + 1    ; xy2 = x2 | ((y2+8)<<8)
                    RET
                    
drawLineLoadDXY     LDWI    SYS_LSLW8_24
                    STW     giga_sysFn          
                    LDW     drawLine_dy1
                    SYS     0x00                ; LSL 8, 0x00 = 270-max(14,24/2)
                    ADDW    drawLine_dx1
                    STW     drawLine_dxy1       ; dxy1 = dx1 + (dy1<<8)
    
                    LDW     drawLine_dy2
                    SYS     0x00                ; LSL 8, 0x00 = 270-max(14,24/2)
                    ADDW    drawLine_dx2
                    STW     drawLine_dxy2       ; dxy2 = dx2 + (dy2<<8)
                    RET
%ENDS   

%SUB                drawVTLine
drawVTLine          PUSH                        ; matches drawVTLineLoop's POP
                    LDWI    giga_videoTable
                    STW     drawLine_addr
                    LDI     1
                    STW     drawLine_dx1
                    STW     drawLine_dx2
                    STW     drawLine_dy1
                    LDI     0
                    STW     drawLine_dy2                
    
                    LDWI    0x8000
                    STW     drawLine_tmp
                    
                    LDW     drawLine_x2         ; sx = x2 - x1
                    SUBW    drawLine_x1
                    STW     drawLine_sx
                    ANDW    drawLine_tmp        
                    BEQ     drawVTL_dy
                    LDWI    -1
                    STW     drawLine_dx1        
                    STW     drawLine_dx2        ; dx1 = dx2 = (sx & 0x8000) ? -1 : 1
                    LDI     0                   ; sx = (sx & 0x8000) ? 0 - sx : sx
                    SUBW    drawLine_sx
                    STW     drawLine_sx                
                    
drawVTL_dy          LDW     drawLine_y2
                    SUBW    drawLine_y1
                    STW     drawLine_sy
                    STW     drawLine_h          ; h = sy
                    ANDW    drawLine_tmp
                    BEQ     drawVTL_ext
                    
                    LDWI    -1
                    STW     drawLine_dy1        ; dy1 = (sy & 0x8000) ? -1 : 1
                    LDI     0                   
                    SUBW    drawLine_sy
                    STW     drawLine_sy         ; sy = (sy & 0x8000) ? 0 - sy : sy
                    SUBW    drawLine_sx
                    BLE     drawVTL_ext           
                    LDW     drawLine_dy1
                    STW     drawLine_dy2        ; if(sx < sy) dy2 = -1
    
drawVTL_ext         LDWI    drawVTLineLoadXY
                    CALL    giga_vAC
                    LDWI    drawVTLineExt
                    CALL    giga_vAC
%ENDS   
                    
%SUB                drawVTLineExt
drawVTLineExt       LDW     drawLine_sy
                    SUBW    drawLine_sx
                    BLE     drawVTL_num
                    LDI     0
                    STW     drawLine_dx2        ; if(sx < sy) dx2 = 0
                    LDW     drawLine_sy       
                    STW     drawLine_tmp
                    LDW     drawLine_sx
                    STW     drawLine_sy
                    LDW     drawLine_tmp
                    STW     drawLine_sx         ; swap sx with sy
                    LDW     drawLine_h
                    BLE     drawVTL_num
                    LDI     1
                    STW     drawLine_dy2        ; if(h > 0) dy2 = 1
    
drawVTL_num         LDWI    SYS_LSRW1_48
                    STW     giga_sysFn          
                    LDW     drawLine_sx
                    SYS     0xF6                ; 0xF6 = 270-max(14,48/2)
                    ADDI    1
                    STW     drawLine_num        ; numerator = sx>>1
                    STW     drawLine_count      ; for(count=sx>>1; counti>=0; --i)
                    
                    LDWI    drawVTLineLoadDXY
                    CALL    giga_vAC
                    LDWI    drawVTLineLoop
                    CALL    giga_vAC
%ENDS

%SUB                drawVTLineLoop
drawVTLineLoop      LDW     drawLine_xy1
                    STW     drawLine_tmp
                    LDWI    drawVTLineAddress
                    CALL    giga_vAC            ; plot start pixel

                    LDW     drawLine_xy2
                    STW     drawLine_tmp
                    LDWI    drawVTLineAddress
                    CALL    giga_vAC            ; plot end pixel, (meet in middle)
                    
                    LDW     drawLine_num        ; numerator += sy
                    ADDW    drawLine_sy
                    STW     drawLine_num
                    SUBW    drawLine_sx
                    BLE     drawVTL_flip        ; if(numerator <= sx) goto flip
                    STW     drawLine_num        ; numerator -= sx
                    
                    LDW     drawLine_xy1
                    ADDW    drawLine_dxy1
                    STW     drawLine_xy1        ; xy1 += dxy1
                    
                    LDW     drawLine_xy2
                    SUBW    drawLine_dxy1
                    STW     drawLine_xy2        ; xy2 -= dxy1
                    BRA     drawVTL_count
                    
drawVTL_flip        LDW     drawLine_xy1        
                    ADDW    drawLine_dxy2
                    STW     drawLine_xy1        ; xy1 += dxy2
                    
                    LDW     drawLine_xy2        
                    SUBW    drawLine_dxy2
                    STW     drawLine_xy2        ; xy2 -= dxy2
                    
drawVTL_count       CALL    realTimeProcAddr
                    LDW     drawLine_count
                    SUBI    0x01
                    STW     drawLine_count
                    BGT     drawVTLineLoop
                    POP                         ; matches drawVTLine's PUSH
                    RET
                    
drawVTLineAddress   LD      drawLine_tmp + 1
                    LSLW
                    ADDW    drawLine_addr
                    PEEK
                    ST      drawLine_tmp + 1
                    LD      fgbgColour + 1
                    POKE    drawLine_tmp
                    RET
%ENDS   
    
%SUB                drawVTLineLoadXY
drawVTLineLoadXY    LD      drawLine_x1
                    ST      drawLine_xy1
                    LD      drawLine_y1
                    ST      drawLine_xy1 + 1    ; xy1 = x1 | (y1<<8)
                    
                    LD      drawLine_x2
                    ST      drawLine_xy2
                    LD      drawLine_y2
                    ST      drawLine_xy2 + 1    ; xy2 = x2 | (y2<<8)
                    RET
                    
drawVTLineLoadDXY   LDWI    SYS_LSLW8_24
                    STW     giga_sysFn          
                    LDW     drawLine_dy1
                    SYS     0x00                ; LSL 8, 0x00 = 270-max(14,24/2)
                    ADDW    drawLine_dx1
                    STW     drawLine_dxy1       ; dxy1 = dx1 + (dy1<<8)
    
                    LDW     drawLine_dy2
                    SYS     0x00                ; LSL 8, 0x00 = 270-max(14,24/2)
                    ADDW    drawLine_dx2
                    STW     drawLine_dxy2       ; dxy2 = dx2 + (dy2<<8)
                    RET
%ENDS   
    
%SUB                atLineCursor
atLineCursor        LD      cursorXY
                    STW     drawLine_x1
                    SUBI    giga_xres
                    BLT     atLC_x1good
                    LDI     0
                    STW     drawLine_x1
                    
atLC_x1good         LD      cursorXY
                    ADDW    drawLine_x2
                    SUBI    giga_xres
                    BLT     atLC_x2good
                    LDWI    -giga_xres
                    
atLC_x2good         ADDI    giga_xres
                    STW     drawLine_x2
                    ST      cursorXY

                    LD      cursorXY + 1
                    STW     drawLine_y1
                    SUBI    giga_yres
                    BLT     atLC_y1good
                    LDI     giga_yres - 1
                    STW     drawLine_y1
                    
atLC_y1good         LD      cursorXY + 1
                    ADDW    drawLine_y2
                    SUBI    giga_yres
                    BLT     atLC_y2good
                    LDWI    -1
                    
atLC_y2good         ADDI    giga_yres
                    STW     drawLine_y2
                    ST      cursorXY + 1
                    RET
%ENDS