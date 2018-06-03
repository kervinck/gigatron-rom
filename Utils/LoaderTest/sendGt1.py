#!/usr/bin/env python
#-----------------------------------------------------------------------
#
# sendGt1.py -- Send GT1 file to Gigatron through Arduino as interface
#
#  XXX So far tested for macOS only
#  XXX Still uses Python 2.7 (default on macOS). Should be reworked to
#      be independent of Python 2 or 3.
#
# 2018-04-29 (marcelk) Initial version
# 2018-05-13 (marcelk) Use application level flow control
# 2018-05-29 (rockybulwinkle) Add option to override detected port,
#		search on /dev/ttyACM* as well, swith to argparse.
# 2018-05-30 (marcelk) Enable stdin again. A bit more consistency.
# 2018-06-04 (marcelk) Allow timeout while waiting for first prompt
#
#-----------------------------------------------------------------------

import argparse
import glob
import serial # pySerial: http://pyserial.readthedocs.io/en/latest/index.html
import sys
from time import sleep

#-----------------------------------------------------------------------
#       Command line arguments
#-----------------------------------------------------------------------

parser = argparse.ArgumentParser(description='Send a GT1 into the Gigatron')
parser.add_argument('-v', '--verbose', dest='verbose',
                    help='be verbose',
                    action='store_true', default=False)
parser.add_argument('-p', '--port', dest='port',
                    help='USB port to Arduino (default is auto-detect)',
                    default=None)
parser.add_argument('filename', help='GT1 file', nargs='?')

args = parser.parse_args()

#-----------------------------------------------------------------------
#       Functions
#-----------------------------------------------------------------------

def log(prefix, line):
  if args.verbose:
    print prefix, line

def sendCommand(cmd=None, timeout=False):
  """Send command to Gigatron Adapter Interface and wait for ready prompt"""
  if cmd:
    log('>', cmd)
    ser.write(cmd + '\n')

  # Wait for prompt
  line = readLine()
  while not (line is None and timeout):
    if line:
      if line[-1] == '?': # Got prompt
        break
      if line.startswith('!'): # Got error
        print 'Failed:', line[1:]
        exit(1)
    line = readLine()
  return line

def readLine():
  """Read line from serial port with optional verbosity"""
  line = ser.readline()
  if line:
    line = line.rstrip() # Remove any trailing newline and spaces
    log('<', line)
  else:
    line = None # Signal that there is a timeout
    if args.verbose:
      sys.stdout.write('.')
      sys.stdout.flush()
  return line

#-----------------------------------------------------------------------
#       Connect to Arduino
#-----------------------------------------------------------------------

port = None
if not args.port:
  # Sometimes the Arduino shows up on /dev/tty.usbmodem* or /dev/ttyACM*
  serPorts = glob.glob('/dev/tty.usbmodem*') + glob.glob('/dev/ttyACM*')
  if len(serPorts) == 0:
    print 'Failed: No USB device detected'
    exit(1)
  if len(serPorts) > 1:
    print 'Failed: More than one USB device detected: %s' % ' '.join(serPorts)
    exit(1)
  port = serPorts[0]
else:
  port = args.port

print 'Connecting to', port
try:
  ser = serial.Serial(port=port, baudrate=115200, timeout=2)
except Exception, e:
  print 'Failed: ' + str(e)
  exit(1)

#-----------------------------------------------------------------------
#       Send program
#-----------------------------------------------------------------------

if args.filename:
  try:
    fp = open(args.filename, 'rb')
  except Exception, e:
    print 'Failed: ' + str(e)
    exit(1)
else:
  fp = sys.stdin

sendCommand(timeout=True) # Wait for prompt, but not for too long

print 'Resetting Gigatron'
sendCommand('R')

print 'Starting Loader'
sendCommand('L')

print 'Sending program %s' % (repr(args.filename) if args.filename else 'from stdin')
ask = sendCommand('U')

while ask[0].isdigit():
  # Arduino will ask for <n> bytes by sending '<n>?'
  n = int(ask[:-1])
  data = fp.read(n)
  if len(data) < n:
    print 'Failed: File too short'
    exit(1)
  sys.stdout.write('.')
  sys.stdout.flush()
  ser.write(data)
  ask = sendCommand(None)
print

print 'Finished'

if args.filename:
  fp.close()

#-----------------------------------------------------------------------
#
#-----------------------------------------------------------------------

