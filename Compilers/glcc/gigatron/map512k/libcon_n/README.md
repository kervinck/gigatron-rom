# LIBCON_N - Narrow console library

This library overrides the default GLCC console to use double horizontal resolution
with video buffers located in banks 12 and 14. This mode provides a video screen
of 320x120 pixels able to display 15 lines of 52 characters.

Doubling the vertical resolution is in the plans but will require a ROM patch.
