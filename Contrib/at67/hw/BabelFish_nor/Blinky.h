// Annotated hexdump for Blinky.gt1

  0x7f, 0x00, 0x0b, // Segment 7f00: 11 bytes
  0x11, 0x50, 0x44, // 7f00 LDWI $4450  ; Load address of center of screen
  0x2b, 0x30,       // 7f03 STW  'p'    ; Store in variable 'p' (at $0030)
  0xf0, 0x30,       // 7f05 POKE 'p'    ; Write low byte of accumulator there
  0xe3, 0x01,       // 7f07 ADDI 1      ; Increment accumulator
  0x90, 0x03,       // 7f09 BRA  $7f05  ; Loop forever
  0x00,             // No more segments
  0x7f, 0x00        // Start address
