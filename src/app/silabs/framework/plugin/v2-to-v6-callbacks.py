#
# v2-to-v6-callbacks.py
#
# Andrew Keesler
#
# Friday, October 16, 2015
#
# Changing v2 callback.xml files to v6 callbacks.info files.
#

import sys
import os
import subprocess
import xml.sax

class CallbackTranslator(xml.sax.handler.ContentHandler):
  def __init__(self, plugin, only_dump_ids):
    self.indent = ""

    # Make sure tuna-fish becomes TunaFish.
    self.plugin = ""
    upper = True
    for c in plugin:
      if c == "-":
        upper = True
      elif upper:
        self.plugin += c.upper()
        upper = False
      else:
        self.plugin += c

    self.pre  = None
    self.post = None

    self.content = None

    self.rettype     = None
    self.name        = None
    self.description = None
    self.args        = None
    self.code        = None

    self.only_dump_ids = only_dump_ids
    self.eyed          = None

  def dump_id(self, fname):
    macro_plugin = ""
    first = True
    for c in self.plugin:
      if not first and c.isupper():
        macro_plugin += "_"
      macro_plugin += c.upper()
      first = False

    really_id = "EMBER_AF_PLUGIN_%s_%s" % (macro_plugin, self.eyed)
    print "%s = %s" % (really_id, fname)

  def dump(self):
    name = self.pre
    name += "Plugin" + self.plugin + self.name.replace(" ", "")
    name += self.post

    if self.only_dump_ids:
      self.dump_id(name)
      return

    print
    print "/** @brief %s" % (self.name)
    print " *"
    print " *",
    col = len(" *") + 1
    for word in self.description.split():
      col += len(word) + 1
      if col > 80:
        print
        print " *",
        col = len(" *") + 1 + len(word) + 1
      print word,
    print
    print " *"
    for arg in self.args:
      print " * @param",
      col = len(" * @param") + 1
      print arg["name"],
      col += len(arg["name"]) + 1
      if "description" in arg and len(arg["description"].strip()) is not 0:
        for word in arg["description"].split():
          col += len(word) + 1
          if col > 80:
            print
            print " *",
            col = len(" * ") + 1 + len(word) + 1
          print word,
        if col + len(" Ver.: always") > 80:
          print
          print " *",
        print "Ver.: always"
      else:
        print "  Ver.: always"
    print " */"

    function = ""
    function += self.rettype
    function += " "
    function += name
    function += "("
    if len(self.args) == 0:
      function += "void"
    else:
      first = True
      align = len(function)
      for arg in self.args:
        space = "" if "*" in arg["type"] else " "
        if not first:
          function += (",\n" + (" " * align))
        function += "%s%s%s" % (arg["type"], space, arg["name"])
        first = False
    function += ")"
    function += self.code
    print function

  def startDocument(self):
    if self.only_dump_ids:
      print
      print "# Plugin: " + self.plugin

  def startElement(self, tag, attrs):
    self.indent += "  "
    debug(self.indent + ("startElement(%s, attrs=%s)" % (tag, dict(attrs))))
    
    self.content = ""
    
    if tag == "callback":
      self.pre  = attrs["prefix"]
      self.post = attrs["postfix"]
    elif tag == "include":
      if not self.only_dump_ids:
        print "#include ",
    elif tag == "function":
      self.description = ""
      self.rettype  = attrs["returnType"]
      self.name = attrs["name"]
      self.args = []
      self.code = "\n{\n\n}"
      self.eyed = attrs["id"]
    elif tag == "arg":
      self.args.append(attrs)

  def characters(self, content):
    self.content += content

  def endElement(self, tag):
    self.indent = self.indent[0:-2]
    debug(self.indent + ("endElement(%s)" % (tag)))

    if tag == "include":
      if not self.only_dump_ids:
        print self.content
    elif tag == "description":
      self.description += self.content
    elif tag == "codeForStub":
      self.code = "\n{\n"
      for code in self.content.split("\r\n"):
        code = code.strip()
        if len(code) != 0:
          self.code += "  " + code + "\n"
      self.code += "}"
    elif tag == "function":
      self.dump()

def debug(message):
  debug = False
  if debug:
    print message

if __name__ == '__main__':
  filename      = None
  plugin        = None
  only_dump_ids = False
  
  if len(sys.argv) < 3:
    print "usage: %s v2-xml-callback-file plugin [--only-dump-ids]" % (sys.argv[0])
    exit(1)
  else:
    filename = sys.argv[1]
    plugin   = sys.argv[2]

  if len(sys.argv) == 4:
    if sys.argv[3] == "--only-dump-ids":
      only_dump_ids = True

  if not os.path.isfile(filename):
    print "error: file %s does exist" % (filename)
    exit(1)
  
  parser = xml.sax.make_parser()
  parser.setContentHandler(CallbackTranslator(plugin, only_dump_ids))
  parser.parse(open(filename, "r"))

    
  
