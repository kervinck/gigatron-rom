#!/usr/bin/env python
#-----------------------------------------------------------------------
#
# sendFile3.py -- Send file to Gigatron through Arduino as interface
#                 for Python 3.x, ported from sendFile.py
#
# Tested with Python 3.8.0 on Linux (Arch Linux x86_64)
# Not tested on other platforms or other Python 3.x releases
# NOT backwards compatible with Python 2.x
#
# From sendFile.py (Python 2.x):
# 2018-04-29 (marcelk) Initial version
# 2018-05-13 (marcelk) Use application level flow control
# 2018-05-29 (rockybulwinkle) Add option to override detected port,
#            search on /dev/ttyACM* as well, swith to argparse.
# 2018-05-30 (marcelk) Enable stdin again. A bit more consistency.
# 2018-06-04 (marcelk) Allow timeout while waiting for first prompt
# 2018-06-29 (marcelk) Send escaped control bytes to newer Babelfishes
# 2018-07-23 (marcelk) Send GTB files into BASIC. Renamed to sendFile.py
# 2018-07-28 (marcelk) Remove escaping of control bytes. Workaround FIFOCON bug.
#
# sendFile3.py:
# 2019-12-19 (crouze) Initial port from senFile.py
#                     BASIC truncation result added to stdout
#
#-----------------------------------------------------------------------

from __future__ import print_function
import argparse
import glob
import serial # pySerial: http://pyserial.readthedocs.io/en/latest/index.html
import sys
from time import sleep

# One-for-all error handler (don't throw scary stack traces at the user)
sys.excepthook = lambda exType, exValue, exTrace: print('%s: %s' % (exType.__name__,  exValue))

#-----------------------------------------------------------------------
#       Command line arguments
#-----------------------------------------------------------------------

parser = argparse.ArgumentParser(description='Send a file into the Gigatron')
parser.add_argument('-v', '--verbose', dest='verbose',
                    help='be verbose',
                    action='store_true', default=False)
parser.add_argument('-p', '--port', dest='port',
                    help='USB port to Arduino (default is auto-detect)',
                    default=None)
parser.add_argument('filename', help='GT1 or GTB file', nargs='?')

args = parser.parse_args()

#-----------------------------------------------------------------------
#       Functions
#-----------------------------------------------------------------------

def log(prefix, line):
  if args.verbose:
    print(prefix, line)

def sendCommand(cmd=None, timeout=False):
  """Send command to Gigatron Adapter Interface and wait for ready prompt"""
  if cmd:
    log('>', cmd)
    ser.write(str.encode(cmd + '\n'))

  # Wait for prompt
  line = readLine()
  while not (line is None and timeout):
    if not line is None:
      line = line.decode()
    if line:
      if line[-1] == '?': # Got prompt
        break
      if line.startswith('!'): # Got error
        raise Exception(line[1:])
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

def sendGt1(fp):
  """Send Gigatron object file"""
  ask = sendCommand('U')

  while ask[0].isdigit():
    # Arduino will ask for <n> bytes by sending '<n>?'
    if ask[0] == '0': # Keep this check for a transition period until end 2018
      raise Exception('Please update BabelFish.ino sketch on Arduino')
    n = int(ask[:-1])
    data = fp.read(n)
    if len(data) < n:
      raise Exception('File too short')
    sys.stdout.write('.')
    sys.stdout.flush()
    ser.write(data)
    ask = sendCommand(None)
  print()

#-----------------------------------------------------------------------
#       Connect to Arduino
#-----------------------------------------------------------------------

port = None
if not args.port:
  # Sometimes the Arduino shows up on /dev/tty.usbmodem* or /dev/ttyACM*
  serPorts = glob.glob('/dev/tty.usbmodem*') + glob.glob('/dev/ttyACM*')
  if len(serPorts) == 0:
    raise Exception('No USB device detected')
  if len(serPorts) > 1:
    raise Exception('More than one USB device detected: %s' % ' '.join(serPorts))
  port = serPorts[0]
else:
  port = args.port

print('Connecting to', port)
ser = serial.Serial(port=port, baudrate=115200, timeout=2)

#-----------------------------------------------------------------------
#       Send program
#-----------------------------------------------------------------------

if args.filename:
  fp = open(args.filename, 'rb')
else:
  fp = sys.stdin

sendCommand(timeout=True) # Wait for prompt, but not for too long

print('Resetting Gigatron')
sendCommand('R')

print('Starting Loader')
sendCommand('L')

if args.filename and args.filename.lower().endswith(('.gt1', '.gt1x')):
  print('Sending program %s' % repr(args.filename))
  sendGt1(fp)
else:
  isBasic = args.filename and args.filename.lower().endswith(('.gtb', 'gtbx', '.bas'))
  if isBasic:
    print('Loading BASIC')
    sendCommand('P')
    sleep(2) # BASIC takes a while on 64K
  print('Sending text %s' % (repr(args.filename) if args.filename else 'from stdin'))
  for line in fp:
    line = line.strip().decode()
    if len(line) > 25:
      print("Truncated %s" % repr(line))
      line = line[0:25]
      print("       to %s" % repr(line))
    sendCommand('.' + line)
  if isBasic:
    sendCommand('.RUN')

print('Finished')

if args.filename:
  fp.close()

#-----------------------------------------------------------------------
#
#-----------------------------------------------------------------------
