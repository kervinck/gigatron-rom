; tetromino data
tetromino_I     EQU     0x08A1          ; format: colour for 2 pixels, w, h, ox, oy, x0, y0, x1, y1, x2, y2, x3, y3, 0, 0  ; ox and oy are offsets within w and h
tetromino_J     EQU     tetromino_I + 0x0100
tetromino_L     EQU     tetromino_I + 0x0200
tetromino_O     EQU     tetromino_I + 0x0300
tetromino_S     EQU     tetromino_I + 0x0400
tetromino_T     EQU     tetromino_I + 0x0500
tetromino_Z     EQU     tetromino_I + 0x0600                                                                                  
tetromino_I     DB      0x3C 0x3C 4 1 0 1 0 1 1 1 2 1 3 1 0 0  0x3C 0x3C 1 4 2 0 2 0 2 1 2 2 2 3 0 0  0x3C 0x3C 4 1 0 2 0 2 1 2 2 2 3 2 0 0  0x3C 0x3C 1 4 1 0 1 0 1 1 1 2 1 3 0 0
tetromino_J     DB      0x30 0x30 3 2 0 0 0 0 0 1 1 1 2 1 0 0  0x30 0x30 2 3 1 0 1 0 2 0 1 1 1 2 0 0  0x30 0x30 3 2 0 1 0 1 1 1 2 1 2 2 0 0  0x30 0x30 2 3 0 0 1 0 1 1 0 2 1 2 0 0  
tetromino_L     DB      0x0B 0x0B 3 2 0 0 2 0 0 1 1 1 2 1 0 0  0x0B 0x0B 2 3 1 0 1 0 1 1 1 2 2 2 0 0  0x0B 0x0B 3 2 0 1 0 1 1 1 2 1 0 2 0 0  0x0B 0x0B 2 3 0 0 0 0 1 0 1 1 1 2 0 0
tetromino_O     DB      0x0F 0x0F 2 2 0 0 0 0 1 0 0 1 1 1 0 0  0x0F 0x0F 2 2 0 0 0 0 1 0 0 1 1 1 0 0  0x0F 0x0F 2 2 0 0 0 0 1 0 0 1 1 1 0 0  0x0F 0x0F 2 2 0 0 0 0 1 0 0 1 1 1 0 0
tetromino_S     DB      0x0C 0x0C 3 2 0 0 1 0 2 0 0 1 1 1 0 0  0x0C 0x0C 2 3 1 0 1 0 1 1 2 1 2 2 0 0  0x0C 0x0C 3 2 0 1 1 1 2 1 0 2 1 2 0 0  0x0C 0x0C 2 3 0 0 0 0 0 1 1 1 1 2 0 0
tetromino_T     DB      0x33 0x33 3 2 0 0 1 0 0 1 1 1 2 1 0 0  0x33 0x33 2 3 1 0 1 0 1 1 2 1 1 2 0 0  0x33 0x33 3 2 0 1 0 1 1 1 2 1 1 2 0 0  0x33 0x33 2 3 0 0 1 0 0 1 1 1 1 2 0 0
tetromino_Z     DB      0x03 0x03 3 2 0 0 0 0 1 0 1 1 2 1 0 0  0x03 0x03 2 3 1 0 2 0 1 1 2 1 1 2 0 0  0x03 0x03 3 2 0 1 0 1 1 1 1 2 2 2 0 0  0x03 0x03 2 3 0 0 1 0 0 1 1 1 0 2 0 0