
# Map overlay that allows using all the available memory
# Note that we do not use segments greater
# than 32k because this breaks malloc.

segments = [(0x7e00, 0x0200, None,   None,   0),
            (0x7c00, 0x8000, None,   None,   0) ]

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
