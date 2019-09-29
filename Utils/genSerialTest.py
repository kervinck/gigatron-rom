# Generate test GT1 file
#
# Sends every pixel value to the screen, each time followed
# by the digits of the pixel value just sent. In case of
# complete success 4 lines of 64 identical colors are drawn,
# and the number 255 shown last

digits = [
[ 0b111, #'0'
  0b101,
  0b101,
  0b101,
  0b111 ],
[ 0b001, #'1'
  0b001,
  0b001,
  0b001,
  0b001 ],
[ 0b111, #'2'
  0b001,
  0b111,
  0b100,
  0b111 ],
[ 0b111, #'3'
  0b001,
  0b111,
  0b001,
  0b111 ],
[ 0b101, #'4'
  0b101,
  0b111,
  0b001,
  0b001 ],
[ 0b111, #'5'
  0b100,
  0b111,
  0b001,
  0b111 ],
[ 0b111, #'6'
  0b100,
  0b111,
  0b101,
  0b111 ],
[ 0b111, #'7'
  0b001,
  0b001,
  0b001,
  0b001 ],
[ 0b111, #'8'
  0b101,
  0b111,
  0b101,
  0b111 ],
[ 0b111, #'9'
  0b101,
  0b111,
  0b001,
  0b111 ],
]

fp = open('ByteTest.gt1', 'wb')

def out(*args):
  for arg in args:
    fp.write(chr(arg))

for b in range(256):
  out(8+b//64,b%64,1,b) # Byte as a pixel
  for y in range(5):    # Five grapical lines
    out(8+y,64,12)      # Segment header
    r, n = 100, b       # Extract digits
    while r > 0:
      d, n, r = n // r, n % r, r // 10
      px = digits[d][y]
      for x in reversed(range(4)):
        out(63 if px & (1<<x) else 32)
out(0,0,0)

fp.close()

