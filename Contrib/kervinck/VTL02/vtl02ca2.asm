;234567890123456789012345678901234567890123456789012345
    .lf  vtl02ca2.lst
    .cr  6502
    .tf  vtl02ca2.obj,ap1
;-----------------------------------------------------;
;             VTL-2 for the 6502 (VTL02C)             ;
;           Original Altair 680b version by           ;
;          Frank McCoy and Gary Shannon 1977          ;
;    2012: Adapted to the 6502 by Michael T. Barry    ;
; Thanks to sbprojects.com for a very nice assembler! ;
;-----------------------------------------------------;
;        Copyright (c) 2012, Michael T. Barry
;       Revision B (c) 2015, Michael T. Barry
;       Revision C (c) 2015, Michael T. Barry
;               All rights reserved.
;
; Redistribution and use in source and binary forms,
;   with or without modification, are permitted,
;   provided that the following conditions are met: 
;
; 1. Redistributions of source code must retain the
;    above copyright notice, this list of conditions
;    and the following disclaimer. 
; 2. Redistributions in binary form must reproduce the
;    above copyright notice, this list of conditions
;    and the following disclaimer in the documentation
;    and/or other materials provided with the
;    distribution. 
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS
; AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
; WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
; FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
; SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
; FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
; EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
; NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
; SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
; LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
; OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
; IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
; ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;-----------------------------------------------------;
; Except for the differences discussed below, VTL02 was
;   designed to duplicate the OFFICIALLY DOCUMENTED
;   behavior of Frank's 680b version, detailed here:
;     http://www.altair680kit.com/manuals/Altair_
;     680-VTL-2%20Manual-05-Beta_1-Searchable.pdf
;   These versions ignore all syntax errors and plow
;   through VTL-2 programs with the assumption that
;   they are "correct", but in their own unique ways,
;   so any claims of compatibility are null and void
;   for VTL-2 code brave (or stupid) enough to stray
;   from the beaten path.
;
; Differences between the 680b and 6502 versions:
; * {&} and {*} are initialized on entry.
; * Division by zero returns 65535 for the quotient and
;     the dividend for the remainder (the original 6800
;     version froze).
; * The 6502 has NO 16-bit registers (other than PC)
;     and less overall register space than the 6800,
;     so the interpreter reserves some obscure VTL02C
;     variables {@ $ ( ) 0 1 2 3 4 5 6 7 8 9 < > : ?}
;     for its internal use (the 680b version used a
;     similar tactic, but differed in the details).
;     The deep nesting of parentheses also puts {; < =}
;     in danger of corruption.  For example, executing
;     the statement A=((((((((1)))))))) sets both {A}
;     and {;} to the value 1.
; * Users wishing to call a machine language subroutine
;     via the system variable {>} must first set the
;     system variable {"} to the proper address vector
;     (for example, "=768).
; * The x register is used to point to a simple VTL02C
;     variable (it can't point explicitly to an array
;     element like the 680b version because it's only
;     8-bits).  In the comments, var[x] refers to the
;     16-bit contents of the zero-page variable pointed
;     to by register x (residing at addresses x, x+1).
; * The y register is used as a pointer offset inside
;     a VTL02C statement (easily handling the maximum
;     statement length of about 128 bytes).  In the
;     comments, @[y] refers to the 16-bit address
;     formed by adding register y to the value in {@}.
; * The structure and flow of this interpreter are
;     similar to the 680b version, but have been
;     reorganized in a more 6502-friendly format (the
;     6502 has no 'bsr' instruction, so the 'stuffing'
;     of subroutines within 128 bytes of the caller is
;     only advantageous for conditional branches).
; * This version is based on the original port, which
;     was wound rather tightly, in a failed attempt to
;     fit it into 768 bytes like the 680b version; many
;     structured programming principles were sacrificed
;     in that effort.  The 6502 simply requires more
;     instructions than the 6800 does to manipulate 16-
;     bit quantities, but the overall execution speed
;     should be comparable due to the 6502's slightly
;     lower average clocks/instruction ratio.  As it is
;     now, it fits into 1KB with just a few bytes to
;     spare, but is more feature-laden than the 680b
;     interpreter whence it came.  Beginning with
;     Revision C, I tried to strike a tasteful balance
;     between execution speed and code size, but I
;     stubbornly kept it under 1024 ROMable bytes and
;     used only documented op-codes that were supported
;     by the original NMOS 6502 (without the ROR bug).
;     I may have missed a few optimizations -- further
;     suggestions are welcome.
; * VTL02C is my free gift (?) to the world.  It may be
;     freely copied, shared, and/or modified by anyone
;     interested in doing so, with only the stipulation
;     that any liabilities arising from its use are
;     limited to the price of VTL02C (nothing).
;-----------------------------------------------------;
; 2015: Revision B included some space optimizations
;         (suggested by dclxvi) and enhancements
;         (suggested by mkl0815 and Klaus2m5):
;
; * Bit-wise operators & | ^ (and, or, xor)
;   Example:  A=$|128) Get a char and set hi-bit
;
; * Absolute addressed 8-bit memory load and store
;   via the {< @} facility:
;   Example:  <=P) Point to the I/O port at P
;             @=@&254^128) Clear low-bit & flip hi-bit
;
; * Starting with VTL02B, the space character is no
;     longer a valid user variable nor a "valid" binary
;     operator.  It's now only significant as a numeric
;     constant terminator and as a place-holder in
;     strings and program listings, where it may be
;     used to improve human readability (at a slight
;     cost in execution speed and memory consumption).
;   Example:
;   *              (VTL-2)
;       1000 A=1)         Init loop index
;       1010 ?=A)           Print index
;       1020 ?="")          Newline
;       1030 A=A+1)         Update index
;       1040 #=A<10*1010) Loop until done
;
;   *              (VTL02B)
;       1000 A = 1             ) Init loop index
;       1010     ? = A         )   Print index
;       1020     ? = ""        )   Newline
;       1030     A = A + 1     )   Update index
;       1040 # = A < 10 * 1010 ) Loop until done
;
; 2015: Revision C includes further enhancements
;   (suggested by Klaus2m5):
;
; * "THEN" and "ELSE" operators [ ]
;     A[B returns 0 if A is 0, otherwise returns B.
;     A]B returns B if A is 0, otherwise returns 0.
;
; * Some effort was made to balance interpreter code
;     density with interpreter performance, while
;     remaining within the 1KB constraint.  Structured
;     programming principles remained at low priority.
;-----------------------------------------------------;
; VTL02C variables occupy RAM addresses $0080 to $00ff,
;   and are little-endian, in the 6502 tradition.
; The use of lower-case and some control characters for
;   variable names is allowed, but not recommended; any
;   attempts to do so would likely result in chaos, due
;   to aliasing with upper-case and system variables.
; Variables tagged with an asterisk are used internally
;   by the interpreter and may change without warning.
;   {@ $ ( ) 0..9 : > ?} are (usually) intercepted by
;   the interpreter, so their internal use by VTL02C is
;   "safe".  The same cannot be said for {; < =}, so be
;   careful!
at       = $80      ; {@}* internal pointer / mem byte
; VTL02C standard user variable space
;                     {A B C .. X Y Z [ \ ] ^ _}
; VTL02C system variable space
space    = $c0      ; { }  Starting with VTL02B:  the
;                       space character is no longer a
;                       valid user variable nor a
;                       "valid" binary operator.
;                       It is now only significant as a
;                       numeric constant terminator and
;                       as a place-holder in strings
;                       and program listings. 
bang     = $c2      ; {!}  return line number
quote    = $c4      ; {"}  user ml subroutine vector
pound    = $c6      ; {#}  current line number
dolr     = $c8      ; {$}* temp storage / char i/o
remn     = $ca      ; {%}  remainder of last division
ampr     = $cc      ; {&}  pointer to start of array
tick     = $ce      ; {'}  pseudo-random number
lparen   = $d0      ; {(}* old line # / begin sub-exp
rparen   = $d2      ; {)}* temp storage / end sub-exp
star     = $d4      ; {*}  pointer to end of free mem
;          $d6      ; {+ , - . /}  valid variables
; Interpreter argument stack space
arg      = $e0      ; {0 1 2 3 4 5 6 7 8 9 :}*
; Rarely used variables and argument stack overflow
;          $f6      ; {;}* valid user variable
lthan    = $f8      ; {<}* user memory byte pointer
;        = $fa      ; {=}* valid user variable
gthan    = $fc      ; {>}* temp / call ML subroutine
ques     = $fe      ; {?}* temp / terminal i/o
;
nulstk   = $01ff    ; system stack resides in page 1
;-----------------------------------------------------;
; Equates for a 48K+ Apple 2 (original, +, e, c, gs)
ESC      = 27       ; "Cancel current input line" key
BS       = 8        ; "Delete last keypress" key
OP_OR    = '!'      ; Bit-wise OR operator
linbuf   = $0200    ; input line buffer
prgm     = $0800    ; VTL02B program grows from here
himem    = $8000    ;   ... up to the top of user RAM
vtl02c   = $8000    ; interpreter cold entry point
;                     (warm entry point is startok)
KBD      = $c000    ; 128 + keypress if waiting
KEYIN    = $fd0c    ; apple monitor keyin routine
COUT     = $fded    ; apple monitor charout routine
;=====================================================;
    .or  vtl02c
;-----------------------------------------------------;
; Initialize program area pointers and start VTL02C
; 17 bytes
    lda  #prgm
    sta  ampr       ; {&} -> empty program
    lda  /prgm
    sta  ampr+1
    lda  #himem
    sta  star       ; {*} -> top of user RAM
    lda  /himem
    sta  star+1
startok:
    sec             ; request "OK" message
; - - - - - - - - - - - - - - - - - - - - - - - - - - ;
; Start/restart VTL02C command line with program intact
; 32 bytes
start:
    cld             ; a sensible precaution
    ldx  #nulstk
    txs             ; drop whatever is on the stack
    bcc  user       ; skip "OK" if carry clear
    jsr  outnl
    lda  #'O'       ; output \nOK\n to terminal
    jsr  outch
    lda  #'K'
    jsr  outch
    jsr  outnl
user:
    jsr  inln       ; input a line from the user
    ldx  #pound     ; cvbin destination = {#}
    jsr  cvbin      ; does line start with a number?
    beq  direct     ;   no: execute direct statement
; - - - - - - - - - - - - - - - - - - - - - - - - - - ;
; Delete/insert/replace program line or list program
; 7 bytes
stmnt:
    clc
    lda  pound
    ora  pound+1    ; {#} = 0?
    bne  skp2       ;   no: delete/insert/replace line
; - - - - - - - - - - - - - - - - - - - - - - - - - - ;
; List program to terminal and restart "OK" prompt
; entry:  Carry must be clear
; uses:   findln:, outch:, prnum:, prstr:, {@ ( )}
; exit:   to command line via findln:
; 20 bytes
list_:
    jsr  findln     ; find program line >= {#}
    ldx  #lparen    ; line number for prnum
    jsr  prnum      ; print the line number
    lda  #' '       ; print a space instead of the
    jsr  outch      ;   line length byte
    lda  #0         ; zero for delimiter
    jsr  prstr      ; print the rest of the line
    bcs  list_      ; (always taken)
;-----------------------------------------------------;
; The main program execution loop
; entry:  with (cs) via "beq direct" in user:
; exit:   to command line via findln: or "beq start"
; 45 bytes
progr:
    beq  eloop0     ; if {#} = 0 then ignore and
    ldy  lparen+1   ;   continue (false branch)
    ldx  lparen     ; else did {#} change?
    cpy  pound+1    ;   yes: perform a branch, with
    bne  branch     ;     carry flag conditioned for
    cpx  pound      ;     the appropriate direction.
    beq  eloop      ;   no: execute next line (cs)
branch:
    inx             ;   execute a VTL02B branch
    bne  branch2
    iny
branch2:
    stx  bang       ;   {!} = {(} + 1 (return ptr)
    sty  bang+1
eloop0:
    rol
    eor  #1         ; complement carry flag
    ror
eloop:
    jsr  findln     ; find first/next line >= {#}
    iny             ; skip over the length byte
direct:
    php             ; (cc: program, cs: direct)
    jsr  exec       ; execute one VTL02B statement
    plp
    lda  pound      ; update Z for {#}
    ora  pound+1    ; if program mode then continue
    bcc  progr      ; if direct mode, did {#} change?
    beq  start      ;   no: restart "OK" prompt
    bne  eloop0     ;   yes: execute program from {#}
;-----------------------------------------------------;
; Delete/insert/replace program line and restart the
;   command prompt (no "OK" means success)
; entry:  Carry must be clear
; uses:   find:, start:, linbuf, {@ > # & * (}
; 151 bytes
skp2:
    tya             ; save linbuf offset pointer
    pha
    jsr  find       ; point {@} to first line >= {#}
    bcs  insrt
    eor  pound      ; if line doesn't already exist
    bne  insrt      ; then skip deletion process
    cpx  pound+1
    bne  insrt
    tax             ; x = 0
    lda  (at),y
    tay             ; y = length of line to delete
    eor  #-1
    adc  ampr       ; {&} = {&} - y
    sta  ampr
    bcs  delt
    dec  ampr+1
delt:
    lda  at
    sta  gthan      ; {>} = {@}
    lda  at+1
    sta  gthan+1
delt2:
    lda  gthan
    cmp  ampr       ; delete the line
    lda  gthan+1
    sbc  ampr+1
    bcs  insrt
    lda  (gthan),y
    sta  (gthan,x)
    inc  gthan
    bne  delt2
    inc  gthan+1
    bcc  delt2      ; (always taken)
insrt:
    pla
    tax             ; x = linbuf offset pointer
    lda  pound
    pha             ; push the new line number on
    lda  pound+1    ;   the system stack
    pha
    ldy  #2
cntln:
    inx  
    iny             ; determine new line length in y
    lda  linbuf-1,x ;   and push statement string on
    pha             ;   the system stack
    bne  cntln
    cpy  #4         ; if empty line then skip the
    bcc  jstart     ;   insertion process
    tax             ; x = 0
    tya
    clc
    adc  ampr       ; calculate new program end
    sta  gthan      ; {>} = {&} + y
    txa
    adc  ampr+1
    sta  gthan+1
    lda  gthan
    cmp  star       ; if {>} >= {*} then the program
    lda  gthan+1    ;   won't fit in available RAM,
    sbc  star+1     ;   so drop the stack and abort
    bcs  jstart     ;   to the "OK" prompt
slide:
    lda  ampr
    bne  slide2
    dec  ampr+1
slide2:
    dec  ampr
    lda  ampr
    cmp  at
    lda  ampr+1
    sbc  at+1
    bcc  move       ; slide open a gap inside the
    lda  (ampr,x)   ;   program just big enough to
    sta  (ampr),y   ;   hold the new line
    bcs  slide      ; (always taken)
move:
    tya
    tax             ; x = new line length
move2:
    pla             ; pull the statement string and
    dey             ;   the new line number and store
    sta  (at),y     ;   them in the program gap
    bne  move2
    ldy  #2
    txa
    sta  (at),y     ; store length after line number
    lda  gthan
    sta  ampr       ; {&} = {>}
    lda  gthan+1
    sta  ampr+1
jstart:
    jmp  start      ; drop stack, restart cmd prompt
;-----------------------------------------------------;
; Point @[y] to the first/next program line >= {#}
; entry:   (cc): start search at beginning of program
;          (cs): start search at next line
;          ({@} -> beginning of current line)
; used by: list_:, progr:
; uses:    find:, jstart:, prgm, {@ # & (}
; exit:    if line not found then abort to "OK" prompt
;          else {@} -> found line, x:a = {#} = {(} =
;            actual line number, y = 2, (cc)
; 10 bytes
findln:
    jsr  find       ; find first/next line >= {#}
    bcs  jstart     ; if end then restart "OK" prompt
    sta  pound      ; {#} = {(}
    stx  pound+1
    rts
;-----------------------------------------------------;
; {?="...} handler; called from exec:
; List line handler; called from list_:
; 2 bytes
prstr:
    iny             ; skip over the " or length byte
    tax             ; x = delimiter, fall through
; - - - - - - - - - - - - - - - - - - - - - - - - - - ;
; Print a string at @[y]
; x holds the delimiter char, which is skipped over,
;   not printed (a null byte is always a delimiter)
; If a key was pressed, it pauses for another keypress
;   before returning.  If either of those keys was a
;   ctrl-C, it drops the stack and restarts the "OK"
;   prompt with the user program intact
; entry:  @[y] -> string, x = delimiter char
; uses:   inch:, inkey:, jstart:, outch:, execrts:
; exit:   (normal) @[y] -> null or byte after delimiter
;         (ctrl-C) drop the stack & restart "OK" prompt
; 39 bytes
prmsg:
    txa
    cmp  (at),y     ; found delimiter or null?
    beq  prmsg2     ; yes: finish up
    lda  (at),y
    beq  prmsg2
    jsr  outch      ; no: print char to terminal
    iny             ;   and loop (with safety escape)
    bpl  prmsg
prmsg2:
    tax             ; save closing delimiter
    jsr  inkey      ; any key = pause?
    bcc  prout      ;   no: proceed
    jsr  inch       ;   yes: wait for another key
prout:
    txa             ; retrieve closing delimiter
    beq  outnl      ; always \n after null delimiter
    jsr  skpbyte    ; skip over the delimiter
    cmp  #';'       ; if trailing char is ';' then
    beq  execrts    ;   suppress the \n
outnl:
    lda  #$0d       ; \n to terminal
joutch:
    jmp  outch
;-----------------------------------------------------;
; Execute a (hopefully) valid VTL02C statement at @[y]
; entry:   @[y] -> left-side of statement
; uses:    nearly everything
; exit:    note to machine language subroutine {>=...}
;            users: no registers or variables are
;            required to be preserved except the system
;            stack pointer, the text base pointer {@},
;            and the original line number {(}
; if there is a {"} directly after the assignment
;   operator, the statement will execute as {?="...},
;   regardless of the variable named on the left side
; 84 bytes
exec:
    jsr  getbyte    ; fetch left-side variable name
    beq  execrts    ; do nothing with a null statement
    cmp  #')'       ; same for a full-line comment
    beq  execrts
    iny
    ldx  #arg       ; initialize argument pointer
    jsr  convp      ; arg[{0}] -> left-side variable
    jsr  getbyte    ; skip over assignment operator
    jsr  skpbyte    ; is right-side a literal string?
    cmp  #'"'       ;   yes: print the string with
    beq  prstr      ;     trailing ';' check & return
    ldx  #arg+2     ; point eval to arg[{1}]
    jsr  eval       ; evaluate right-side in arg[{1}]
    lda  arg+2
    ldy  #0
    ldx  arg+1      ; was left-side an array element?
    bne  exec3      ;   yes: skip to default actions
    ldx  arg
    cpx  #at        ; if {@=...} statement then poke
    beq  poke       ;   low half of arg[{1}] to ({<})
    cpx  #dolr      ; if {$=...} statement then print
    beq  joutch     ;   arg[{1}] as ASCII character
    cpx  #ques      ; if {?=...} statement then print
    beq  prnum0     ;   arg[{1}] as unsigned decimal
    cpx  #gthan     ; if {>=...} statement then call
    beq  usr        ;   user-defined ml routine
exec3:
    sta  (arg),y
    adc  tick+1     ; store arg[{1}] in the left-side
    rol             ;   variable
    tax
    iny
    lda  arg+3
    sta  (arg),y
    adc  tick       ; pseudo-randomize {'}
    rol
    sta  tick+1
    stx  tick
execrts:
    rts
usr:
    tax             ; jump to user ml routine with
    lda  arg+3      ;   arg[{1}] in a:x (MSB:LSB)
    jmp  (quote)    ; {"} must point to valid 6502 code
poke:
    sta  (lthan),y
    rts
;-----------------------------------------------------;
; {?=...} handler; called by exec:
; 2 bytes
prnum0:
    ldx  #arg+2     ; x -> arg[{1}], fall through
; - - - - - - - - - - - - - - - - - - - - - - - - - - ;
; Print an unsigned decimal number (0..65535) in var[x]
; entry:   var[x] = number to print
; uses:    div:, outch:, var[x+2], saves original {%}
; exit:    var[x] = 0, var[x+2] = 10
; 43 bytes
prnum:
    lda  remn
    pha             ; save {%}
    lda  remn+1
    pha
    lda  #0         ; null delimiter for print
    pha    
    sta  3,x
    lda  #10        ; divisor = 10
    sta  2,x        ; repeat {
prnum2:
    jsr  div        ;   divide var[x] by 10
    lda  remn
    ora  #'0'       ;   convert remainder to ASCII
    pha             ;   stack digits in ascending
    lda  0,x        ;     order ('0' for zero)
    ora  1,x
    bne  prnum2     ; } until var[x] is 0
    pla
prnum3:
    jsr  outch      ; print digits in descending
    pla             ;   order until delimiter is
    bne  prnum3     ;   encountered
    pla
    sta  remn+1     ; restore {%}
    pla
    sta  remn
    rts
;-----------------------------------------------------;
; Evaluate a (hopefully) valid VTL02C expression at
;   @[y] and place its calculated value in arg[x]
; A VTL02C expression is defined as a string of one or
;   more terms, separated by operators and terminated
;   with a null or an unmatched right parenthesis
; A term is defined as a variable name, a decimal
;   constant, or a parenthesized sub-expression; terms
;   are evaluated strictly from left to right
; A variable name is defined as a user variable, an
;   array element expression enclosed in {: )}, or a
;   system variable (which may have side-effects)
; entry:   @[y] -> expression text, x -> argument
; uses:    getval:, oper:, {@}, argument stack area
; exit:    arg[x] = result, @[y] -> next text
; 31 bytes
eval:
    lda  #0
    sta  0,x        ; start evaluation by simulating
    sta  1,x        ;   {0+expression}
    lda  #'+'
notdn:
    pha             ; stack alleged operator
    inx             ; advance the argument stack
    inx             ;   pointer
    jsr  getval     ; arg[x+2] = value of next term
    dex
    dex
    pla             ; retrieve and apply the operator
    jsr  oper       ;   to arg[x], arg[x+2]
    jsr  getbyte    ; end of expression?
    beq  evalrts    ;   (null or right parenthesis)
    iny
    cmp  #')'       ;   no: skip over the operator
    bne  notdn      ;     and continue the evaluation
evalrts:
    rts             ;   yes: return with final result
;-----------------------------------------------------;
; Get numeric value of the term at @[y] into var[x]
; Some examples of valid terms:  123, $, H, (15-:J)/?)
; 83 bytes
getval:
    jsr  cvbin      ; decimal number at @[y]?
    bne  getrts     ;   yes: return with it in var[x]
    jsr  getbyte
    iny
    cmp  #'?'       ; user line input?
    bne  getval2
    tya             ;   yes:
    pha
    lda  at         ;     save @[y]
    pha             ;     (current expression ptr)
    lda  at+1
    pha
    jsr  inln       ; input expression from user
    jsr  eval       ; evaluate, var[x] = result
    pla
    sta  at+1
    pla
    sta  at         ; restore @[y]
    pla
    tay
    rts             ; skip over "?" and return
getval2:
    cmp  #'$'       ; user char input?
    bne  getval2a
    jsr  inch       ;   yes: input one char
    bcs  getval5    ;     (always taken)
getval2a:
    cmp  #'@'       ; memory access?
    bne  getval3
    sty  dolr       ;   yes:
    ldy  #0
    lda  (lthan),y  ;     access memory byte at ({<})
    ldy  dolr
    bne  getval5    ;     (always taken)
getval3:
    cmp  #'('       ; sub-expression?
    beq  eval       ;   yes: evaluate it recursively
    jsr  convp      ;   no: first set var[x] to the
    lda  (0,x)      ;     named variable's address,
    pha             ;     then replace that address
    inc  0,x        ;     with the variable's actual
    bne  getval4    ;     value before returning
    inc  1,x
getval4:
    lda  (0,x)
    sta  1,x        ; store high-byte of term value
    pla
getval5:
    sta  0,x        ; store low-byte of term value
getrts:
    rts
;-----------------------------------------------------;
; Set var[x] to the address of the variable named in a
; entry:   a holds variable name, @[y] -> text holding
;            array index expression (if a = ':')
; uses:    plus, eval, oper8d, {@ &}
; exit:    (eq): var[x] -> variable, @[y] unchanged
;          (ne): var[x] -> array element,
;                @[y] -> following text
; 26 bytes
convp:
    cmp  #':'       ; array element?
    bne  simple     ;   no: var[x] -> simple variable
    jsr  eval       ;   yes: evaluate array index at
    asl  0,x        ;     @[y] and advance y
    rol  1,x
    lda  ampr       ;     var[x] -> array element
    sta  2,x        ;       at address 2*index+&
    lda  ampr+1
    sta  3,x
    bne  plus       ;     (always taken)
; The following section is designed to translate the
;   named simple variable from its ASCII value to its
;   zero-page address.  In this case, 'A' translates
;   to $82, '!' translates to $c2, etc.  The method
;   employed must correspond to the zero-page equates
;   above, or strange and not-so-wonderful bugs will
;   befall the weary traveller on his or her porting
;   journey.
simple:
    asl             ; form simple variable address
    ora  #$80       ; mapping function is (a*2)|128
    bmi  oper8d     ; (always taken)
;-----------------------------------------------------;
; 16-bit unsigned multiply routine: var[x] *= var[x+2]
; exit:    overflow is ignored/discarded, var[x+2] and
;          {>} are modified, a = 0
; 40 bytes
mul:
    lda  0,x
    sta  gthan
    lda  1,x        ; {>} = var[x]
    sta  gthan+1
    lda  #0
    sta  0,x        ; var[x] = 0
    sta  1,x
mul2:
    lda  gthan
    ora  gthan+1
    beq  mulrts     ; exit early if {>} = 0
    lsr  gthan+1
    ror  gthan      ; {>} /= 2
    bcc  mul3
    jsr  plus       ; form the product in var[x]
mul3:
    asl  2,x
    rol  3,x        ; left-shift var[x+2]
    lda  2,x
    ora  3,x        ; loop until var[x+2] = 0
    bne  mul2
mulrts:
    rts
;-----------------------------------------------------;
; var[x] += var[x+2]
; 14 bytes
plus:
    clc
    lda  0,x
    adc  2,x
    sta  0,x
    lda  1,x
    adc  3,x
    sta  1,x
    rts
;-----------------------------------------------------;
; Apply the binary operator in a to var[x] and var[x+2]
; Valid VTL02C operators are {* + / [ ] - | ^ & < = >}
; {>} is defined as greater than _or_equal_
; An undefined operator will be interpreted as one of
;   the three comparison operators
; 37 bytes
oper:
    cmp  #'+'       ; addition operator?
    beq  plus
    cmp  #'*'       ; multiplication operator?
    beq  mul
    cmp  #'/'       ; division operator?
    beq  div
    cmp  #'['       ; "then" operator?
    beq  then_
    cmp  #']'       ; "else" operator?
    beq  else_
    dex             ; (factored from the following ops)
    cmp  #'-'       ; subtraction operator?
    beq  minus
    cmp  #OP_OR     ; bit-wise or operator?
    beq  or_
    cmp  #'^'       ; bit-wise xor operator?
    beq  xor_
    cmp  #'&'       ; bit-wise and operator?
    beq  and_
; - - - - - - - - - - - - - - - - - - - - - - - - - - ;
; Apply comparison operator in a to var[x] and var[x+2]
;   and place result in var[x] (1: true, 0: false)
; expects:  (cs), pre-decremented x
; 29 bytes
    eor  #'<'       ; 0: '<'  1: '='  2: '>'
    sta  gthan      ; other values in a are undefined,
    jsr  minus      ;   but _will_ produce some result
    dec  gthan      ; var[x] -= var[x+2]
    bne  oper8b     ; equality test?
    ora  0,x        ;   yes: 'or' high and low bytes
    beq  oper8c     ;     (cs) if 0
    clc             ;     (cc) if not 0
oper8b:
    lda  gthan
    rol
oper8c:
    adc  #0
    and  #1         ; var[x] = 1 (true), 0 (false)
oper8d:
    sta  0,x
    lda  #0
    beq  minus3     ; (always taken)
;-----------------------------------------------------;
; expects:  (cs)
; 14 bytes
then_:
    lda  0,x
    ora  1,x
    beq  minus4
    lda  2,x
    sta  0,x
    lda  3,x
    bcs  minus3     ; (always taken)
;-----------------------------------------------------;
; expects:  (cs)
; 10 bytes
else_:
    lda  0,x
    ora  1,x
    beq  plus
    lda  #0
    beq  oper8d     ; (always taken)
;-----------------------------------------------------;
; var[x] -= var[x+2]
; expects:  (cs), pre-decremented x
; 11 bytes
minus:
    jsr  minus2
    inx
minus2:
    lda  1,x
    sbc  3,x
minus3:
    sta  1,x
minus4:
    rts
;-----------------------------------------------------;
; var[x] &= var[x+2]
; expects:  (cs), pre-decremented x
; 10 bytes
and_:
    jsr  and_2
    inx
and_2:
    lda  1,x
    and  3,x
    bcs  minus3     ; (always taken)
;-----------------------------------------------------;
; var[x] |= var[x+2]
; expects:  (cs), pre-decremented x
; 10 bytes
or_:
    jsr  or_2
    inx
or_2:
    lda  1,x
    ora  3,x
    bcs  minus3     ; (always taken)
;-----------------------------------------------------;
; var[x] ^= var[x+2]
; expects:  (cs), pre-decremented x
; 10 bytes
xor_:
    jsr  xor_2
    inx
xor_2:
    lda  1,x
    eor  3,x
    bcs  minus3     ; (always taken)
;-----------------------------------------------------;
; 16-bit unsigned division routine
;   var[x] /= var[x+2], {%} = remainder, {>} modified
;   var[x] /= 0 produces {%} = var[x], var[x] = 65535
; 43 bytes
div:
    lda  #0
    sta  remn       ; {%} = 0
    sta  remn+1
    lda  #16
    sta  gthan      ; {>} = loop counter
div1:
    asl  0,x        ; var[x] is gradually replaced
    rol  1,x        ;   with the quotient
    rol  remn       ; {%} is gradually replaced
    rol  remn+1     ;   with the remainder
    lda  remn
    cmp  2,x
    lda  remn+1     ; partial remainder >= var[x+2]?
    sbc  3,x
    bcc  div2
    sta  remn+1     ;   yes: update the partial
    lda  remn       ;     remainder and set the
    sbc  2,x        ;     low bit in the partial
    sta  remn       ;     quotient
    inc  0,x
div2:
    dec  gthan
    bne  div1       ; loop 16 times
    rts
;-----------------------------------------------------;
; If text at @[y] is a decimal constant, translate it
;   into var[x] (discarding any overflow) and update y
; entry:   @[y] -> text containing possible constant;
;            leading space characters are skipped, but
;            any spaces encountered after a conversion
;            has begun will end the conversion.
; used by: user:, getval:
; uses:    mul:, plus:, var[x], var[x+2], {@ > ?}
; exit:    (ne): var[x] = constant, @[y] -> next text
;          (eq): var[x] = 0, @[y] unchanged
;          (cs): in all but the truly strangest cases
; 43 bytes
cvbin:
    lda  #0
    sta  0,x        ; var[x] = 0
    sta  1,x
    sta  3,x
    jsr  getbyte    ; skip any leading spaces
    sty  ques       ; save pointer
cvbin2:
    lda  (at),y     ; grab a char
    eor  #'0'       ; if char at @[y] is not a
    cmp  #10        ;   decimal digit then stop
    bcs  cvbin3     ;   the conversion
    pha             ; save decimal digit
    lda  #10
    sta  2,x
    jsr  mul        ; var[x] *= 10
    sta  3,x
    pla             ; retrieve decimal digit
    sta  2,x
    jsr  plus       ; var[x] += digit
    iny             ; loop for more digits
    bpl  cvbin2     ;   (with safety escape)
cvbin3:
    cpy  ques       ; (ne) if valid, (eq) if not
    rts
;-----------------------------------------------------;
; Accept input line from user and store it in linbuf,
;   zero-terminated (allows very primitive edit/cancel)
; entry:   (jsr to inln or newln, not inln6)
; used by: user:, getval:
; uses:    inch:, outnl:, linbuf, {@}
; exit:    @[y] -> linbuf
; 42 bytes
inln6:
    cmp  #ESC       ; escape?
    beq  newln      ;   yes: discard entire line
    iny             ; line limit exceeded?
    bpl  inln2      ;   no: keep going
newln:
    jsr  outnl      ;   yes: discard entire line
inln:
    ldy  #linbuf    ; entry point: start a fresh line
    sty  at         ; {@} -> input line buffer
    ldy  /linbuf
    sty  at+1
    ldy  #1
inln5:
    dey
    bmi  newln
inln2:
    jsr  inch       ; get (and echo) one key press
    cmp  #BS        ; backspace?
    beq  inln5      ;   yes: delete previous char
    cmp  #$0d       ; cr?
    bne  inln3
    lda  #0         ;   yes: replace with null
inln3:
    sta  (at),y     ; put key in linbuf
    bne  inln6      ; continue if not null
    tay             ; y = 0
    rts
;-----------------------------------------------------;
; Find the first/next stored program line >= {#}
; entry:   (cc): start search at program beginning
;          (cs): start search at next line
;          ({@} -> beginning of current line)
; used by: skp2:, findln:
; uses:    prgm, {@ # & (}
; exit:    (cs): {@}, x:a and {(} undefined, y = 2
;          (cc): {@} -> beginning of found line, y = 2,
;                x:a = {(} = actual found line number
; 62 bytes
find:
    ldx  /prgm
    lda  #prgm
    bcc  find1st    ; cc: search begins at first line
    ldx  at+1
    ldy  #2
findnxt:
    lda  at
    cmp  ampr
    lda  at+1
    sbc  ampr+1     ; {@} >= {&} (end of program)?
    bcs  findrts    ;   yes: search failed (cs)
find3:
    lda  at
    adc  (at),y     ;   no: {@} -> next line
    bcc  find5
    inx
find1st:
    stx  at+1
find5:
    sta  at
    ldy  #0
    lda  (at),y
    sta  lparen     ; {(} = current line number
    cmp  pound      ;   (invalid if {@} >= {&}, but
    iny             ;   we'll catch that later...)
    lda  (at),y
    sta  lparen+1
    sbc  pound+1    ; if {(} < {#} then try the next
    iny             ;   program line
    bcc  findnxt
    lda  at         ; {@} >= {&} (end of program)?
    cmp  ampr       ;   yes: search failed (cs)
    lda  at+1       ;   no: search succeeded (cc)
    sbc  ampr+1
    lda  lparen
    ldx  lparen+1
findrts:
    rts
;-----------------------------------------------------;
; Fetch a byte at @[y], ignoring space characters
; 10 bytes
skpbyte:
    iny             ; skip over current char
getbyte:
    lda  (at),y
    beq  getbyt2
    cmp  #' '
    beq  skpbyte    ; skip over any space char(s)
getbyt2:
    rts
;-----------------------------------------------------;
; Check for user keypress and return with (cc) if none
;   is pending.  Otherwise, fall through to inch
;   and return with (cs).
; 6 bytes
inkey:
    lda  KBD        ; is there a keypress waiting?
    asl
    bcc  outrts     ;   no: return with (cc)
; - - - - - - - - - - - - - - - - - - - - - - - - - - ;
; Read key from stdin into a, echo, (cs)
; drop stack and abort to "OK" prompt if ctrl-C
; 16 bytes
inch:
    sty  dolr       ; save y reg
    jsr  KEYIN      ; get a char from keyboard
    ldy  dolr       ; restore y reg
    and  #$7f       ; strip apple's hi-bit
    cmp  #$03       ; ctrl-C?
    bne  outch      ;   no: echo to terminal
    jmp  start      ;   yes: abort to "OK" prompt
; - - - - - - - - - - - - - - - - - - - - - - - - - - ;
; Print ASCII char in a to stdout, (cs)
; 9 bytes
outch:
    pha             ; save original char
    ora  #$80       ; apples prefer "high" ASCII
    jsr  COUT       ; emit char via apple monitor
    pla             ; restore original char
    sec             ; (by contract with callers)
outrts:
    rts
;-----------------------------------------------------;
    .en  vtl02ca2
