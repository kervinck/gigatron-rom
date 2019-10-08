#!/bin/bash
#-----------------------------------------------------------------------
#
#       plotGt1Maps.sh -- Create heatmaps from GT1 files
#
#-----------------------------------------------------------------------
#
# 2019-10-08 (marcelk) First version
#

find . -name \*.gt1 |
 xargs -n 1 gt1dump.py |
  cut -c -54 |
   awk '
   function copy() {
     for (addr in f)
      n[addr]++
     delete f
   }
   /\* file:/ {
     if (length(f)) copy()
   }
   /^[0-9a-f]/ {
    addr=int("0x"$1)
    for (i=0;i<NF-1;i++)
     f[addr+i]++
   }
   END {
    copy()
    for (y=0;y<256;y++) {
     for (x=0;x<256;x++) {
      addr = y*256+x
      if (addr in n)
        z=n[addr]
      else
        z=0
      print int(addr/256), addr%256, z
     }
     print""
    }
   }' > segments.dat

gnuplot <<!
 set title 'Gigatron file segment heatmap (32K)'
 set nokey
 set noborder
 set xrange [-1:256]
 set yrange [128:-1]
 set format x2 'xx%02X'
 set format y  '%02Xxx'
 set noxtics
 set x2tics out 0,16,255
 set x2tics add (255)
 set ytics  out 0,8,127
 set ytics  add (127)
 set palette defined (\
        0    .7 .7 .7,\
        .001 0  0  1, \
        2    0  .5 1, \
        3    0  1  1, \
        4    .5 1  .5,\
        5    1  1  0, \
        6    1  .5 0, \
        7    1  0  0, \
        8    1  0  0)
 set cblabel 'Number of GT1 files'
 set terminal png size 1280,960
 set output 'Heatmap_32K.png'
 set cbrange [0.7:]
 set logscale cb
 set cbtics out 1,2
 set nomcbtics
 # Invalidate pages >= 128
 plot 'segments.dat' using 2:1:(\$1<128?\$3:1/0) with image

 # 64K view
 set title 'Gigatron file segment heatmap (64K)'
 set yrange [256:-2]
 set ytics  out 0,16,255
 set ytics  add (255)
 set output 'Heatmap_64K.png'
 plot 'segments.dat' using 2:1:3 with image
!

open Heatmap_*K.png
