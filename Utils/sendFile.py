#!/usr/bin/env python3
#-----------------------------------------------------------------------
#
#  sendFile.py -- Send file to Gigatron through Arduino as interface
#
#-----------------------------------------------------------------------
#
# Examples:
#
#   Utils/sendFile.py Apps/Overworld/Overworld.gt1 # Reset and starts Loader
#   Utils/sendFile.py BASIC/FastLines.gtb          # Also loads BASIC
#   Utils/sendFile.py foo.bas                      # Only send text lines
#   Utils/sendFile.py < foo.txt                    # Same...
#
# Notes:
#
#  - Needs Python 3.x: Python 2.7 isn't supported any longer
#  - Systems: macOS, Linux and Windows
#  - Devices: Arduino Uno/Micro/Nano, SparkFun Pro Micro
#  - Arduino (-compatible) must run BabelFish.ino
#
# History:
#
# 2018-04-29 (marcelk) Initial version
# 2018-05-13 (marcelk) Use application level flow control
# 2018-05-29 (rockybulwinkle) Add option to override detected port,
#               search on /dev/ttyACM* as well, swith to argparse.
# 2018-05-30 (marcelk) Enable stdin again. A bit more consistency.
# 2018-06-04 (marcelk) Allow timeout while waiting for first prompt
# 2018-06-29 (marcelk) Send escaped control bytes to newer Babelfishes
# 2018-07-23 (marcelk) Send GTB files into BASIC. Renamed to sendFile.py
# 2018-07-28 (marcelk) Remove escaping of control bytes. Workaround FIFOCON bug.
# 2019-12-19 (crouze) Initial port from senFile.py
#                     BASIC truncation result added to stdout
# 2019-12-22 (marcelk) Adopt crouze's sendFile3.py mods; Remove legacy check;
#                      Simplifications; Check COM ports on Windows
#
#-----------------------------------------------------------------------

from __future__ import print_function
import argparse
import serial # pip3 install pyserial
from serial.tools.list_ports import comports
import sys
from time import sleep

# One-for-all error handler (don't throw scary stack traces at the user)
sys.excepthook = lambda exType, exValue, exTrace: print('%s: %s' % (exType.__name__,  exValue))

if sys.version_info[0] < 3:     # Require Python 3
  raise Exception('Need Python 3')

#-----------------------------------------------------------------------
#       Command line arguments
#-----------------------------------------------------------------------

parser = argparse.ArgumentParser(description='Send a file into the Gigatron')
parser.add_argument('-v', '--verbose', dest='verbose', default=False,
                    help='be verbose',
                    action='store_true')
parser.add_argument('-p', '--port', dest='port', default=None,
                    help='USB port to Arduino (default is auto-detect)')
parser.add_argument('-t', '--truncate', dest='truncate', default=250,
                    help='Truncate text lines')
parser.add_argument('filename', help='GT1 or GTB file', nargs='?')

args = parser.parse_args()

#-----------------------------------------------------------------------
#       Functions
#-----------------------------------------------------------------------

def log(prefix, line):
  if args.verbose:
    print(prefix, line)

def sendCommand(cmd):
  """Send command to Gigatron Adapter Interface and wait for ready prompt"""
  log('>', cmd)
  ser.write(str.encode(cmd + '\n'))
  return waitReply()

def waitReply(expectPrompt=True):
  line = readLine()
  while True:
    if line is None:
      if not expectPrompt:
        break
    else:
      if line[-1] == '?':       # Got prompt
        break
      if line[0] == '!':        # Got error
        raise Exception(line[1:])
    line = readLine()
  return line

def readLine():
  """Read line from serial port with optional verbosity"""
  line = ser.readline()
  if line:
    line = line.decode()        # Convert from bytes to Unicode
    line = line.rstrip()        # Remove any trailing newline and spaces
    log('<', line)
  else:
    line = None                 # Signal that there is a timeout
    if args.verbose:
      sys.stdout.write('.')
      sys.stdout.flush()
  return line

def sendGt1(fp):
  """Send Gigatron object file"""
  ask = sendCommand('U')

  while ask[0].isdigit():
    # Arduino will ask for <n> bytes by sending '<n>?'
    n = int(ask[:-1])
    data = fp.read(n)
    if len(data) < n:
      raise Exception('File too short')
    sys.stdout.write('.')
    sys.stdout.flush()
    ser.write(data)
    ask = waitReply()           # Wait for next prompt
  print()

#-----------------------------------------------------------------------
#       Connect to Arduino
#-----------------------------------------------------------------------

autoDetect = [                  # (VID, PID)
        (0x2341, 0x0043),       # Arduino Uno
        (0x2341, 0x8037),       # Arduino Micro
        (0x0403, 0x6001),       # Arduino Nano
        (0x1b4f, 0x9206) ]      # SparkFun Pro Micro (5V, 16 MHz)

port = None
if not args.port:
  serPorts = [p for p in comports() if (p.vid, p.pid) in autoDetect]
  if len(serPorts) == 0:
    raise Exception('No Arduino detected')
  if len(serPorts) > 1:
    raise Exception('More than one Arduino detected: %s' % ' '.join(serPorts))
  port = serPorts[0].device
  desc = ' (%s)' % serPorts[0].description
else:
  port, desc = args.port, ''

print('Connecting to %s%s' % (port, desc))
ser = serial.Serial(port=port, baudrate=115200, timeout=2)

#-----------------------------------------------------------------------
#       Send program
#-----------------------------------------------------------------------

if args.filename:
  isGt1 = args.filename.lower().endswith(('.gt1', '.gt1x'))
  isGtb = args.filename.lower().endswith(('.gtb', '.gtbx'))
  fp = open(args.filename, 'rb' if isGt1 else 'r')
else:
  isGt1, isGtb = False, False
  fp = sys.stdin

waitReply(expectPrompt=False)   # Check for prompt, but don't wait forever

if isGt1 or isGtb:
  print('Resetting Gigatron')
  sendCommand('R')

  print('Starting Loader')
  sendCommand('L')

if isGt1:
  print('Sending program %s' % repr(args.filename))
  sendGt1(fp)

if isGtb:
  print('Loading Tiny BASIC')
  sendCommand('P')              # Load BASIC from BabelFish PROGMEM
  sleep(2)                      # Startup takes a while on 64K
  args.truncate = 25            # Line limit

if not isGt1:
  print('Sending text %s' % (repr(args.filename) if args.filename else 'from stdin'))
  for line in fp:
    line = line.rstrip('\r\n')
    if len(line) > args.truncate:
      print('Truncated %s' % repr(line))
      line = line[0:args.truncate]
      print('       to %s' % repr(line))
    sendCommand('.' + line)

if isGtb:
  sendCommand('.RUN')           # Start BASIC program

print('Finished')

if args.filename:
  fp.close()

#-----------------------------------------------------------------------
#
#-----------------------------------------------------------------------

