Utilities for Gigatron development
==================================

Files
=====
```
gcl.vim                         VIM syntax highlighting file
raw24toraw6.py                  Raw 24bpp to GCL-format packed 6bpp converter
sendGt1.py                      Send a GT1 file from laptop/PC to Gigatron through
LoaderTest/LoaderTest.ino       Sketch to install on Arduino for Gigatron interfacing
```

Installing VIM syntax highlighting
==================================

Place `gcl.vim` in `$HOME/.vim/after/syntax/` and create a file,
`$HOME/.vim/ftdetect/gcl.vim` with the contents:

```
au BufRead,BufNewFile *.gcl set filetype=gcl
```

Using raw24toraw6.py
====================

Running the file will output usage instructions. When executed successfully,
two files will be produced, a `.raw` file and a `.gcl` file. Both files contain
packed 6bpp image data. The `.gcl` file is formatted as valid GCL code and can
be copy-pasted to use directly in GCL code. The data can then be drawn to the
screen with `\SYS_Unpack_56` and `\SYS_Draw4_30`.
