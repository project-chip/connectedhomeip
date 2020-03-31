#!/usr/bin/python

import re
import sys
import urllib

def debugpprint(stuff):
  if False:
    print stuff

def cbor(json):
  url = 'http://www.cbor.me?%s' % (urllib.urlencode({'diag': json}))
  debugpprint('cbor.me request "%s"' % (url))
  response = urllib.urlopen(url).read()
  debugpprint('cbor.me response "%s"' % (response))
  return reallycbor(response)

def reallycbor(response):
  # A poor man's HTML DOM parser.
  starttext = '<textarea id="bytes" name="bytes" placeholder="Bytes in hex: 82 01 82 02 03">'
  start = response.find(starttext)
  assert start != -1
  start += len(starttext)
  end = response.find('\n</textarea>', start)
  assert end != -1
  response = response[start:end]
  response = re.sub(r'#.*', '', response)
  response = re.sub(r'\s+', '', response)
  return response.upper()

def json(cbor):
  url = 'http://www.cbor.me?%s' % (urllib.urlencode({'bytes': cbor}))
  debugpprint('cbor.me request "%s"' % (url))
  response = urllib.urlopen(url).read()
  debugpprint('cbor.me response "%s"' % (response))
  return reallyjson(response)

def reallyjson(response):
  # A poor man's HTML DOM parser.
  starttext = '<textarea autofocus="autofocus" id="diag" name="diag" placeholder="Diagnostic notation: [1, [2, 3]]">'
  start = response.find(starttext)
  assert start != -1
  start += len(starttext)
  end = response.find('</textarea>', start)
  assert end != -1
  return response[start:end]

if __name__ == '__main__':
  p = "usage: cbor.py <cbor-or-json>"
  if len(sys.argv) == 2:
    if re.compile("^[A-Fa-f0-9]+$").match(sys.argv[1]) is not None:
      p = json(sys.argv[1])
    else:
      p = cbor(sys.argv[1])
  print p
