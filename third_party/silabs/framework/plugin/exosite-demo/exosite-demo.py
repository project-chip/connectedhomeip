#!/usr/bin/env python

## python_helloworld.py
## Tested with python 2.6.5
##
## Copyright (c) 2011, Exosite LLC
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without 
## modification, are permitted provided that the following conditions are met:
##
##    * Redistributions of source code must retain the above copyright notice,
##      this list of conditions and the following disclaimer.
##    * Redistributions in binary form must reproduce the above copyright 
##      notice, this list of conditions and the following disclaimer in the
##      documentation and/or other materials provided with the distribution.
##    * Neither the name of Exosite LLC nor the names of its contributors may
##      be used to endorse or promote products derived from this software 
##      without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.
""" 
  Exosite 'Hello World' modified for use with the Silicon Labs App Framework v2.
"""

""" 
  Directions:
  1) Have an Exosite Portals account
  2) Add a new Device client to your portal.  Click on it to get it's CIK (KEY)
  3) Add the CIK below where it says 'PUTYOURCIKHERE'. (inside of the quotes)
  4) On your portal, add one data source.  This script does not auto-create the data sources.
      Data source should be called "status" with type of 'int'.
  5) Run this python script in Python 2.6.5 or higher.
  6) Assuming your computer has an active network connection, you should see data sent to
      these data sources on your portal.  The script runs only once.  It should also print out
      what it sent, since it is doing a 'read' after the 'write'.
  
"""

import urllib 
import httplib
import argparse
import sys

# PUTYOURCIKHERE
cik = 'b49622db0e10aaeb63f3e283e74b3249c6aa47cc'

server = 'm2.exosite.com'
baseUrl = '/api:v1/stack/alias' 

global DEBUG_ON
DEBUG_ON = 0

def writeLightState(message, on):
  url = baseUrl
  value = 0
  params = urllib.urlencode({'state': on})
  headers = {'X-Exosite-CIK': cik, 'content-type': 'application/x-www-form-urlencoded; charset=utf-8'} 
  if (DEBUG_ON):
    print '=================='
    print 'POST'
    print '=================='
    print 'URL: ', url
    print 'Data: ', params
    print ' '
  conn = httplib.HTTPConnection(server) 
  if (DEBUG_ON):
    conn.set_debuglevel(1)
  conn.request("POST",url,params,headers) 
  response = conn.getresponse(); 
  if DEBUG_ON:
    print 'response: ',response.status,response.reason 
  data = response.read() 
  if DEBUG_ON:
    print '\r\n\r\n'
  conn.close()

def readLightState():
  url = baseUrl + '?state'
  headers = {'Accept':'application/x-www-form-urlencoded; charset=utf-8','X-Exosite-CIK':cik}
  if DEBUG_ON:
    print '=================='
    print 'GET'
    print '=================='  
    print 'URL: ', url
    print ' '

  conn = httplib.HTTPConnection(server)
  conn.request("GET",url,"",headers)
  response = conn.getresponse();
  if DEBUG_ON:
    print 'response: ',response.status,response.reason
  data = response.read()
  conn.close()
  if DEBUG_ON:
    print 'response data:', data
  conn.close()
  return data

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="Read/write the state of the light bulb on Exosite.")
  parser.add_argument('-r', '--read',
    action="store_true",
    help='Read the current state of the light-build from Exosite and print the state (0 or 1)')
  parser.add_argument('-w', '--write',
    type=str,
    help='Write the passed state of the light-bulb to Exosite (0 or 1)')
  parser.add_argument('-d', '--debug',
    action='store_true',
    help='Print additional debug messages.')

  args = parser.parse_args()
  if (args.debug):
    DEBUG_ON = True

  if (args.write and args.read):
    print("Error: Cannot use both --read and --write.")
    sys.exit(-1)

  if (not args.write and not args.read):
    print("Error: Must specify either --read or --write.")
    sys.exit(-1)

  if (args.write):
    if (args.write != "on" and args.write != "off"):
      print("Error:  Write value must be 'on' or 'off'")
      sys.exit(-1)

    writeValue = 0
    if (args.write == "on"):
      writeValue = 1
    writeLightState("On", writeValue)

  else:
    print(readLightState())

