#!/usr/bin/env python3

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

import sys
import optparse
import os
import subprocess
from pathlib import Path
import chef_config as config
from sys import platform

global commandQueue
commandQueue = ""


class TermColors:
    STRBOLD = '\033[1m'
    STRRESET = '\033[0m'
    STRCYAN = '\033[1;36m'
    STRYELLOW = '\033[1;33m'


def splash():
    splashText = TermColors.STRBOLD + TermColors.STRYELLOW + '''\
  ______  __    __   _______  _______
 /      ||  |  |  | |   ____||   ____|
|  ,----'|  |__|  | |  |__   |  |__
|  |     |   __   | |   __|  |   __|
|  `----.|  |  |  | |  |____ |  |
 \\______||__|  |__| |_______||__|\
''' + TermColors.STRRESET
    return splash


def printc(strInput):
    color = TermColors.STRCYAN
    print(color + strInput + TermColors.STRRESET)


def definePaths():
    paths = dict()
    paths["scriptFolder"] = os.path.abspath(os.path.dirname(__file__))
    paths["matterFolder"] = paths["scriptFolder"] + "/../../"
    paths["rootSampleFolder"] = paths["scriptFolder"]
    paths["genFolder"] = paths["rootSampleFolder"] + "/zap-generated"
    paths["devices"] = []
    for filepath in Path(f"{paths['rootSampleFolder']}/devices").rglob('*.zap'):
        paths["devices"].append(
            str(os.path.splitext(os.path.basename(filepath))[0]))
    return paths


def checkPythonVersion():
    if sys.version_info[0] < 3:
        print('Must use Python 3. Current version is ' +
              str(sys.version_info[0]))
        exit(1)


def queuePrint(str):
    global commandQueue
    if (len(commandQueue) > 0):
        commandQueue = commandQueue + "; "
    commandQueue = commandQueue + "echo -e " + str


def queueCommand(command):
    global commandQueue
    if (len(commandQueue) > 0):
        commandQueue = commandQueue + "; "
    commandQueue = commandQueue + command


def execQueue():
    global commandQueue
    subprocess.run(commandQueue, shell=True, executable=shellApp)
    commandQueue = ""


def hexInputToInt(valIn):
    '''Parses inputs as hexadecimal numbers, takes into account optional 0x
       prefix
    '''
    if (type(valIn) is str):
        if (valIn[0:2].lower() == "0x"):
            valOut = valIn[2:]
        else:
            valOut = valIn
        valOut = int(valOut, 16)
    else:
        valOut = valIn
    return valOut


def main(argv):
    checkPythonVersion()
    paths = definePaths()
    global myEnv
    myEnv = os.environ.copy()

    #
    # Build environment switches
    #

    global shellApp
    if platform == "linux" or platform == "linux2":
        shellApp = '/bin/bash'
    elif platform == "darwin":
        shellApp = '/bin/zsh'
    elif platform == "win32":
        print('Windows is currently not supported. Use Linux or MacOS platforms')
        exit(1)

    #
    # Checks if user has configured its custom settings
    #
    if not config.configured:
        print('Please edit chef_config.py file and change the <configured> flag to True')
        exit(1)

    #
    # Arguments parser
    #

    deviceTypes = "\n    ".join(paths["devices"])

    usage = f'''usage: chef.py [options]

Platforms:
    nrf52840dk_nrf52840
    esp32
    linux

Device Types:
    {deviceTypes}

Notes:
- Whenever you change a device type, make sure to also use options -zbe
- Be careful if you have more than one device connected. The script assumes you have only one device connected and might flash the wrong one\
'''
    parser = optparse.OptionParser(usage=usage)

    parser.add_option("-u", "--update_toolchain", help="updates toolchain & installs zap",
                      action="store_true", dest="doUpdateToolchain")
    parser.add_option("-b", "--build", help="builds",
                      action="store_true", dest="doBuild")
    parser.add_option("-c", "--clean", help="clean build. Only valid if also building",
                      action="store_true", dest="doClean")
    parser.add_option("-f", "--flash", help="flashes device",
                      action="store_true", dest="doFlash")
    parser.add_option("-e", "--erase", help="erases flash before flashing. Only valid if also flashing",
                      action="store_true", dest="doErase")
    parser.add_option("-i", "--terminal", help="opens terminal to interact with with device",
                      action="store_true", dest="doInteract")
    parser.add_option("-m", "--menuconfig", help="runs menuconfig on platforms that support it",
                      action="store_true", dest="doMenuconfig")
    parser.add_option("", "--bootstrap_zap", help="installs zap dependencies",
                      action="store_true", dest="doBootstrapZap")
    parser.add_option("-z", "--zap", help="runs zap to generate data model & interaction model artifacts",
                      action="store_true", dest="doRunZap")
    parser.add_option("-g", "--zapgui", help="runs zap GUI display to allow editing of data model",
                      action="store_true", dest="doRunGui")
    parser.add_option("-d", "--device", dest="sampleDeviceTypeName",
                      help="specifies device type. Default is lighting. See info above for supported device types", metavar="TARGET", default="lighting")
    parser.add_option("-t", "--target", type='choice',
                      action='store',
                      dest="buildTarget",
                      help="specifies target platform. Default is esp32. See info below for currently supported target platforms",
                      choices=['nrf52840dk_nrf52840', 'esp32', 'linux', ],
                      metavar="TARGET",
                      default="esp32")
    parser.add_option("-r", "--rpc", help="enables Pigweed RPC interface. Enabling RPC disables the shell interface. Your sdkconfig configurations will be reverted to default. Default is PW RPC off. When enabling or disabling this flag, on the first build force a clean build with -c", action="store_true", dest="doRPC")
    parser.add_option("-v", "--vid", dest="vid",
                      help="specifies the Vendor ID. Default is 0xFFF1", metavar="VID", default=0xFFF1)
    parser.add_option("-p", "--pid", dest="pid",
                      help="specifies the Product ID. Default is 0x8000", metavar="PID", default=0x8000)
    parser.add_option("", "--rpc_console", help="Opens PW RPC Console",
                      action="store_true", dest="doRPC_CONSOLE")

    options, _ = parser.parse_args(argv)

    splash()

    #
    # Platform Folder
    #

    queuePrint(f"Target is set to {options.sampleDeviceTypeName}")
    queuePrint("Setting up environment...")
    if options.buildTarget == "esp32":
        paths["platFolder"] = os.path.normpath(
            paths["rootSampleFolder"] + "/esp32")
        queueCommand(f"source {config.esp32Folder}/export.sh")
    elif options.buildTarget == "nrf52840dk_nrf52840":
        paths["platFolder"] = os.path.normpath(
            paths["rootSampleFolder"] + "/nrfconnect")
        queueCommand(f"source {config.nrfconnectFolder}/zephyr/zephyr-env.sh")
        queueCommand("export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb")
    elif options.buildTarget == "linux":
        pass
    else:
        print(f"Target {options.buildTarget} not supported")

    queueCommand(f"source {paths['matterFolder']}/scripts/activate.sh")

    #
    # Toolchain update
    #

    if options.doUpdateToolchain:
        if options.buildTarget == "esp32":
            queuePrint("ESP32 toolchain update not supported. Skipping")
        elif options.buildTarget == "nrf52840dk_nrf52840":
            queuePrint("Updating toolchain")
            queueCommand(
                f"cd {paths['matterFolder']} && python3 scripts/setup/nrfconnect/update_ncs.py --update")
        elif options.buildTarget == "linux":
            queuePrint("Linux toolchain update not supported. Skipping")

    #
    # ZAP bootstrapping
    #

    if options.doBootstrapZap:
        if platform == "linux" or platform == "linux2":
            queuePrint("Installing ZAP OS package dependencies")
            queueCommand(f"sudo apt-get install sudo apt-get install node node-yargs npm libpixman-1-dev libcairo2-dev libpango1.0-dev node-pre-gyp libjpeg9-dev libgif-dev node-typescript")
        if platform == "darwin":
            queuePrint("Installation of ZAP OS packages not supported on MacOS")
        if platform == "win32":
            queuePrint(
                "Installation of ZAP OS packages not supported on Windows")

        queuePrint("Running NPM to install ZAP Node.JS dependencies")
        queueCommand(
            f"cd {paths['matterFolder']}/third_party/zap/repo/ && npm install")

    #
    # Cluster customization
    #

    if options.doRunGui:
        queuePrint("Starting ZAP GUI editor")
        queueCommand(f"cd {paths['rootSampleFolder']}/devices")
        queueCommand(
            f"{paths['matterFolder']}/scripts/tools/zap/run_zaptool.sh {options.sampleDeviceTypeName}.zap")

    if options.doRunZap:
        queuePrint("Running ZAP script to generate artifacts")
        queueCommand(f"mkdir -p {paths['genFolder']}/")
        queueCommand(f"rm {paths['genFolder']}/*")
        queueCommand(
            f"{paths['matterFolder']}/scripts/tools/zap/generate.py {paths['rootSampleFolder']}/devices/{options.sampleDeviceTypeName}.zap -o {paths['genFolder']}")
        # sometimes af-gen-event.h is not generated
        queueCommand(f"touch {paths['genFolder']}/af-gen-event.h")

    #
    # Menuconfig
    #

    if options.doMenuconfig:
        if options.buildTarget == "esp32":
            queueCommand(f"cd {paths['rootSampleFolder']}/esp32")
            queueCommand("idf.py menuconfig")
        elif options.buildTarget == "nrf52840dk_nrf52840":
            queueCommand(f"cd {paths['rootSampleFolder']}/nrfconnect")
            queueCommand("west build -t menuconfig")
        elif options.buildTarget == "linux":
            queuePrint("Menuconfig not available on Linux target. Skipping")

    #
    # Build
    #

    if options.doBuild:
        queuePrint("Building...")
        if options.doRPC:
            queuePrint("RPC PW enabled")
            queueCommand(
                f"export SDKCONFIG_DEFAULTS={paths['rootSampleFolder']}/esp32/sdkconfig_rpc.defaults")
        else:
            queuePrint("RPC PW disabled")
            queueCommand(
                f"export SDKCONFIG_DEFAULTS={paths['rootSampleFolder']}/esp32/sdkconfig.defaults")
        options.vid = hexInputToInt(options.vid)
        options.pid = hexInputToInt(options.pid)
        queuePrint(
            f"Product ID 0x{options.pid:02X} / Vendor ID 0x{options.vid:02X}")
        queueCommand(f"cd {paths['rootSampleFolder']}")

        if (options.buildTarget == "esp32") or (options.buildTarget == "nrf52840dk_nrf52840"):
            queueCommand(f'''
cat > project_include.cmake <<EOF
set(CONFIG_DEVICE_VENDOR_ID {options.vid})
set(CONFIG_DEVICE_PRODUCT_ID {options.pid})
set(CONFIG_ENABLE_PW_RPC {"1" if options.doRPC else "0"})
set(SAMPLE_NAME {options.sampleDeviceTypeName})
EOF
true''')

        if options.buildTarget == "esp32":
            queueCommand(f"cd {paths['rootSampleFolder']}/esp32")
            if options.doClean:
                queueCommand(f"rm {paths['rootSampleFolder']}/esp32/sdkconfig")
                queueCommand(f"cd {paths['rootSampleFolder']}/esp32")
                queueCommand(f"rm -rf {paths['rootSampleFolder']}/build")
                queueCommand("idf.py fullclean")
            queueCommand("idf.py build")
        elif options.buildTarget == "nrf52840dk_nrf52840":
            queueCommand(f"cd {paths['rootSampleFolder']}/nrfconnect")
            if options.doClean:
                queueCommand(f"west build -b {options.buildTarget} -c")
            else:
                queueCommand(f"west build -b {options.buildTarget}")
        elif options.buildTarget == "linux":
            queueCommand(f"cd {paths['rootSampleFolder']}/linux")
            queueCommand(f'''
cat > args.gni <<EOF
import("//build_overrides/chip.gni")
import("\\${{chip_root}}/config/standalone/args.gni")
chip_shell_cmd_server = false
target_defines = ["CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID={options.vid}", "CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID={options.pid}", "CONFIG_ENABLE_PW_RPC={'1' if options.doRPC else '0'}"]
EOF

cat > sample.gni <<EOF
sample_zap_file = "{options.sampleDeviceTypeName}.zap"
sample_name = "{options.sampleDeviceTypeName}"
EOF
true''')
            if options.doClean:
                queueCommand(f"rm -rf out")
            queueCommand("gn gen out")
            queueCommand("ninja -C out")

    #
    # Compilation DB TODO
    #

    #
    # Flash
    #

    if options.doFlash:
        queuePrint("Flashing target")
        if options.buildTarget == "esp32":
            queueCommand(f"cd {paths['rootSampleFolder']}/esp32")
            if options.doErase:
                queueCommand(
                    f"idf.py -p {config.esp32SerialDevice} erase_flash")
            queueCommand(f"idf.py -p {config.esp32SerialDevice} flash")
        elif options.buildTarget == "nrf52840dk_nrf52840":
            queueCommand(f"cd {paths['rootSampleFolder']}/nrfconnect")
            if options.doErase:
                queueCommand("west flash --erase")
            else:
                queueCommand("west flash")

    #
    # Terminal interaction
    #

    if options.doInteract:
        queuePrint("Starting terminal...")
        if options.buildTarget == "esp32":
            queueCommand(f"cd {paths['rootSampleFolder']}/esp32")
            queueCommand(f"idf.py -p {config.esp32SerialDevice} monitor")
        elif options.buildTarget == "nrf52840dk_nrf52840":
            queueCommand("killall screen")
            queueCommand(f"screen {config.nrfConnectSerialDevice} 115200")
        elif options.buildTarget == "linux":
            queuePrint(
                f"{paths['rootSampleFolder']}/linux/out/{options.sampleDeviceTypeName}")
            queueCommand(
                f"{paths['rootSampleFolder']}/linux/out/{options.sampleDeviceTypeName}")

    #
    # RPC Console
    #
    if options.doRPC_CONSOLE:
        queueCommand(
            f"python3 -m chip_rpc.console --device {config.esp32SerialDevice}")

    queuePrint("Done")

    execQueue()


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
