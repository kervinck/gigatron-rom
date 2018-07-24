#!/usr/bin/env python
#-----------------------------------------------------------------------
#
# sendFile.py -- Send file to Gigatron through Arduino as interface
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
# 2018-06-29 (marcelk) Send escaped control bytes to newer Babelfishes
# 2018-07-23 (marcelk) Send GTB files into BASIC. Renamed to sendFile.py
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
    ser.write(cmd + '\n')

  # Wait for prompt
  line = readLine()
  while not (line is None and timeout):
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

  def escape(x):
    return '\x7d' + chr(ord(x) ^ 0x20)
  special = '' # Backwards compatibility with older Babelfishes

  while ask[0].isdigit():
    # Arduino will ask for <n> bytes by sending '<n>?'
    if ask[0] == '0': # Trigger to enable escaping of XON/OFF
      special = '\x11\x13\x7d'
    n = int(ask[:-1])
    data = fp.read(n)
    if len(data) < n:
      raise Exception('File too short')
    sys.stdout.write('.')
    sys.stdout.flush()
    data = ''.join([escape(x) if x in special else x for x in data])
    if '\x13' in data: # Just a warning, and just for XOFF
      print('\nWarning: sending unescaped XOFF')
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
    print('Starting BASIC')
    sendCommand('P1')
    sleep(2) # BASIC takes a while on 64K
  print('Sending text %s' % (repr(args.filename) if args.filename else 'from stdin'))
  for line in fp:
    sendCommand('.' + line.strip())
  if isBasic:
    sendCommand('.RUN')

print('Finished')

if args.filename:
  fp.close()

#-----------------------------------------------------------------------
#
#-----------------------------------------------------------------------

