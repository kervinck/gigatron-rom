; "15 puzzle" for the Apple I

; Jeff Jetton
; Februrary 2020

; Written for the dasm assembler, but should assemble under others
; with a few tweaks here and there.



        processor 6502
        
        ; Contants
KBD     equ $D010           ; Location of input character from keyboard
KBDCR   equ $D011           ; Keyboard control: Indicator that a new input
                            ;                   character is ready
PRBYTE  equ $FFDC           ; WozMon routine to diaplay register A in hex
ECHO    equ $FFEF           ; WozMon routine to display register A char
GETLINE equ $FF1F           ; Entry point back to WozMonitor
ERR_MAX equ $09             ; Bad game inputs before redisplaying board


        ; Zero-page variables
        seg.u VARS

        org $0040

PUZZ    ds 16   ; The puzzle board state
CURMOV  ds 1    ; ASCII code of the letter chosen as the current move
CUROFF  ds 1    ; Offset in the board of the location of that move's letter
EMPOFF  ds 1    ; Current offset of the empty space
TEMPCMP ds 1    ; Stores some temp bits used for row/col comparisons in HNDLMOV
ADJFAC  ds 1    ; Adjustment factor to use when shifting tiles in HNDLMOV
DIFFLVL ds 1    ; Difficulty level (raw ascii of number chosen)
MOVELO  ds 1    ; Tracks total number of moves
MOVEHI  ds 1    ;   ...using two bytes of binary-coded decimal (0000-9999)
TXTLO   ds 1    ; Where the printxt routine looks for the string address
TXTHI   ds 1    ;   (two bytes)
PRNG    ds 1    ; Running pseudo-random number generator
SHUFCTR ds 1    ; Counter of valid moves when "shuffling" new puzzle in INITPUZ
PREVMOV ds 1    ; Keeps track of previous randomly-selected move in INITPUZ
ERRCNT  ds 1    ; Tracks number of invalid moves in a row (to redisplay board)
        
        
; Main program  --------------------------------------------------------------

        seg CODE
        org $0400
        
        ; Init the program
        cld                 ; Start with BCD mode off
        ldx #$FF
        txs                 ; Reset stack to $FF
        lda #42             ; Set PNRG seed
        sta PRNG
        jsr INITPUZ         ; Set up an ordered puzzle board
        
        ; Show welcome message and ask if user wants instructions
        lda #<TXT_WELCOME   ; Store low byte of text data location
        sta TXTLO
        lda #>TXT_WELCOME   ; Store high byte of text data location
        sta TXTHI
        jsr PRINTXT         ; Call generic print function
        
        ; Get and handle answer to instructions question
        jsr GETYN
        bne NEWGAME         ; Skip to game if they didn't type "Y"
        lda #<TXT_INSTRUCT  ; Otherwise, print instructions...
        sta TXTLO
        lda #>TXT_INSTRUCT
        sta TXTHI
        jsr PRINTXT
        jsr PRINPUZ     ; Show the (currently solved) puzzle as an example

NEWGAME jsr GETDIFF     ; Ask for difficulty level
        jsr SHUFPUZ     ; Shuffle up a new puzzle and reset move counter
        jsr PRINPUZ     ; Print initial board state

NXTMOVE jsr INCMOVE     ; Handle next move by first incrementing  move counter
        jsr PRINMOV     ; Show that move number
GETMOVE jsr GETKEY      ; Grab keyboard input
        cmp #"Q"        ; Is user trying to quit?
        bne NOQUIT
        jmp ENDGAME
        
NOQUIT  jsr HNDLMOV     ; Get move and, if valid, update puzzle board
        lda CUROFF      ; Is the current offset 16 (i.e. move was invalid?)
        cmp #16
        bne SKIPERR     ; Skip error display if the move was valid
        dec ERRCNT      ; Reprint puzzle every ERROR_MAX errors
        bne SKIPRP
        jsr PRINPUZ     ; (this also resets ERRCNT)
SKIPRP  jsr PRINERR     ; Show error message
        jmp GETMOVE     ; Get new move, but don't incr. counter or show move #
        
SKIPERR jsr PRINPUZ     ; Display current board state
        jsr NEWLINE
        jsr CHKWIN      ; Check for a winning board (sets Z if so)
        beq WINNER
        jmp NXTMOVE
        
WINNER  jsr PRINYAY         ; Display a random interjection
        lda #<TXT_WINNER1   ; Display next part of winning message
        sta TXTLO
        lda #>TXT_WINNER1
        sta TXTHI
        jsr PRINTXT
        lda DIFFLVL         ; Show difficulty level
        jsr ECHO
        lda #<TXT_WINNER2   ; Display rest of winning message
        sta TXTLO
        lda #>TXT_WINNER2
        sta TXTHI
        jsr PRINTXT
        jsr PRINMOV         ; Display total moves
        
        lda #<TXT_REPLAY    ; Prompt for playing another round
        sta TXTLO
        lda #>TXT_REPLAY
        sta TXTHI
        jsr PRINTXT
        jsr GETYN           ; Get valid Y or N input
        bne ENDGAME
        jmp NEWGAME
        
ENDGAME lda #<TXT_BYE
        sta TXTLO
        lda #>TXT_BYE
        sta TXTHI
        jsr PRINTXT
        jmp GETLINE         ; Return to WozMonitor



        
; Subroutines   ***************************************************************


NEWLINE SUBROUTINE  ; Just print out a newline (destroys A)
        lda #$0D
        jmp ECHO



PRINTXT SUBROUTINE  ; Put string pointer in TXTLO & TXTHI before calling
        ldy #0          ; Y is the offset within the string
.loop   lda (TXTLO),Y   ; Load A with whatever's at pointer + Y
        beq .end        ; If char data is zero, that's the end
        jsr ECHO        ; Otherwise, print it
        iny
        jmp .loop
.end    rts             ; Note that we've destoyed A & Y



GETKEY  SUBROUTINE  ; Get one character of input and munge it into valid ASCII
                    ; Also cycles the PRNG while waiting for key input!
        jsr NEXTRND     ; Cycle  PRNG
        lda KBDCR       ; Check PIA for keyboard input
        bpl GETKEY      ; Loop if A is "positive" (bit 7 low)
        lda KBD         ; Get the keyboard character
        and #%01111111  ; Clear bit 7, which is always set for some reason
        jmp ECHO        ; Always echo what the user just typed (Thanks Marcel!)



GETYN   SUBROUTINE  ; Gets a valid Y or N response from user. Sets Z flag on Y.
        jsr GETKEY
        cmp #"N"
        beq .nope
        cmp #"Y"
        beq .yup
        jsr PRINERR
        jmp GETYN       ; Try again
.nope   tsx             ; This clears the zero flag
.yup    rts



NEXTRND SUBROUTINE  ; Cycle the PRNG (simple 8-bit Xorshift)
        lda PRNG
        asl
        bcc .noeor
        eor #$A9
.noeor  sta PRNG
        rts



PRINPUZ SUBROUTINE  ; Display the current puzzle state
                    ; And reset the error counter
        jsr NEWLINE
        jsr NEWLINE
        ldx #0      ; Offset into PUZZ data
        ldy #4      ; Counts columns, for linebreaks
.loop   lda PUZZ,X
        jsr ECHO    ; Print current slot
        dey
        bne .skipln ; Skip to inx if not at end of line
        ldy #4      ; Otherwise reset y and do a newline
        jsr NEWLINE
.skipln inx
        cpx #16
        bne .loop
        jsr NEWLINE
        lda #ERR_MAX
        sta ERRCNT
        rts



INCMOVE SUBROUTINE  ; Does some BCD fussing around to increment the counter
        sed
        lda MOVELO
        clc
        adc #1
        sta MOVELO
        lda MOVEHI
        adc #0      ; Add anything in the carry bit to the high byte
        sta MOVEHI
        cld
        rts
        
        
        
PRINMOV SUBROUTINE  ; Displays move number. Destroys A.
        lda MOVEHI
        beq .low    ; Skip to the low byte if high byte is still zero
        jsr PRBYTE
.low    lda MOVELO
        jsr PRBYTE
        rts



PRINERR SUBROUTINE  ; Displays a standard input error message
        lda #<TXT_BADMOVE
        sta TXTLO
        lda #>TXT_BADMOVE
        sta TXTHI
        jmp PRINTXT



PRINYAY SUBROUTINE      ; Prints a randomly-selected expression of joy
        lda PRNG        ; Put latest random number in A
        and #%00111000  ; Get bits 3-5 as a "multiple of 8" offset
        tax
.loop   lda YAYLKP,X
        beq .done
        jsr ECHO
        inx
        jmp .loop
.done   rts



INITPUZ SUBROUTINE  ; Create a new, ordered puzzle
        ; Put the space in the last byte of the puzz data
        lda #" "
        sta PUZZ + 15
        lda #15
        sta EMPOFF
        ; Work backwards through alphabet for the rest of the puzzle
        ldx #15
        ldy #"O"
.loop   tya
        sta.wx  PUZZ-1  ; STA PUZZ-1,X
        dey
        dex
        bne .loop
        rts



SHUFPUZ SUBROUTINE  ; Randomly shuffle puzzle board and init game variables
        ; Reset move counter and seed previous valid move variable
        lda #0
        sta MOVELO
        sta MOVEHI
        sta PREVMOV
        ; Get number of "shuffle" moves based on DIFFLVL and DIFFLKP table
        ldx DIFFLVL
        lda.wx DIFFLKP-"1"  ; DIFFLKP-"1", X
        sta SHUFCTR
        ; Push tiles around at random for SHUFCTR number of valid moves
.loop2  jsr NEXTRND
        lda PRNG
        and #%00001111  ; Get lower 4 bits of current random value
        clc
        adc #"A"        ; Convert random 0-15 value to ascii A-P
        cmp PREVMOV     ; We don't want the same letter as last valid move
        beq .loop2
        jsr HNDLMOV     ; Try making that move
        lda CUROFF      ; ...was it valid?
        cmp #16
        beq .loop2      ; No? Then it doesn't count. Try again.
        lda CURMOV      ; Otherwise, it "counts", so we'll remember the move
        sta PREVMOV
        dec SHUFCTR     ; ...and decrement the counter
        bne .loop2
        ; As long as the number of "shuffles" is odd, it's unlikely that we've
        ; randomly wound up back at a solved board. But since it COULD happen
        ; (moreso at low diff levels), we'd better check and redo if so...
        jsr CHKWIN      ; CHKWIN puts 0 in A right before returning if a winner
        beq SHUFPUZ
        rts

    
        
HNDLMOV SUBROUTINE  ; Handle the current move in register A

        ; There are three steps to be done:
        ;   1. Figure out the offset of the chosen letter (if a valid letter)
        ;   2. Determine the adjustment factor based on the relationship between
        ;      the chosen tile and the empty space, or mark move as invalid
        ;   3. Use the adjustment factor to shift the tile(s) appropriately
        
        ; Step 1: Did the user pick a letter that's on the board? If so, where?
        sta CURMOV      ; Store input letter as current move
        cmp #" "        ; Did user type a space?
        beq .badmv
        ldx #0
.loop   lda PUZZ,X      ; Look at one of the tiles on the board
        cmp CURMOV      ; Is it the current move?
        beq .found
        inx             ; Nope. Bump up the offset...
        cpx #16         ; Are we at the end of the board?
        bne .loop       ; No? Keep checking
        jmp .badmv      ; Otherwise, we're done. User chose a weird letter.

        ; Step 2: Is the tile moveable toward the space? If so, how?
.found  stx CUROFF
        ; A moveable tile must be on the same row or column as the empty space
        ; First check if CUROFF is on the same ROW as EMPOFF (bits 2 & 3 match)
        txa
        and #%00001100  ; Isolate bits 2 & 3 of the offset of the current move
        sta TEMPCMP     ; Remember the result
        lda EMPOFF
        and #%00001100  ; Isolate bits 2 & 3 of the offset of the empty spot
        cmp TEMPCMP     ; Do the bits match?
        bne .chkcol     ; No? Bummer. Branch ahead and check for same column.
        txa             ; Yes? Subtract EMPOFF from CUROFF (which is still in X)
        sec
        sbc EMPOFF
        bpl .posadj
        lda #-1         ; CUROFF < EMPOFF = adjustment factor of -1
        jmp .shift
.posadj lda #1          ; CUROFF > EMPOFF = adjustment factor of +1
        jmp .shift
        
.chkcol ; Check if CUROFF is on the same COLUMN as EMPOFF (bits 0 & 1 match)
        txa             ; Get current move offset from X again
        and #%00000011  ; Isolate bits 0 & 1
        sta TEMPCMP
        lda EMPOFF
        and #%00000011  ; Same for offset of empty space
        cmp TEMPCMP     ; If they match, they're in the same column
        beq .setadj
.badmv  ldx #16         ; Load X with 16, indicating an invalid move
        stx CUROFF
        rts             ; Back to the game at hand
        
.setadj txa
        sec
        sbc EMPOFF
        bpl .posad2
        lda #-4         ; CUROFF < EMPOFF = adjustment factor of -4 (1 row "up")
        jmp .shift
.posad2 lda #4          ; CUROFF > EMPOFF = adjustment factor of 4 (1 row down)

        ; Step 3: Shift the tiles appropriately.
        ; This section takes care of swaping the space with an adjacent tile,
        ; over and over until the space is in the original move offset.
        ; The amount/direction by which the space is shifted each time is the
        ; "adjustment factor" which lives register in A by this point.
.shift  sta ADJFAC      ; A contains the current adjustment factor. Remember it.
.again  lda EMPOFF      ; Put current EMPOFF into X
        tax
        clc
        adc ADJFAC      ; Temp offset is current empoff plus adjustment factor
        tay             ; Maintain that temp offset in Y
        lda PUZZ,Y      ; Put whatever's in temp offset into A
        sta PUZZ,X      ; And store it where the empty space was
        lda #" "        ; The empty space...
        sta PUZZ,Y      ; Goes where the temp offset is
        sty EMPOFF      ; And that temp offset is the new empty space offset
        cpy CUROFF      ; Is the temp offset where the original move offset is?
        bne .again
        rts



CHKWIN  SUBROUTINE      ; Compare current board to sorted board
        ldx #15
        lda #"O"
        sta TEMPCMP
.loop   lda.wx PUZZ-1   ; lda PUZZ-1,X
        cmp TEMPCMP
        bne .nowin
        dec TEMPCMP
        dex
        bne .loop
        ; If we made it this far, we have a winning board and the Zero flag
        ; will be set (due to the previous dex), indicating the win
.nowin  rts     ; But if we jumped here, Z will be unset (due to failed cmp)
                ; indicating a non-winning board state

        
        

GETDIFF SUBROUTINE      ; Prompt for and get/set difficulty level
                        ; Check for valid input and puts result in DIFFLVL
        lda #<TXT_DIFFASK
        sta TXTLO
        lda #>TXT_DIFFASK
        sta TXTHI
        jsr PRINTXT
.input  jsr GETKEY
        sta DIFFLVL     ; Store input as ASCII code, not as actual number
        cmp #"1"        ; Compare input (in register A) to ascii 1
        bpl .nxtchk     ; If result is positive, input was >= 1. So far so good.
        jmp .inval
.nxtchk lda #"5"        ; Subtract input from ascii 5
        sec
        sbc DIFFLVL
ZERO    bmi .inval      ; If result is negative, input was > 5 and invalid
        rts             ; At this point, we're good, so return
.inval  jsr PRINERR
        jmp .input
        


; Stored data   ****************************************************************

DIFFLKP ; Lookup table translating difficulty levels to number of "shuffles"
        ; Low values should be odd (reduced chance of shuffling a solved board)
        .byte 3     ; Level 1, only three moves
        .byte 9     ; 2
        .byte 19    ; 3
        .byte 35    ; 4
        .byte 255   ; Level 5, the full monty
        
YAYLKP  ; Lookup table of strings for a random winner message
        ; First seven strings are padded to ensure 8 byte offsets
        dc "HOORAY!"
        .byte 0
        dc "HUZZAH!"
        .byte 0
        dc "WOOHOO!"
        .byte 0
        dc "YIPPIE!"
        .byte 0
        dc "SWEET!"
        .byte 0
        .byte 0
        dc "COOL!"
        .byte 0
        .byte 0
        .byte 0
        dc "NICE!"
        .byte 0
        .byte 0
        .byte 0
        dc "GADZOOKS!"
        .byte 0

TXT_WELCOME
        .byte $0D
        .byte $0D
        dc "15 PUZZLE - BY JEFF JETTON"
        .byte $0D
        .byte $0D
        dc "INSTRUCTIONS (Y/N)? "
        .byte $00

TXT_INSTRUCT
        .byte $0D
        .byte $0D
        dc "TYPE A LETTER THAT'S ON THE SAME ROW OR"
        .byte $0D
        dc "COLUMN AS THE EMPTY SPACE TO SLIDE THAT"
        .byte $0D
        dc "LETTER (AND ANY IN BETWEEN) TOWARD THE"
        .byte $0D
        dc "SPACE.  TYPE Q TO QUIT."
        .byte $0D
        .byte $0D
        dc "SOLVED PUZZLE LOOKS LIKE THIS:"
        .byte $0D
        .byte $00
        
TXT_DIFFASK
        .byte $0D
        .byte $0D
        dc "DIFFICULTY LEVEL (1-5)? "
        .byte $00

TXT_BADMOVE
        .byte $0D
        dc "INVALID CHOICE.  TRY AGAIN: "
        .byte $00
        
TXT_WINNER1
        dc "  YOU SOLVED A LEVEL "
        .byte $00
        
TXT_WINNER2
        dc " PUZZLE!"
        .byte $0D
        .byte $0D
        dc "TOTAL MOVES: "
        .byte $00
        
TXT_REPLAY
        .byte $0D
        .byte $0D
        dc "PLAY AGAIN (Y/N)? "
        .byte $00

TXT_BYE
        .byte $0D
        .byte $0D
        dc "BYE!"
        .byte $0D
        .byte $0D
        byte $00
