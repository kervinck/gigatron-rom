; do *NOT* use register4 to register7 during time slicing
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
drawLine_dx1        EQU     register8
drawLine_dy1        EQU     register9
drawLine_dx2        EQU     register10
drawLine_dy2        EQU     register11
drawLine_sx         EQU     register12
drawLine_sy         EQU     register13
drawLine_h          EQU     register14
drawLine_num        EQU     register15
drawLine_count      EQU     register14
drawLine_tmp        EQU     register15
drawLine_dx         EQU     register2
drawLine_dy         EQU     register3
drawLine_u          EQU     register8
drawLine_v          EQU     register9
drawLine_addr       EQU     register10
drawLine_ddx        EQU     register11
drawLine_cnt        EQU     register12
drawLine_swp        EQU     register13

drawPixel_xy        EQU     register15
readPixel_xy        EQU     register15

drawCircle_cx       EQU     register0
drawCircle_cy       EQU     register1
drawCircle_r        EQU     register2
drawCircle_a        EQU     register3
drawCircle_d        EQU     register8
drawCircle_x        EQU     register9
drawCircle_y        EQU     register10
drawCircle_ch0      EQU     register11
drawCircle_ch1      EQU     register12
drawCircle_ch2      EQU     register13
drawCircle_ch3      EQU     register14

drawCircleF_x1      EQU     register0
drawCircleF_y1      EQU     register1
drawCircleF_x2      EQU     register2
drawCircleF_cx      EQU     register15
drawCircleF_cy      EQU     register10
drawCircleF_r       EQU     register11
drawCircleF_v       EQU     register8
drawCircleF_w       EQU     register9

drawRect_x1         EQU     register7
drawRect_y1         EQU     register10
drawRect_x2         EQU     register11
drawRect_y2         EQU     register15

drawRectF_x1        EQU     register0
drawRectF_y1        EQU     register1
drawRectF_x2        EQU     register2
drawRectF_y2        EQU     register7

drawPoly_mode       EQU     register14
drawPoly_addr       EQU     register15

    
%SUB                scanlineMode
scanlineMode        LDWI    SYS_SetMode_v2_80
                    STW     giga_sysFn
                    LDW     graphicsMode
                    SYS     80
                    RET
%ENDS   

%SUB                waitVBlanks
waitVBlanks         PUSH

waitVB_loop0        LDW     waitVBlankNum
                    SUBI    0x01
                    STW     waitVBlankNum
                    BGE     waitVB_vblank
                    POP
                    RET
    
waitVB_vblank       CALLI   waitVBlank

waitVB_loop1        LD      giga_videoY         ; wait until bottom scanline, (0x01EE), is done
                    XORI    &HEE
                    BEQ     waitVB_loop1
                    BRA     waitVB_loop0
%ENDS   

%SUB                waitVBlank
                    ; 179 is normally the start of vBlank, but if a vBlank routine is executing there
                    ; is a very good chance by the time the vBlank routine is over giga_videoY will
                    ; have progressed past 179, (by how much is nondeterministic). So instead we wait
                    ; for the scanline before vBlank, i.e. when videoY = 0xEE, (videoTablePtr = 0x01EE)
waitVBlank          LD      giga_videoY
                    XORI    &HEE
                    BNE     waitVBlank
                    RET
%ENDS

%SUB                readPixel
readPixel           STW     readPixel_xy
                    LD      readPixel_xy + 1    ; pixel = peek(peek(256 + 2*y)*256 + x)
                    LSLW
                    INC     giga_vAC + 1
                    PEEK
                    ST      readPixel_xy + 1
                    LDW     readPixel_xy
                    PEEK
                    RET
%ENDS

%SUB                drawPixel
drawPixel           STW     drawPixel_xy
                    LD      drawPixel_xy + 1    ; poke peek(256 + 2*y)*256 + x, fg_colour
                    LSLW
                    INC     giga_vAC + 1
                    PEEK
                    ST      drawPixel_xy + 1
                    LD      fgbgColour + 1
                    POKE    drawPixel_xy
                    RET
%ENDS   

%SUB                drawHLine
drawHLine           LD      drawHLine_x1
                    ST      giga_sysArg2                    ; low start address
                    LD      drawHLine_x2
                    SUBW    drawHLine_x1
                    BGE     drawHL_cont
                    LD      drawHLine_x2
                    ST      giga_sysArg2                    ; low start address
                    LD      drawHLine_x1
                    SUBW    drawHLine_x2
                    
drawHL_cont         ADDI    1
                    ST      giga_sysArg0                    ; count
                    LD      fgbgColour + 1
                    ST      giga_sysArg1                    ; fill value
                    LD      drawHLine_y1
                    ADDI    8
                    ST      giga_sysArg3                    ; high start address
                    LDWI    SYS_SetMemory_v2_54
                    STW     giga_sysFn
                    SYS     54                              ; fill memory
                    RET
%ENDS

%SUB                drawVLine
drawVLine           LDW     drawVLine_y2
                    SUBW    drawVLine_y1
                    BGE     drawVL_cont
                    LDW     drawVLine_y2
                    STW     drawLine_tmp
                    LDW     drawVLine_y1
                    STW     drawVLine_y2
                    LDW     drawLine_tmp
                    STW     drawVLine_y1        ; if y2 < y1 then swap y2 with y1
                    
drawVL_cont         LD      drawVLine_x1
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
                    SYS     134                 ; SYS_VDrawBits_134, 270 - 134/2 = 0xCB
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
    
drawL_ext           CALLI    drawLineLoadXY
                    CALLI    drawLineExt
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
                    SYS     48
                    ADDI    1
                    STW     drawLine_num        ; numerator = sx>>1
                    STW     drawLine_count      ; for(count=sx>>1; counti>=0; --i)
                    
                    CALLI    drawLineLoadDXY
                    CALLI    drawLineLoop
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
                    
drawL_count         LDW     drawLine_count
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
                    SYS     28
                    ADDW    drawLine_dx1
                    STW     drawLine_dxy1       ; dxy1 = dx1 + (dy1<<8)
    
                    LDW     drawLine_dy2
                    SYS     28
                    ADDW    drawLine_dx2
                    STW     drawLine_dxy2       ; dxy2 = dx2 + (dy2<<8)
                    RET
%ENDS   

%SUB                drawLineSlow
drawLineSlow        PUSH
                    LDI     1
                    STW     drawLine_u
                    LDW     drawLine_x2
                    SUBW    drawLine_x1                     ; dx = x2 - x1
                    BGE     drawLS_dxp
                    LDWI    -1
                    STW     drawLine_u
                    LDW     drawLine_x1
                    SUBW    drawLine_x2                     ; dx = x1 - x2
                    
drawLS_dxp          STW     drawLine_dx
                    LDWI    256
                    STW     drawLine_v
                    LDW     drawLine_y2
                    SUBW    drawLine_y1                     ; dy = y2 - y1
                    BGE     drawLS_dyp
                    LDWI    -256
                    STW     drawLine_v
                    LDW     drawLine_y1
                    SUBW    drawLine_y2                     ; sy = y1 - y2
                    
drawLS_dyp          STW     drawLine_dy
                    CALLI   drawLineSlowExt
%ENDS

%SUB                drawLineSlowExt
drawLineSlowExt     LD      drawLine_x1
                    ST      drawLine_addr
                    LD      drawLine_y1
                    ADDI    8
                    ST      drawLine_addr + 1
                    LDW     drawLine_dx
                    SUBW    drawLine_dy
                    BGE     drawLS_noswap
                    CALLI   drawLineSlowSwap
                    
drawLS_noswap       LDI     0
                    SUBW    drawLine_dx
                    STW     drawLine_ddx
                    STW     drawLine_cnt
                    LDW     drawLine_dx
                    ADDW    drawLine_dx
                    STW     drawLine_dx
                    LDW     drawLine_dy
                    ADDW    drawLine_dy
                    STW     drawLine_dy
                    CALLI   drawLineSlowLoop
%ENDS

%SUB                drawLineSlowLoop
drawLineSlowLoop    LD      fgbgColour + 1
                    POKE    drawLine_addr
                    LDW     drawLine_ddx
                    ADDW    drawLine_dy
                    STW     drawLine_ddx
                    BLE     drawLLS_xy
                    SUBW    drawLine_dx
                    STW     drawLine_ddx
                    LDW     drawLine_addr
                    ADDW    drawLine_v
                    STW     drawLine_addr

drawLLS_xy          LDW     drawLine_addr
                    ADDW    drawLine_u
                    STW     drawLine_addr
                    LDW     drawLine_cnt
                    ADDI    1
                    STW     drawLine_cnt
                    BLE     drawLineSlowLoop

                    POP
                    RET
%ENDS

%SUB                drawLineSlowSwap
drawLineSlowSwap    LDW     drawLine_dx
                    STW     drawLine_swp
                    LDW     drawLine_dy
                    STW     drawLine_dx
                    LDW     drawLine_swp
                    STW     drawLine_dy
                    LDW     drawLine_u
                    STW     drawLine_swp
                    LDW     drawLine_v
                    STW     drawLine_u
                    LDW     drawLine_swp
                    STW     drawLine_v
                    RET
%ENDS

%SUB                drawVTLine
drawVTLine          PUSH                        ; matches drawVTLineLoop's POP
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
    
drawVTL_ext         CALLI   drawVTLineLoadXY
                    CALLI   drawVTLineExt
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
                    SYS     48
                    ADDI    1
                    STW     drawLine_num        ; numerator = sx>>1
                    STW     drawLine_count      ; for(count=sx>>1; counti>=0; --i)
                    
                    CALLI   drawVTLineLoadDXY
                    CALLI   drawVTLineLoop
%ENDS

%SUB                drawVTLineLoop
drawVTLineLoop      LDW     drawLine_xy1
                    CALLI   drawPixel           ; plot start pixel

                    LDW     drawLine_xy2
                    CALLI   drawPixel           ; plot end pixel, (meet in middle)
                    
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
                    
drawVTL_count       LDW     drawLine_count
                    SUBI    0x01
                    STW     drawLine_count
                    BGT     drawVTLineLoop
                    POP                         ; matches drawVTLine's PUSH
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
                    SYS     28
                    ADDW    drawLine_dx1
                    STW     drawLine_dxy1       ; dxy1 = dx1 + (dy1<<8)
    
                    LDW     drawLine_dy2
                    SYS     28
                    ADDW    drawLine_dx2
                    STW     drawLine_dxy2       ; dxy2 = dx2 + (dy2<<8)
                    RET
%ENDS   

%SUB                drawCircle
drawCircle          PUSH
                    LDI     0
                    STW     drawCircle_ch0
                    STW     drawCircle_ch1
                    STW     drawCircle_ch2
                    STW     drawCircle_ch3
                    STW     drawCircle_x
                    LDW     drawCircle_r
                    STW     drawCircle_y
                    LDI     1
                    SUBW    drawCircle_r
                    STW     drawCircle_d
                    
drawC_loop          CALLI   drawCircleExt1
                    
                    LDW     drawCircle_d
                    BGE     drawC_skip
                    LDW     drawCircle_x
                    LSLW
                    LSLW
                    ADDW    drawCircle_d
                    ADDI    3
                    STW     drawCircle_d
                    BRA     drawC_cont
                    
drawC_skip          LDW     drawCircle_x
                    SUBW    drawCircle_y
                    LSLW
                    LSLW
                    ADDW    drawCircle_d
                    ADDI    5
                    STW     drawCircle_d
                    LDW     drawCircle_y
                    SUBI    1
                    STW     drawCircle_y

drawC_cont          INC     drawCircle_x
                    LDW     drawCircle_x
                    SUBW    drawCircle_y
                    BLE     drawC_loop

                    POP
                    RET
%ENDS

%SUB                drawCircleExt1
drawCircleExt1      PUSH
                    LDW     drawCircle_cy
                    ADDW    drawCircle_y
                    ST      drawCircle_ch0 + 1
                    LDW     drawCircle_cy
                    SUBW    drawCircle_y
                    ST      drawCircle_ch1 + 1
                    LDW     drawCircle_cy
                    ADDW    drawCircle_x
                    ST      drawCircle_ch2 + 1
                    LDW     drawCircle_cy
                    SUBW    drawCircle_x
                    ST      drawCircle_ch3 + 1

                    LDW     drawCircle_cx
                    ADDW    drawCircle_x
                    ADDW    drawCircle_ch0
                    STW     drawCircle_a
                    LD      fgbgColour + 1
                    POKE    drawCircle_a

                    LDW     drawCircle_cx
                    SUBW    drawCircle_x
                    ADDW    drawCircle_ch0
                    STW     drawCircle_a
                    LD      fgbgColour + 1
                    POKE    drawCircle_a
                    
                    LDW     drawCircle_cx
                    ADDW    drawCircle_x
                    ADDW    drawCircle_ch1
                    STW     drawCircle_a
                    LD      fgbgColour + 1
                    POKE    drawCircle_a

                    LDW     drawCircle_cx
                    SUBW    drawCircle_x
                    ADDW    drawCircle_ch1
                    STW     drawCircle_a
                    LD      fgbgColour + 1
                    POKE    drawCircle_a
                    
                    CALLI   drawCircleExt2      ; doesn't return to here
%ENDS
                    
%SUB                drawCircleExt2
drawCircleExt2      LDW     drawCircle_cx
                    ADDW    drawCircle_y
                    ADDW    drawCircle_ch2
                    STW     drawCircle_a
                    LD      fgbgColour + 1
                    POKE    drawCircle_a

                    LDW     drawCircle_cx
                    SUBW    drawCircle_y
                    ADDW    drawCircle_ch2
                    STW     drawCircle_a
                    LD      fgbgColour + 1
                    POKE    drawCircle_a
                    
                    LDW     drawCircle_cx
                    ADDW    drawCircle_y
                    ADDW    drawCircle_ch3
                    STW     drawCircle_a
                    LD      fgbgColour + 1
                    POKE    drawCircle_a

                    LDW     drawCircle_cx
                    SUBW    drawCircle_y
                    ADDW    drawCircle_ch3
                    STW     drawCircle_a
                    LD      fgbgColour + 1
                    POKE    drawCircle_a

                    POP
                    RET
%ENDS

%SUB                drawCircleF
drawCircleF         PUSH
                    LDI     0
                    STW     drawCircleF_v
                    STW     drawCircleF_w
                    
drawCF_wloop        LDW     drawCircleF_cx
                    SUBW    drawCircleF_r
                    STW     drawCircleF_x1
                    LDW     drawCircleF_cx
                    ADDW    drawCircleF_r
                    STW     drawCircleF_x2
                    LDW     drawCircleF_cy
                    SUBW    drawCircleF_v
                    STW     drawCircleF_y1
                    CALLI   drawHLine
                    LDW     drawCircleF_cy
                    ADDW    drawCircleF_v
                    STW     drawCircleF_y1
                    CALLI   drawHLine
                    
                    LDW     drawCircleF_w
                    ADDW    drawCircleF_v
                    ADDW    drawCircleF_v
                    ADDI    1
                    STW     drawCircleF_w
                    INC     drawCircleF_v
                    
drawCF_rloop        LDW     drawCircleF_w
                    BLT     drawCF_wloop
                    LDW     drawCircleF_w
                    SUBW    drawCircleF_r
                    SUBW    drawCircleF_r
                    ADDI    1
                    STW     drawCircleF_w
                    LDW     drawCircleF_r
                    SUBI    1
                    STW     drawCircleF_r
                    BGT     drawCF_rloop
                    POP
                    RET
%ENDS

%SUB                drawRect
drawRect            PUSH
                    LDW     drawRect_x1
                    STW     drawHLine_x1
                    LDW     drawRect_y1
                    STW     drawHLine_y1
                    LDW     drawRect_x2
                    STW     drawHLine_x2
                    CALLI   drawHLine
                    LDW     drawRect_y2
                    STW     drawHLine_y1
                    CALLI   drawHLine

                    LDW     drawRect_x1
                    STW     drawVLine_x1
                    LDW     drawRect_y1
                    STW     drawVLine_y1
                    LDW     drawRect_y2
                    STW     drawVLine_y2
                    CALLI   drawVLine
                    LDW     drawRect_x2
                    STW     drawVLine_x1
                    LDW     drawRect_y1
                    STW     drawVLine_y1
                    LDW     drawRect_y2
                    STW     drawVLine_y2
                    CALLI   drawVLine

                    POP
                    RET
%ENDS

%SUB                drawRectF
drawRectF           PUSH
                    LDW     drawRectF_y2
                    SUBW    drawRectF_y1
                    BGE     drawRF_loop
                    LDW     drawRectF_y2
                    STW     drawLine_tmp
                    LDW     drawRectF_y1
                    STW     drawRectF_y2
                    LDW     drawLine_tmp
                    STW     drawRectF_y1        ; if y2 < y1 then swap y2 with y1
                    
drawRF_loop         LDW     drawRectF_y1
                    STW     drawHLine_y1
                    CALLI   drawHLine
                    INC     drawRectF_y1
                    LDW     drawRectF_y1
                    SUBW    drawRectF_y2
                    BLE     drawRF_loop

                    POP
                    RET
%ENDS

%SUB                drawPoly
drawPoly            PUSH

drawP_loop          LD      cursorXY
                    STW     drawLine_x1
                    LD      cursorXY + 1
                    STW     drawLine_y1
                    LDW     drawPoly_addr
                    PEEK
                    STW     drawLine_x2
                    SUBI    255
                    BEQ     drawP_exit
                    LDW     drawLine_x2
                    ST      cursorXY
                    INC     drawPoly_addr
                    LDW     drawPoly_addr
                    PEEK
                    STW     drawLine_y2
                    ST      cursorXY + 1
                    CALLI   drawLine
                    INC     drawPoly_addr
                    BRA     drawP_loop
                    
drawP_exit          POP
                    RET
%ENDS

%SUB                drawPolyRel
drawPolyRel         PUSH

drawPR_loop         LD      cursorXY
                    STW     drawLine_x1
                    LD      cursorXY + 1
                    STW     drawLine_y1
                    LDW     drawPoly_addr
                    DEEK
                    STW     drawLine_x2
                    SUBI    255
                    BEQ     drawPR_exit
                    LDW     drawLine_x1
drawPR_x2           ADDW    drawLine_x2                     ;relative X mode
                    STW     drawLine_x2
                    ST      cursorXY
                    INC     drawPoly_addr
                    INC     drawPoly_addr
                    LDW     drawPoly_addr
                    DEEK
                    STW     drawLine_y2
                    LDW     drawLine_y1
drawPR_y2           ADDW    drawLine_y2                     ;relative Y mode
                    STW     drawLine_y2
                    ST      cursorXY + 1
                    CALLI   drawLineSlow
                    INC     drawPoly_addr
                    INC     drawPoly_addr
                    BRA     drawPR_loop
                    
drawPR_exit         LDI     0x99                            ;ADDW
                    ST      drawPoly_mode
                    CALLI   setPolyRelFlipX
                    CALLI   setPolyRelFlipY                 ;reset X and Y modes
                    POP
                    RET
%ENDS

%SUB                setPolyRelFlipX
setPolyRelFlipX     LDWI    drawPR_x2
                    STW     drawPoly_addr
                    LDW     drawPoly_mode
                    POKE    drawPoly_addr
                    RET
%ENDS

%SUB                setPolyRelFlipY
setPolyRelFlipY     LDWI    drawPR_y2
                    STW     drawPoly_addr
                    LDW     drawPoly_mode
                    POKE    drawPoly_addr
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