#!/usr/bin/env python3
# Take raw byte data and wrap it in GT1 format
import sys
#address = sys.stdin.buffer.read(2)
#address = address[0]+ address[1]*256
address = 0x212
raw = sys.stdin.buffer.read()
while len(raw) > 0:
  nextPage = (address + 256) & ~255
  dataSize = min(nextPage - address, len(raw))
  segmentData, raw = raw[0:dataSize], raw[dataSize:]
  sys.stdout.buffer.write(bytes([address>>8, address&255, dataSize&255]))
  sys.stdout.buffer.write(segmentData)
  address = nextPage
sys.stdout.buffer.write(bytes([0,0,0]))
