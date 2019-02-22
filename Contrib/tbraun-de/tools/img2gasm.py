#!/usr/local/bin/python3

from PIL import Image
import PIL
from array import array
import sys

copyLinesCode1 = """
xysource   EQU    0x30
xsource    EQU    0x30
ysource    EQU    0x31
xydest     EQU    0x32
ydest      EQU    0x33
xdest      EQU    0x32  
cnt        EQU    0x34         

copy_lines LDI     0xB0
           ST      cnt
"""
copyLinesCode2 = """           STW     xysource
           LDWI    0x5900
           STW     xydest
copy_next  LDW     xysource
           PEEK    xysource
           POKE    xydest
           INC     xsource
           INC     xdest
           INC     cnt
           LD      cnt
           BNE     copy_next
           ; when we're here, the line has been fully copied
           LD      xdest
           SUBI    0x50          ; have we just finished the first halv of a line?
           BEQ     next_half
           ; if not continue with the next line
           LDI     0x00
           ST      xdest
           INC     ydest
           LD      ydest
           SUBI    0x5c
           BEQ     end_copy      ; we copied all lines when ydest has reached 0x5c           
next_half  LDI     0xa0
           ST      xsource
           INC     ysource
           LDI     0xB0
           ST      cnt
           BRA     copy_next

end_copy   RET
"""

if(len(sys.argv) != 3):
  print("Usage: img2gasm.py <input.jpg> <output.gasm>")
  exit(0)
  
im = Image.open(sys.argv[1])
rgb_im = im.convert('RGB')
size = rgb_im.size
print("Image size: " + str(size))
scalefactor = min(1 / (size[0]/160), 1 / (size[1]/120))
print("Scalefactor: " + str(scalefactor))
newsize = tuple(int(scalefactor*x) for x in size)
print("Scaled image size: " + str(newsize))
scaled_rgb_im = rgb_im.resize(newsize, resample=PIL.Image.LANCZOS)

output_im = Image.new(mode="RGB", size=(160, 120))

# we split lines 81, 82 and 83 and store the data behind line 154, 155, 156, 157, 158 and 159
lineHalves = []
for y in range(0, newsize[1]):
  lineHalves.append(["", ""])
  for x in range(0, newsize[0]):
    r, g, b = scaled_rgb_im.getpixel((x, y))
    value = (int(round(r / 85))) + (int(round(g / 85)) << 2) + (int(round(b / 85)) << 4) 
    output_im.putpixel((x, y), ((int(round(r / 85))) * 85, (int(round(g / 85))) * 85, (int(round(b / 85))) * 85))
    if(x < 80):
      lineHalves[y][0] += hex(value) + " "
    else:
      lineHalves[y][1] += hex(value) + " "



gasm = open(sys.argv[2], "w")
gasm.write("""_callTable_ EQU     0x007E      ; call addresses are automatically stored here by the assembler, it grows downwards,
                                        ; make sure it doesn't crash into anything important

           CALL       copy_lines
loop       BRA loop
""");
height = len(lineHalves)

for y in range(0, height):
  if(y < 81 or y > 83):
    gasm.write("LINE" + str(y) + "      EQU    " + hex(0x0800 + (y*256)) + "\n")
    gasm.write("LINE" + str(y) + "      DB " + lineHalves[y][0] + lineHalves[y][1])
  if(y == height - 7):
    gasm.write("\n" + copyLinesCode1)
    gasm.write("           LDWI    " + hex(0x08a0 + ((height-6)*256)) + "\n")
    gasm.write(copyLinesCode2)
  if(y == height - 6):
    gasm.write(lineHalves[81][0])
  if(y == height - 5):
    gasm.write(lineHalves[81][1])
  if(y == height - 4):
    gasm.write(lineHalves[82][0])
  if(y == height - 3):
    gasm.write(lineHalves[82][1])
  if(y == height - 2):
    gasm.write(lineHalves[83][0])
  if(y == height - 1):
    gasm.write(lineHalves[83][1])
  gasm.write("\n")

scaled_output_im = output_im.resize((640, 480), resample=PIL.Image.BOX)
scaled_output_im.save(sys.argv[1] + ".gigatronified.jpg", format="JPEG")
