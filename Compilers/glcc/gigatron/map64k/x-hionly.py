
# This overlay uses only the high memory 0x8000-0xffff except for a
# small starting stub in 0x200-0x23f that checks that there is at
# least 64KB of RAM.  It also avoids loading anything in 0x8200-0x823f
# in order to keep the stub intact on a 32KB machine.

segments = [ (0x79c0, 0x8240, None,   None,   0),   # main hi memory
	     (0x0100, 0x8100, None,   None,   0),   # extra page
             (0x0040, 0x0200, None,   None,   7) ]  # ram check stub at location 0x200


