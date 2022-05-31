# This overlays eats the memory normally allocated to channels 2,3,4
# in order to provide a contiguous memory block from 0x200 to 0x5ff.

segments = [ (0x0060, 0x08a0, 0x0100, 0x80a0, 0),
	     (0x0400, 0x0200, None,   None,   1) ]
