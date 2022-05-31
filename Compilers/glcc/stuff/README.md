# Additional stuff

Contents:

  * The `doc` subdirectory contains a late draft of the ANSI C 1989 spec that glcc targets.
  
  * The `horizon` subdirectory contains the historical gigatron horizon program and a makefile to compile it for rom v4, v5a, or for at67x's experimental rom. You either need `glcc` in the path, or need to provide argument `CC=path-to-glcc` to the `make` command.
  
  * The `fp` subdirectory contains code to exercise the floating point emulation and show how costly it is to call the function `printf`.  You either need `glcc` in the path, or need to provide argument `CC=path-to-glcc` to the `make` command. You can also provide `ROM=xxx` or `MAP=xxx` arguments to experiement.

  * The `tst` subdirectory contains small programs to test the console emulation, test the standard io library on the console, and test the function `_memcpyext` that copies bytes across memory banks when a RAM and I/O expansion board is present.
