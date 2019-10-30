graphicsMode    EQU     register0
waitVBlankNum   EQU     register0
drawLine_tmp    EQU     register0
drawLine_x1     EQU     register1
drawLine_y1     EQU     register2
drawLine_x2     EQU     register3
drawLine_y2     EQU     register4
drawLine_num    EQU     register3
drawLine_count  EQU     register4
drawLine_dx1    EQU     register5
drawLine_dy1    EQU     register6
drawLine_dx2    EQU     register7
drawLine_dy2    EQU     register8
drawLine_sx     EQU     register9
drawLine_sy     EQU     register10
drawLine_h      EQU     register11
drawLine_sgn    EQU     register12


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
                STW     drawLine_sgn
                
                LDW     drawLine_x2         ; sx = x2 - x1
                SUBW    drawLine_x1
                STW     drawLine_sx
                ANDW    drawLine_sgn        
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
                ANDW    drawLine_sgn
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

drawL_ext       LDWI    drawLineExt
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

drawL_num       LDW     drawLine_sx         ; numerator = sx
                STW     drawLine_num
                STW     drawLine_count      ; for(count=sx; counti>=0; --i)
                
drawL_loop      LDWI    drawLinePixel
                CALL    giga_vAC
                
                LDW     drawLine_num        ; numerator += sy
                ADDW    drawLine_sy
                STW     drawLine_num
                SUBW    drawLine_sx
                BLE     drawL_swap          ; if(numerator <= sx) goto swap
                
                STW     drawLine_num        ; numerator -= sx
                LDW     drawLine_dx1        
                ADDW    drawLine_x1
                STW     drawLine_x1         ; x1 += dx1
                LDW     drawLine_dy1        
                ADDW    drawLine_y1
                STW     drawLine_y1         ; y1 += dy1
                BRA     drawL_count
                
drawL_swap      LDW     drawLine_dx2        
                ADDW    drawLine_x1
                STW     drawLine_x1         ; x1 += dx2
                LDW     drawLine_dy2        
                ADDW    drawLine_y1
                STW     drawLine_y1         ; y1 += dy2
                
drawL_count     LDW     drawLine_count
                SUBI    0x01
                STW     drawLine_count
                BGT     drawL_loop
                POP
                RET
%ENDS

%SUB            drawLinePixel
drawLinePixel   LD      drawLine_x1         
                ST      drawLine_tmp
                LD      drawLine_y1
                ADDI    0x08
                ST      drawLine_tmp+1      ; generate pixel address
    
                LD      fgbgColour + 1
                POKE    drawLine_tmp        ; plot new pixel
                RET
%ENDS                