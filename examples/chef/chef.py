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
import sys
import textwrap
import yaml

import constants
import stateful_shell


TermColors = constants.TermColors

shell = stateful_shell.StatefulShell()

_CHEF_SCRIPT_PATH = os.path.abspath(os.path.dirname(__file__))
_REPO_BASE_PATH = os.path.join(_CHEF_SCRIPT_PATH, "../../")
_DEVICE_FOLDER = os.path.join(_CHEF_SCRIPT_PATH, "devices")
_DEVICE_LIST = [file[:-4] for file in os.listdir(_DEVICE_FOLDER) if file.endswith(".zap")]

gen_dir = ""  # Filled in after sample app type is read from args.


def splash():
    splashText = textwrap.dedent(
        f"""\
        {TermColors.STRBOLD}{TermColors.STRYELLOW}
          ______  __    __   _______  _______
         /      ||  |  |  | |   ____||   ____|
        |  ,----'|  |__|  | |  |__   |  |__
        |  |     |   __   | |   __|  |   __|
        |  `----.|  |  |  | |  |____ |  |
         \\______||__|  |__| |_______||__|{TermColors.STRRESET}
        """)
    print(splashText)


def loadConfig():
    config = dict()
    config["nrfconnect"] = dict()
    config["esp32"] = dict()

    configFile = f"{_CHEF_SCRIPT_PATH}/config.yaml"
    if (os.path.exists(configFile)):
        configStream = open(configFile, 'r')
        config = yaml.load(configStream, Loader=yaml.SafeLoader)
        configStream.close()
    else:
        print("Running for the first time and configuring config.yaml. " +
              "Change this configuration file to include correct configuration " +
              "for the vendor's SDK")
        configStream = open(configFile, 'w')
        config["nrfconnect"]["ZEPHYR_BASE"] = os.environ.get('ZEPHYR_BASE')
        config["nrfconnect"]["TTY"] = None
        config["esp32"]["IDF_PATH"] = os.environ.get('IDF_PATH')
        config["esp32"]["TTY"] = None
        print(yaml.dump(config))
        yaml.dump(config, configStream)
        configStream.close()

    return config


def checkPythonVersion():
    if sys.version_info[0] < 3:
        print('Must use Python 3. Current version is ' +
              str(sys.version_info[0]))
        exit(1)


def main(argv):
    checkPythonVersion()
    config = loadConfig()

    #
    # Build environment switches
    #

    if sys.platform == "win32":
        print('Windows is currently not supported. Use Linux or MacOS platforms')
        exit(1)

    #
    # Arguments parser
    #

    deviceTypes = "\n  ".join(_DEVICE_LIST)

    usage = textwrap.dedent(f"""\
        usage: chef.py [options]

        Platforms:
            nrfconnect
            esp32
            linux

        Device Types:
            {deviceTypes}

        Notes:
        - Whenever you change a device type, make sure to also use options -zbe
        - Be careful if you have more than one device connected. The script assumes you have only one device connected and might flash the wrong one\
        """)
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
                      choices=['nrfconnect', 'esp32', 'linux', ],
                      metavar="TARGET",
                      default="esp32")
    parser.add_option("-r", "--rpc", help="enables Pigweed RPC interface. Enabling RPC disables the shell interface. Your sdkconfig configurations will be reverted to default. Default is PW RPC off. When enabling or disabling this flag, on the first build force a clean build with -c", action="store_true", dest="doRPC")
    parser.add_option("-v", "--vid", dest="vid", type=int,
                      help="specifies the Vendor ID. Default is 0xFFF1", metavar="VID", default=0xFFF1)
    parser.add_option("-p", "--pid", dest="pid", type=int,
                      help="specifies the Product ID. Default is 0x8000", metavar="PID", default=0x8000)
    parser.add_option("", "--rpc_console", help="Opens PW RPC Console",
                      action="store_true", dest="doRPC_CONSOLE")
    parser.add_option("-y", "--tty", help="Enumerated USB tty/serial interface enumerated for your physical device. E.g.: /dev/ACM0",
                      dest="tty", metavar="TTY", default=None)

    options, _ = parser.parse_args(argv)

    splash()

    #
    # Platform Folder
    #

    print(f"Target is set to {options.sampleDeviceTypeName}")
    global gen_dir
    gen_dir = (
        f"{_CHEF_SCRIPT_PATH}/out/{options.sampleDeviceTypeName}/zap-generated/")

    print("Setting up environment...")
    if options.buildTarget == "esp32":
        if config['esp32']['IDF_PATH'] is None:
            print('Path for esp32 SDK was not found. Make sure esp32.IDF_PATH is set on your config.yaml file')
            exit(1)
        plat_folder = os.path.normpath(f"{_CHEF_SCRIPT_PATH}/esp32")
        shell.run_cmd(f'source {config["esp32"]["IDF_PATH"]}/export.sh')
    elif options.buildTarget == "nrfconnect":
        if config['nrfconnect']['ZEPHYR_BASE'] is None:
            print('Path for nrfconnect SDK was not found. Make sure nrfconnect.ZEPHYR_BASE is set on your config.yaml file')
            exit(1)
        plat_folder = os.path.normpath(f"{_CHEF_SCRIPT_PATH}/nrfconnect")
        shell.run_cmd(f'source {config["nrfconnect"]["ZEPHYR_BASE"]}/zephyr-env.sh')
        shell.run_cmd("export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb")
    elif options.buildTarget == "linux":
        pass
    else:
        print(f"Target {options.buildTarget} not supported")

    shell.run_cmd(f"source {_REPO_BASE_PATH}/scripts/activate.sh")

    #
    # Toolchain update
    #

    if options.doUpdateToolchain:
        if options.buildTarget == "esp32":
            print("ESP32 toolchain update not supported. Skipping")
        elif options.buildTarget == "nrfconnect":
            print("Updating toolchain")
            shell.run_cmd(
                f"cd {_REPO_BASE_PATH} && python3 scripts/setup/nrfconnect/update_ncs.py --update")
        elif options.buildTarget == "linux":
            print("Linux toolchain update not supported. Skipping")

    #
    # ZAP bootstrapping
    #

    if options.doBootstrapZap:
        if sys.platform == "linux" or sys.platform == "linux2":
            print("Installing ZAP OS package dependencies")
            shell.run_cmd(
                f"sudo apt-get install sudo apt-get install node node-yargs npm libpixman-1-dev libcairo2-dev libpango1.0-dev node-pre-gyp libjpeg9-dev libgif-dev node-typescript")
        if sys.platform == "darwin":
            print("Installation of ZAP OS packages not supported on MacOS")
        if sys.platform == "win32":
            print(
                "Installation of ZAP OS packages not supported on Windows")

        print("Running NPM to install ZAP Node.JS dependencies")
        shell.run_cmd(
            f"cd {_REPO_BASE_PATH}/third_party/zap/repo/ && npm install")

    #
    # Cluster customization
    #

    if options.doRunGui:
        print("Starting ZAP GUI editor")
        shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/devices")
        shell.run_cmd(
            f"{_REPO_BASE_PATH}/scripts/tools/zap/run_zaptool.sh {options.sampleDeviceTypeName}.zap")

    if options.doRunZap:
        print("Running ZAP script to generate artifacts")
        shell.run_cmd(f"mkdir -p {gen_dir}/")
        shell.run_cmd(f"rm {gen_dir}/*")
        shell.run_cmd(
            f"{_REPO_BASE_PATH}/scripts/tools/zap/generate.py {_CHEF_SCRIPT_PATH}/devices/{options.sampleDeviceTypeName}.zap -o {gen_dir}")
        # af-gen-event.h is not generated
        shell.run_cmd(f"touch {gen_dir}/af-gen-event.h")

    #
    # Menuconfig
    #

    if options.doMenuconfig:
        if options.buildTarget == "esp32":
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/esp32")
            shell.run_cmd("idf.py menuconfig")
        elif options.buildTarget == "nrfconnect":
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/nrfconnect")
            shell.run_cmd("west build -t menuconfig")
        elif options.buildTarget == "linux":
            print("Menuconfig not available on Linux target. Skipping")

    #
    # Build
    #

    if options.doBuild:
        print("Building...")
        if options.doRPC:
            print("RPC PW enabled")
            shell.run_cmd(
                f"export SDKCONFIG_DEFAULTS={_CHEF_SCRIPT_PATH}/esp32/sdkconfig_rpc.defaults")
        else:
            print("RPC PW disabled")
            shell.run_cmd(
                f"export SDKCONFIG_DEFAULTS={_CHEF_SCRIPT_PATH}/esp32/sdkconfig.defaults")
        print(
            f"Product ID 0x{options.pid:02X} / Vendor ID 0x{options.vid:02X}")
        shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}")

        if (options.buildTarget == "esp32") or (options.buildTarget == "nrfconnect"):
            with open("project_include.cmake", "w") as f:
                f.write(textwrap.dedent(f"""\
                        set(CONFIG_DEVICE_VENDOR_ID {options.vid})
                        set(CONFIG_DEVICE_PRODUCT_ID {options.pid})
                        set(CONFIG_ENABLE_PW_RPC {"1" if options.doRPC else "0"})
                        set(SAMPLE_NAME {options.sampleDeviceTypeName})"""))

        if options.buildTarget == "esp32":
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/esp32")
            if options.doClean:
                shell.run_cmd(f"rm {_CHEF_SCRIPT_PATH}/esp32/sdkconfig")
                shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/esp32")
                shell.run_cmd(f"rm -rf {_CHEF_SCRIPT_PATH}/esp32/build")
                shell.run_cmd("idf.py fullclean")
            shell.run_cmd("idf.py build")
        elif options.buildTarget == "nrfconnect":
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/nrfconnect")
            if options.doClean:
                # shell.run_cmd(f"rm -rf {paths['rootSampleFolder']}/nrfconnect/build")
                shell.run_cmd("west build -b nrf52840dk_nrf52840")
            else:
                shell.run_cmd("west build -b nrf52840dk_nrf52840")
        elif options.buildTarget == "linux":
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/linux")
            with open(f"{_CHEF_SCRIPT_PATH}/linux/args.gni", "w") as f:
                f.write(textwrap.dedent(f"""\
                        import("//build_overrides/chip.gni")
                        import("\\${{chip_root}}/config/standalone/args.gni")
                        chip_shell_cmd_server = false
                        target_defines = ["CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID={options.vid}", "CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID={options.pid}", "CONFIG_ENABLE_PW_RPC={'1' if options.doRPC else '0'}"]
                        """))
            with open(f"{_CHEF_SCRIPT_PATH}/linux/sample.gni", "w") as f:
                f.write(textwrap.dedent(f"""\
                        sample_zap_file = "{options.sampleDeviceTypeName}.zap"
                        sample_name = "{options.sampleDeviceTypeName}
                        """))
            if options.doClean:
                shell.run_cmd(f"rm -rf out")
            shell.run_cmd("gn gen out")
            shell.run_cmd("ninja -C out")

    #
    # Compilation DB TODO
    #

    #
    # Flash
    #

    if options.doFlash:
        print("Flashing target")
        if options.buildTarget == "esp32":
            if config['esp32']['TTY'] is None:
                print('The path for the serial enumeration for esp32 is not set. Make sure esp32.TTY is set on your config.yaml file')
                exit(1)
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/esp32")
            if options.doErase:
                shell.run_cmd(
                    f"idf.py -p {config['esp32']['TTY']} erase-flash")
            shell.run_cmd(f"idf.py -p {config['esp32']['TTY']} flash")
        elif options.buildTarget == "nrfconnect":
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/nrfconnect")
            if options.doErase:
                shell.run_cmd("rm -rf build")
            else:
                shell.run_cmd("west flash")

    #
    # Terminal interaction
    #

    if options.doInteract:
        print("Starting terminal...")
        if options.buildTarget == "esp32":
            if config['esp32']['TTY'] is None:
                print('The path for the serial enumeration for esp32 is not set. Make sure esp32.TTY is set on your config.yaml file')
                exit(1)
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/esp32")
            shell.run_cmd(f"idf.py -p {config['esp32']['TTY']} monitor")
        elif options.buildTarget == "nrfconnect":
            if config['nrfconnect']['TTY'] is None:
                print('The path for the serial enumeration for nordic is not set. Make sure nrfconnect.TTY is set on your config.yaml file')
                exit(1)
            shell.run_cmd("killall screen")
            shell.run_cmd(f"screen {config['nrfconnect']['TTY']} 115200")
        elif options.buildTarget == "linux":
            print(
                f"{_CHEF_SCRIPT_PATH}/linux/out/{options.sampleDeviceTypeName}")
            shell.run_cmd(
                f"{_CHEF_SCRIPT_PATH}/linux/out/{options.sampleDeviceTypeName}")

    #
    # RPC Console
    #
    if options.doRPC_CONSOLE:
        shell.run_cmd(
            f"python3 -m chip_rpc.console --device {config['esp32']['TTY']}")

    print("Done")


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
