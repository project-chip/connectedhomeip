#!/usr/bin/env python

# Copyright (c) 2020 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import os
import sys

def main(argv):
  parser = argparse.ArgumentParser(description=("Generate dbus bindings."))

  parser.add_argument(
      "--output_files",
      default=None,
      help="The list of source files containing the GDBus proxy implementation")
  
  parser.add_argument(
      "--input_files",
      default=None,
      help="The list of dbus service definition XML files.")

  options = parser.parse_args(argv)

  if options.input_files is None:
    parser.error("The input xml file list is empty.")

  if options.output_files is None:
    parser.error("The output file list is empty.")

  input_file_list = options.input_files.split(',')
  output_file_list = options.output_files.split(',')

  # assemble the shell command for dbus-binding-generator
  for i in range(3):
    cmd = "gdbus-codegen --body --output "
    cmd += output_file_list[i]
    cmd += " --c-namespace Wpa "
    cmd += input_file_list[i]
    os.system(cmd)
    os.system('sed -i "s/config\.h/BuildConfig.h/g" "%s"' % output_file_list[i])
  
  return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))