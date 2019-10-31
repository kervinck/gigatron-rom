graphicsMode    EQU     register0
waitVBlankNum   EQU     register0
drawLine_x1     EQU     register0
drawLine_y1     EQU     register1
drawLine_x2     EQU     register2
drawLine_y2     EQU     register3
drawLine_xy1    EQU     register0
drawLine_xy2    EQU     register1
drawLine_dxy1   EQU     register2
drawLine_dxy2   EQU     register3
drawLine_dx1    EQU     register4
drawLine_dy1    EQU     register5
drawLine_dx2    EQU     register6
drawLine_dy2    EQU     register7
drawLine_sx     EQU     register8
drawLine_sy     EQU     register9
drawLine_h      EQU     register10
drawLine_num    EQU     register11
drawLine_count  EQU     register12
drawLine_tmp    EQU     register13


%SUB            scanlineMode
scanlineMode    LDW     giga_romType
                ANDI    0xF8
                SUBI    romTypeValue_ROMv2
                BGE     scanlineM_cont
                RET

scanlineM_cont  LDWI    SYS_SetMode_v2_80
                STW     giga_sysFn
                LDW     graphicsMode
                SYS     0xE6                ; 270 - max(14,80/2)
                RET
%ENDS

%SUB            waitVBlank
waitVBlank      LD      waitVBlankNum
                SUBI    0x01
                ST      waitVBlankNum
                BGE     waitVB_start
                RET

waitVB_start    LD      giga_frameCount
                SUBW    frameCountPrev
                BEQ     waitVB_start
                LD      giga_frameCount
                STW     frameCountPrev
                BRA     waitVBlank
%ENDS

%SUB            drawLine
drawLine        PUSH
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
                
drawL_dy        LDW     drawLine_y2
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

drawL_ext       LDWI    drawLineLoadXY
                CALL    giga_vAC
                LDWI    drawLineExt
                CALL    giga_vAC
%ENDS
                
%SUB            drawLineExt
drawLineExt     LDW     drawLine_sy
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

drawL_num       LDWI    SYS_LSRW1_48
                STW     giga_sysFn          
                LDW     drawLine_sx
                SYS     0xF6                ; 0xF6 = 270-max(14,48/2)
                STW     drawLine_num        ; numerator = sx>>1
                STW     drawLine_count
                INC     drawLine_count      ; for(count=sx>>1; counti>=0; --i)
                
                LDWI    drawLineLoadDXY
                CALL    giga_vAC     
                
drawL_loop      LD      fgbgColour + 1
                POKE    drawLine_xy1        ; plot start pixel
                POKE    drawLine_xy2        ; plot end pixel, (meet in middle)      
                
                LDW     drawLine_num        ; numerator += sy
                ADDW    drawLine_sy
                STW     drawLine_num
                SUBW    drawLine_sx
                BLE     drawL_flip          ; if(numerator <= sx) goto flip
                
                STW     drawLine_num        ; numerator -= sx
                LDWI    drawLineDelta1      ; x1 += dx1, y1 += dx1, x2 -= dx1, y2 -= dx1
                CALL    giga_vAC
                BRA     drawL_count
                
drawL_flip      LDWI    drawLineDelta2      ; x1 += dx2, y1 += dx2, x2 -= dx2, y2 -= dx2
                CALL    giga_vAC
                
drawL_count     LDW     drawLine_count
                SUBI    0x01
                STW     drawLine_count
                BGT     drawL_loop
                POP
                RET
%ENDS

%SUB            drawLineDelta1
drawLineDelta1  LDW     drawLine_xy1
                ADDW    drawLine_dxy1
                STW     drawLine_xy1        ; xy1 += dxy1
                
                LDW     drawLine_xy2
                SUBW    drawLine_dxy1
                STW     drawLine_xy2        ; xy2 -= dxy1
                RET
                
drawLineDelta2  LDW     drawLine_xy1        
                ADDW    drawLine_dxy2
                STW     drawLine_xy1        ; xy1 += dxy2
                
                LDW     drawLine_xy2        
                SUBW    drawLine_dxy2
                STW     drawLine_xy2        ; xy2 -= dxy2
                RET
                
drawLineLoadXY  LD      drawLine_x1
                ST      drawLine_xy1
                LD      drawLine_y1
                ADDI    08
                ST      drawLine_xy1 + 1    ; xy1 = x1 | ((y1+8)<<8)
                
                LD      drawLine_x2
                ST      drawLine_xy2
                LD      drawLine_y2
                ADDI    08
                ST      drawLine_xy2 + 1    ; xy2 = x2 | ((y2+8)<<8)
                RET
                
drawLineLoadDXY LDWI    SYS_LSLW8_24
                STW     giga_sysFn          
                LDW     drawLine_dy1
                SYS     0x00                ; 0x00 = 270-max(14,24/2)
                ADDW    drawLine_dx1
                STW     drawLine_dxy1       ; dxy1 = dx1 + (dy1<<8)

                LDW     drawLine_dy2
                SYS     0x00                ; 0x00 = 270-max(14,24/2)
                ADDW    drawLine_dx2
                STW     drawLine_dxy2       ; dxy2 = dx2 + (dy2<<8)
                RET
%ENDS
