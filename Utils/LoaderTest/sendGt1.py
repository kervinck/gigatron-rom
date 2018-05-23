#!/usr/bin/env python
#-----------------------------------------------------------------------
#
# sendGt1.py -- Send GT1 file to Gigatron through Arduino as interface
#
#   *** So far tested for MacOSX only ***
#
# 2018-04-29 (marcelk) Initial version
# 2018-05-13 (marcelk) Use application level flow control
#
#-----------------------------------------------------------------------

import glob
import serial
import sys
from time import sleep

#-----------------------------------------------------------------------
#       Command line arguments
#-----------------------------------------------------------------------

verbose = False
filename = None

argi = 1
if len(sys.argv) > argi and sys.argv[argi] == '-v':
  verbose = True
  argi += 1

if len(sys.argv) - argi == 1:
  filename = sys.argv[argi]
  argi += 1

if argi < len(sys.argv):
  print 'Error: Bad arguments'
  print 'Usage: python sendGt1.py [-v] [filename]'
  sys.exit(1)

def log(prefix, line):
  if verbose:
    print prefix, line

#-----------------------------------------------------------------------
#       Functions
#-----------------------------------------------------------------------

def sendCommand(cmd=None):
  """Send command to Gigatron Adapter Interface and wait for ready prompt"""
  if cmd:
    log('>', cmd)
    ser.write(cmd + '\n')

  # Wait for prompt (line ending with a question mark)
  line = ser.readline().rstrip()
  log('<', line)
  while len(line) == 0 or line[-1] != '?':
    if line.startswith('!'):
      print 'Failed:', line[1:]
      sys.exit(1)
    line = ser.readline().rstrip()
    log('<', line)
  return line

#-----------------------------------------------------------------------
#       Connect to Arduino
#-----------------------------------------------------------------------

serPorts = glob.glob('/dev/tty.usbmodem*')
if len(serPorts) == 0:
  print 'Failed: No USB device detected'
  sys.exit(1)
if len(serPorts) > 1:
  print 'Failed: More than one USB device detected: %s' % ' '.join(serPorts)
  sys.exit(1)
print 'Connecting to', serPorts[0]
ser = serial.Serial(port=serPorts[0], baudrate=115200)
sleep(2)

#-----------------------------------------------------------------------
#       Send program
#-----------------------------------------------------------------------

if filename:
  fp = open(filename, 'rb')
else:
  fp = sys.stdin

sendCommand() # Wait for prompt

print 'Reseting Gigatron'
sendCommand('R')

print 'Starting Loader'
sendCommand('L')

print 'Sending program %s' % (repr(filename) if filename else 'from stdin')
ask = sendCommand('U')

while ask[0].isdigit():
  # Arduino will ask for <n> bytes by sending '<n>?'
  n = int(ask[:-1])
  data = fp.read(n)
  if len(data) < n:
    print 'Failed: File too short'
    sys.exit(1)
  sys.stdout.write('.')
  sys.stdout.flush()
  ser.write(data)
  ask = sendCommand(None)
print

print 'Finished'

if filename:
  fp.close()

#-----------------------------------------------------------------------
#
#-----------------------------------------------------------------------

