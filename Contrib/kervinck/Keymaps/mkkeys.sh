#!/bin/bash

./remap  |
sort -n -k 8 -k6 -k 4 -k 2 |
awk '
function emit()
{
  if (ascii == 0)
    return;

  a[j++] = map FS mods FS code FS ascii
  if (j == 3 && a[0] ~ /^UGDFIE/ && a[1] ~ /^UGDFIE/) {
    map = "UGDFIE"
    mods = 3
    j = 0
    a[j++] = map FS mods FS code FS ascii
  }

  if ($8 != ascii || $6 != code) {
    for (i=0; i<j; i++)
      print a[i]
    j = 0
  }
}
BEGIN {
  L[0] = "U"
  L[1] = "G"
  L[2] = "D"
  L[3] = "F"
  L[4] = "I"
  L[5] = "E"
  emit()
}
{
  if ($8 != ascii || $6 != code || $4 != mods) {
    emit()
    map = "------"
    mods = $4
    code = $6
    ascii = $8
  }
  map = substr(map, 1, $2) L[$2] substr(map, $2+2, length(map)-$2)
}
END {
  $8 = 0
  emit()
}' |

awk '{
  map = ""
  if ($1~/U/) map = map "+US"; else map = map "   "
  if ($1~/G/) map = map "+GB"; else map = map "   "
  if ($1~/D/) map = map "+DE"; else map = map "   "
  if ($1~/F/) map = map "+FR"; else map = map "   "
  if ($1~/I/) map = map "+IT"; else map = map "   "
  if ($1~/E/) map = map "+ES"; else map = map "   "
  if ($2 == 0) mod = "NOMOD"
  if ($2 == 1) mod = "SHIFT"
  if ($2 == 2) mod = "ALTGR"
  if ($2 == 3) mod = "EVERY"
  printf "  { %-18s +%s, 0x%02x, %3d },", map, mod, $3, $4
  if (32 < $4 && $4 <= 126 && $4 != last)
    printf " // '"'"'%c'"'"'", $4
  last = $4
  printf "\n"
}' > keys.txt

echo "Sanity check:"
for C in +US +GB +DE +FR +IT +ES
do
  echo -n "$C:"
  grep $C keys.txt | awk -F, '{print $3}' | sort -u | wc -l
done

