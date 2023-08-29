
import os
import argparse
import json
import subprocess
import shutil
import sys

# workaround untill we find a better solution 
sys.path.append('/usr/local/miniconda/lib/python3.9/site-packages/')
from deb_pkg_tools.package import build_package as build_pkg
import deb_pkg_tools.package

# content of License file to be used in packaging
silabsCopyrights = '/******************************************************************************\n\
 * # License\n\
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>\n\
 ******************************************************************************\n\
 * The licensor of this software is Silicon Laboratories Inc. Your use of this\n\
 * software is governed by the terms of Silicon Labs Master Software License\n\
 * Agreement (MSLA) available at\n\
 * www.silabs.com/about-us/legal/master-software-license-agreement. This\n\
 * software is distributed to you in Source Code format and is governed by the\n\
 * sections of the MSLA applicable to Source Code.\n\
 *\n\
 *****************************************************************************/'

# service file format to be used in packaging
serviceFileEntry = '[Unit]\n\
Description=%s\n\
After=network.target mosquitto.service\n\
\n\
[Service]\n\
ExecStart=/usr/bin/%s --conf /etc/uic/uic.cfg\n\
Restart=on-failure\n\
KillMode=mixed\n\
KillSignal=SIGINT\n\
User=uic\n\
Group=uic\n\
\n\
[Install]\n\
WantedBy=multi-user.target\n\
'

# function adds service file in temporary packaging directory
def addServiceFile(srvDir, desc, pkgName):
  # open service file in write mode
  srvEntryFile = open(srvDir + '/' + pkgName + '.service', 'w')

  # fill description and pkg name into service file format and dump into file
  srvEntryFile.write(serviceFileEntry%(desc, pkgName))
  srvEntryFile.close()

# function will setup the temporary packaging directory
def SetupPkgDir(pkgDir, pkgName, desc, pkgBin):
  # bin path
  binDir = pkgDir + '/usr/bin'
  # service file path
  srvDir = pkgDir + '/lib/systemd/system'
  # docs path  
  docDir = pkgDir + '/usr/share/doc/' + pkgName
  # var files path
  varDir = pkgDir + '/var/lib/' + pkgName
  
  # create the various folders within packaging directory if doesn't already exist
  os.makedirs(binDir,  exist_ok=True)
  os.makedirs(srvDir,  exist_ok=True)
  os.makedirs(docDir,  exist_ok=True)
  os.makedirs(varDir,  exist_ok=True)
  
  # dump copyright content to license file in docs path
  copyrightsFile = open(docDir + '/copyright', 'w')
  copyrightsFile.write(silabsCopyrights)
  copyrightsFile.close()

  # add service file
  addServiceFile(srvDir, desc, pkgName)

  # copy binary to bin path
  shutil.copy2(pkgBin, binDir + '/' + pkgName)

# function fills in the ControlFile of debian package with meta data
def updateControlFile(pkgDir, pkgName, desc, ver, unifyVer, arch='arm64', deps=['libunify'], priority='optional', section='devel'):
  # create DEBIAN file within tmp root for package if doesn't already exist
  os.makedirs(pkgDir + '/DEBIAN', exist_ok=True)

  # fill in available meta data into dictionary
  fields = {}
  fields[deb_pkg_tools.control.module.MANDATORY_BINARY_CONTROL_FIELDS[0]] = arch #Architecture
  fields['Depends'] = 'libunify (>= ' + unifyVer + ')'
  print(fields['Depends'])
  fields[deb_pkg_tools.control.module.MANDATORY_BINARY_CONTROL_FIELDS[1]] = desc #Package Description
  fields[deb_pkg_tools.control.module.MANDATORY_BINARY_CONTROL_FIELDS[2]] = 'Silicon Labs' #Maintainer
  fields[deb_pkg_tools.control.module.MANDATORY_BINARY_CONTROL_FIELDS[3]] = pkgName #Package Name
  fields[deb_pkg_tools.control.module.MANDATORY_BINARY_CONTROL_FIELDS[4]] = ver  #Version
  fields['Priority'] = priority
  fields['Section'] = section
  fields[deb_pkg_tools.control.module.INSTALLED_SIZE_FIELD] = 0

  # create control file from the meta data filled
  deb_pkg_tools.control.module.create_control_file(pkgDir + '/DEBIAN/control', fields)

  # find the size of package dir (including control file size) and update into control file
  size=subprocess.check_output(['du','-s', pkgDir]).split()[0].decode('utf-8')
  fields[deb_pkg_tools.control.module.INSTALLED_SIZE_FIELD] = size
  deb_pkg_tools.control.module.create_control_file(pkgDir + '/DEBIAN/control', fields)

def main():
  # setup and parse commandline arguments of the script
  cmdArgParser = argparse.ArgumentParser(prog='create_package', description='Create debian pkg', epilog='')
  cmdArgParser.add_argument('cfg_file')
  cmdArgParser.add_argument('pkg_name')
  cmdArgParser.add_argument('pkg_bin')
  cmdArgParser.add_argument('pkg_dir')
  cmdArgParser.add_argument('arch')
  cmdArgParser.add_argument('-o', '--output-dir')

  cmdArgs = cmdArgParser.parse_args()

  # set update output dir path if set via cmdline args
  out_dir = cmdArgs.pkg_dir + '/..'
  if cmdArgs.output_dir is not None:
    out_dir = cmdArgs.output_dir

  # Read package config file
  cfg = open(cmdArgs.cfg_file, 'r')
  ctrlFields = json.load(cfg)
  
  # setup package root with necessary files
  SetupPkgDir(cmdArgs.pkg_dir, cmdArgs.pkg_name, ctrlFields['Description'], cmdArgs.pkg_bin)
  
  # Add control file for package with meta data info
  updateControlFile(cmdArgs.pkg_dir, cmdArgs.pkg_name, ctrlFields['Description'], ctrlFields['Version'], ctrlFields['UnifyVersion'], cmdArgs.arch)
  
  # create if output path doesn't already exists and create debian package in the output path
  os.makedirs(out_dir,  exist_ok=True)
  build_pkg(cmdArgs.pkg_dir, out_dir)

  # delete the tempory package root directory
  shutil.rmtree(cmdArgs.pkg_dir)

main()