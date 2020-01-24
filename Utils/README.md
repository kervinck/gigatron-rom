Utilities for Gigatron development
==================================

Files
=====
```
BabelFish/BabelFish.ino         Sketch to install on Arduino for Gigatron hookup
gcl.vim                         VIM syntax highlighting file
raw24toraw6.py                  Raw 24bpp to GCL-format packed 6bpp converter
sendFile.py                     Send a GT1 or BASIC file from laptop/PC to Gigatron
SerialTest.gt1                  Test file for verifying serial transmission
gt1dump.py                      Dump GT1 file to show internal structure
```

Installing VIM syntax highlighting
==================================

Place `gcl.vim` in `$HOME/.vim/after/syntax/` and create a file,
`$HOME/.vim/ftdetect/gcl.vim` with the contents:

```
au BufRead,BufNewFile *.gcl set filetype=gcl
```

Installing Gedit/Pluma/Xed syntax highlighting
==============================================

Place `gcl.lang` in `$HOME/.local/share/gtksourceview-3.0/language-specs`.

Using raw24toraw6.py
====================

Running the file will output usage instructions. When executed successfully,
two files will be produced, a `.raw` file and a `.gcl` file. Both files contain
packed 6bpp image data. The `.gcl` file is formatted as valid GCL code and can
be copy-pasted to use directly in GCL code. The data can then be drawn to the
screen with `\SYS_Unpack_56` and `\SYS_Draw4_30`.

Using sendFile.py
=================

```
usage: sendFile.py [-h] [-v] [-p PORT] [filename]

Send a file into the Gigatron

positional arguments:
  filename              GT1 or GTB file

optional arguments:
  -h, --help            show this help message and exit
  -v, --verbose         be verbose
  -p PORT, --port PORT  USB port to Arduino (default is auto-detect)
```

Using gt1file.py
================

```
usage: gt1dump.py [-h] [-d] [-f] [filename]

Dump GT1 file

positional arguments:
  filename           GT1 file

optional arguments:
  -h, --help         show this help message and exit
  -d, --disassemble  disassmble as vCPU code
  -f, --force        accept any filename extension
```
