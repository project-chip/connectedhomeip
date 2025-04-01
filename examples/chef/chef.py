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

import json
import optparse
import os
import re
import shutil
import sys
import tarfile
import textwrap
from typing import Any, Dict

import constants
import stateful_shell
import yaml
from sample_app_util import zap_file_parser

TermColors = constants.TermColors

shell = stateful_shell.StatefulShell()

_CHEF_SCRIPT_PATH = os.path.abspath(os.path.dirname(__file__))
_REPO_BASE_PATH = os.path.join(_CHEF_SCRIPT_PATH, "../../")
_DEVICE_FOLDER = os.path.join(_CHEF_SCRIPT_PATH, "devices")
_DEVICE_LIST = [file[:-4]
                for file in os.listdir(_DEVICE_FOLDER) if file.endswith(".zap") and file != 'template.zap']
_CICD_CONFIG_FILE_NAME = os.path.join(_CHEF_SCRIPT_PATH, "cicd_config.json")
_CD_STAGING_DIR = os.path.join(_CHEF_SCRIPT_PATH, "staging")
_EXCLUDE_DEVICE_FROM_LINUX_CI = [  # These do not compile / deprecated.
    "noip_rootnode_dimmablelight_bCwGYSDpoe",
    "icd_rootnode_contactsensor_ed3b19ec55",
    "rootnode_refrigerator_temperaturecontrolledcabinet_temperaturecontrolledcabinet_ffdb696680",
]

gen_dir = ""  # Filled in after sample app type is read from args.


def splash() -> None:
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
    flush_print(splashText)


def load_config() -> None:
    config = dict()
    config["nrfconnect"] = dict()
    config["esp32"] = dict()
    config["silabs-thread"] = dict()
    config["ameba"] = dict()
    config["telink"] = dict()
    configFile = f"{_CHEF_SCRIPT_PATH}/config.yaml"
    if (os.path.exists(configFile)):
        configStream = open(configFile, 'r')
        config = yaml.load(configStream, Loader=yaml.SafeLoader)
        configStream.close()
    else:
        flush_print("Running for the first time and configuring config.yaml. " +
                    "Change this configuration file to include correct configuration " +
                    "for the vendor's SDK")
        configStream = open(configFile, 'w')
        config["nrfconnect"]["ZEPHYR_BASE"] = os.environ.get('ZEPHYR_BASE')
        config["nrfconnect"]["ZEPHYR_SDK_INSTALL_DIR"] = os.environ.get(
            'ZEPHYR_SDK_INSTALL_DIR')
        config["nrfconnect"]["TTY"] = None
        config["esp32"]["IDF_PATH"] = os.environ.get('IDF_PATH')
        config["esp32"]["TTY"] = None
        config["silabs-thread"]["GECKO_SDK"] = f"{_REPO_BASE_PATH}third_party/efr32_sdk/repo"
        config["silabs-thread"]["TTY"] = None
        config["silabs-thread"]["CU"] = None
        config["silabs-thread"]["SILABS_BOARD"] = None
        config["ameba"]["AMEBA_SDK"] = None
        config["ameba"]["MATTER_SDK"] = None
        config["ameba"]["MODEL"] = 'D'
        config["ameba"]["TTY"] = None
        config["telink"]["ZEPHYR_BASE"] = os.environ.get('TELINK_ZEPHYR_BASE')
        config["telink"]["ZEPHYR_SDK_INSTALL_DIR"] = os.environ.get(
            'TELINK_ZEPHYR_SDK_DIR')
        config["telink"]["TTY"] = None

        flush_print(yaml.dump(config))
        yaml.dump(config, configStream)
        configStream.close()

    return config


def check_python_version() -> None:
    if sys.version_info[0] < 3:
        flush_print('Must use Python 3. Current version is ' +
                    str(sys.version_info[0]))
        exit(1)


def load_cicd_config() -> Dict[str, Any]:
    with open(_CICD_CONFIG_FILE_NAME) as config_file:
        config = json.loads(config_file.read())
    return config


def flush_print(
        to_print: str,
        with_border: bool = False) -> None:
    """Prints and flushes stdout buffer.

    Args:
        to_print: The string to print.
        with_border: Add boarder above and below to_print.
    """
    if with_border:
        border = ('-' * len(to_print)) + '\n'
        to_print = f"{border}{to_print}\n{border}"
    print(to_print, flush=True)


def unwrap_cmd(cmd: str) -> str:
    """Dedent and replace new line with space."""
    return textwrap.dedent(cmd).replace("\n", " ")


def bundle(platform: str, device_name: str) -> None:
    """Filters files from the build output folder for CD.
    Clears _CD_STAGING_DIR.
    Calls bundle_{platform}(device_name).
    exit(1) for missing bundle_{platform}.
    Adds .matter files into _CD_STAGING_DIR.
    Generates metadata for device in _CD_STAGING_DIR.

    Args:
        platform: The platform to bundle.
        device_name: The example to bundle.
    """
    bundler_name = f"bundle_{platform}"
    matter_file = f"{device_name}.matter"
    zap_file = os.path.join(_DEVICE_FOLDER, f"{device_name}.zap")
    flush_print(f"Bundling {platform}", with_border=True)
    flush_print(f"Cleaning {_CD_STAGING_DIR}")
    shutil.rmtree(_CD_STAGING_DIR, ignore_errors=True)
    os.mkdir(_CD_STAGING_DIR)
    flush_print(f"Checking for {bundler_name}")
    chef_module = sys.modules[__name__]
    if hasattr(chef_module, bundler_name):
        flush_print(f"Found {bundler_name}")
        bundler = getattr(chef_module, bundler_name)
        bundler(device_name)
    else:
        flush_print(f"No bundle function for {platform}!")
        exit(1)
    flush_print(f"Copying {matter_file}")
    src_item = os.path.join(_DEVICE_FOLDER,
                            matter_file)
    dest_item = os.path.join(_CD_STAGING_DIR, matter_file)
    shutil.copy(src_item, dest_item)
    flush_print(f"Generating metadata for {device_name}")
    metadata_file = zap_file_parser.generate_metadata_file(zap_file)
    metadata_dest = os.path.join(_CD_STAGING_DIR,
                                 os.path.basename(metadata_file))
    shutil.copy(metadata_file, metadata_dest)


#
# Per-platform bundle functions
#


def bundle_linux(device_name: str) -> None:
    linux_root = os.path.join(_CHEF_SCRIPT_PATH,
                              "linux",
                              "out")
    map_file_name = f"{device_name}.map"
    src_item = os.path.join(linux_root, device_name)
    dest_item = os.path.join(_CD_STAGING_DIR, device_name)
    shutil.copy(src_item, dest_item)
    src_item = os.path.join(linux_root, map_file_name)
    dest_item = os.path.join(_CD_STAGING_DIR, map_file_name)
    shutil.copy(src_item, dest_item)


def bundle_nrfconnect(device_name: str) -> None:
    zephyr_exts = ["elf", "map", "hex"]
    script_files = ["firmware_utils.py",
                    "nrfconnect_firmware_utils.py"]
    nrf_root = os.path.join(_CHEF_SCRIPT_PATH,
                            "nrfconnect",
                            "build",
                            "nrfconnect",
                            "zephyr")
    scripts_root = os.path.join(_REPO_BASE_PATH,
                                "scripts",
                                "flashing")
    gen_script_path = os.path.join(scripts_root,
                                   "gen_flashing_script.py")
    sub_dir = os.path.join(_CD_STAGING_DIR, device_name)
    os.mkdir(sub_dir)
    for zephyr_ext in zephyr_exts:
        input_base = f"zephyr.{zephyr_ext}"
        output_base = f"{device_name}.{zephyr_ext}"
        src_item = os.path.join(nrf_root, input_base)
        if zephyr_ext == "hex":
            dest_item = os.path.join(sub_dir, output_base)
        else:
            dest_item = os.path.join(_CD_STAGING_DIR, output_base)
        shutil.copy(src_item, dest_item)
    for script_file in script_files:
        src_item = os.path.join(scripts_root, script_file)
        dest_item = os.path.join(sub_dir, script_file)
        shutil.copy(src_item, dest_item)
    shell.run_cmd(f"cd {sub_dir}")
    command = f"""\
    python3 {gen_script_path} nrfconnect
    --output {device_name}.flash.py
    --application {device_name}.hex"""
    shell.run_cmd(unwrap_cmd(command))


def bundle_esp32(device_name: str) -> None:
    """Reference example for bundle_{platform}
    functions, which should copy/move files from a build
    output dir into _CD_STAGING_DIR to be archived.

    Args:
        device_name: The device to bundle.
    """
    esp_root = os.path.join(_CHEF_SCRIPT_PATH,
                            "esp32",
                            "build")
    manifest_file = os.path.join(esp_root,
                                 "chip-shell.flashbundle.txt")
    with open(manifest_file) as manifest:
        for item in manifest:
            item = item.strip()
            src_item = os.path.join(esp_root, item)
            dest_item = os.path.join(_CD_STAGING_DIR, item)
            os.makedirs(os.path.dirname(dest_item), exist_ok=True)
            shutil.copy(src_item, dest_item)


def bundle_telink(device_name: str) -> None:
    zephyr_exts = ["elf", "map", "bin"]
    telink_root = os.path.join(_CHEF_SCRIPT_PATH,
                               "telink",
                               "build",
                               "zephyr")
    sub_dir = os.path.join(_CD_STAGING_DIR, device_name)
    os.mkdir(sub_dir)
    for zephyr_ext in zephyr_exts:
        input_base = f"zephyr.{zephyr_ext}"
        output_base = f"{device_name}.{zephyr_ext}"
        src_item = os.path.join(telink_root, input_base)
        if zephyr_ext == "bin":
            dest_item = os.path.join(sub_dir, output_base)
        else:
            dest_item = os.path.join(_CD_STAGING_DIR, output_base)
        shutil.copy(src_item, dest_item)


def main() -> int:

    check_python_version()
    config = load_config()
    cicd_config = load_cicd_config()

    #
    # Build environment switches
    #

    if sys.platform == "win32":
        flush_print(
            'Windows is currently not supported. Use Linux or MacOS platforms')
        exit(1)

    #
    # Arguments parser
    #

    deviceTypes = "\n            ".join(_DEVICE_LIST)

    usage = textwrap.dedent(f"""\
        usage: chef.py [options]

        Platforms:
            nrfconnect
            esp32
            linux
            silabs-thread
            ameba
            telink

        Device Types:
            {deviceTypes}

        Notes:
        - Whenever you change a device type, make sure to also use options -zbe
        - Be careful if you have more than one device connected.
          The script assumes you have only one device connected and might flash the wrong one
        """)
    parser = optparse.OptionParser(usage=usage)

    parser.add_option("-u", "--update_toolchain", help="updates toolchain & installs zap",
                      action="store_true", dest="do_update_toolchain")
    parser.add_option("-b", "--build", help="builds",
                      action="store_true", dest="do_build")
    parser.add_option("-c", "--clean", help="clean build. Only valid if also building",
                      action="store_true", dest="do_clean")
    parser.add_option("-f", "--flash", help="flashes device",
                      action="store_true", dest="do_flash")
    parser.add_option("-e", "--erase", help="erases flash before flashing. Only valid if also flashing",
                      action="store_true", dest="do_erase")
    parser.add_option("-i", "--terminal", help="opens terminal to interact with with device",
                      action="store_true", dest="do_interact")
    parser.add_option("-I", "--enable_lit_icd", help="enable LIT ICD (Long Idle Time Intermittently Connected Device) mode",
                      action="store_true", default=False)
    parser.add_option("-m", "--menuconfig", help="runs menuconfig on platforms that support it",
                      action="store_true", dest="do_menuconfig")
    parser.add_option("-z", "--zap", help="runs zap to generate data model & interaction model artifacts",
                      action="store_true", dest="do_run_zap")
    parser.add_option("-g", "--zapgui", help="runs zap GUI display to allow editing of data model",
                      action="store_true", dest="do_run_gui")
    parser.add_option("-d", "--device", dest="sample_device_type_name",
                      help="specifies device type. Default is lighting. See info above for supported device types",
                      metavar="TARGET", choices=_DEVICE_LIST)
    parser.add_option("-t", "--target", type='choice',
                      action='store',
                      dest="build_target",
                      help="specifies target platform. See info below for currently supported target platforms",
                      choices=['nrfconnect', 'esp32',
                               'linux', 'silabs-thread', 'ameba', 'telink'],
                      metavar="TARGET",
                      default="linux")
    parser.add_option("-r", "--rpc",
                      help=("enables Pigweed RPC interface. Enabling RPC disables the shell interface. "
                            "Your sdkconfig configurations will be reverted to default. Default is PW RPC off. "
                            "When enabling or disabling this flag, on the first build force a clean build with -c"),
                      action="store_true", dest="do_rpc", default=False)
    parser.add_option("-a", "--automated_test_stamp",
                      help="provide the additional stamp \"branch:commit_id\" as the software version string for automated tests.",
                      action="store_true", dest="do_automated_test_stamp")
    parser.add_option("-v", "--vid", dest="vid", type=int,
                      help="specifies the Vendor ID. Default is 0xFFF1", metavar="VID", default=0xFFF1)
    parser.add_option("-p", "--pid", dest="pid", type=int,
                      help="specifies the Product ID. Default is 0x8000", metavar="PID", default=0x8000)
    parser.add_option("-P", "--pname", dest="pname", type=str, metavar="PRODUCT_NAME",
                      help="specifies the Product Name. Default is TEST_PRODUCT", default="TEST_PRODUCT")
    parser.add_option("", "--rpc_console", help="Opens PW RPC Console",
                      action="store_true", dest="do_rpc_console")
    parser.add_option("-y", "--tty",
                      help="Enumerated USB tty/serial interface enumerated for your physical device. E.g.: /dev/ACM0",
                      dest="tty", metavar="TTY", default=None)
    # Build CD params.
    parser.add_option("", "--build_all",
                      help=("For use in CD only. Builds and bundles all chef examples for "
                            "the specified platform. Chef exits after completion."),
                      dest="build_all", action="store_true")
    parser.add_option("", "--dry_run", help="Display list of target builds of the --build_all command without building them.",
                      dest="dry_run", action="store_true")
    parser.add_option("", "--build_exclude",
                      help=("For use with --build_all. Build labels to exclude. "
                            "Accepts a regex pattern. Mutually exclusive with --build_include."),
                      dest="build_exclude")
    parser.add_option("", "--build_include",
                      help=("For use with --build_all. Build labels to include. "
                            "Accepts a regex pattern. Mutually exclusive with --build_exclude."),
                      dest="build_include")
    parser.add_option("-k", "--keep_going",
                      help="For use in CD only. Continues building all sample apps in the event of an error.",
                      dest="keep_going", action="store_true")
    parser.add_option("", "--ci",
                      help=("Builds Chef examples defined in cicd_config. "
                            "Uses specified target from -t. Chef exits after completion."),
                      dest="ci", action="store_true")
    parser.add_option("", "--ci_linux",
                      help=("Builds Chef Examples defined in cicd_config under ci_allow_list_linux. "
                            "Devices are built without -c for faster compilation."),
                      dest="ci_linux", action="store_true")
    parser.add_option("", "--cpu_type",
                      help="CPU type to compile for. Linux only.",
                      choices=["arm64", "arm", "x64"])
    parser.add_option("", "--enable_ipv4",
                      help="Enable IPv4 mDNS. Only applicable to platforms that can support IPV4 (e.g, Linux, ESP32)",
                      action="store_true", default=False)
    parser.add_option("", "--icd_persist_subscription",
                      help="Enable ICD persistent subscription and re-establish subscriptions from the server side after reboot",
                      action="store_true", default=False)
    parser.add_option("", "--icd_subscription_resumption",
                      help="Enable subscription resumption after timeout",
                      action="store_true", default=False)

    options, _ = parser.parse_args(sys.argv[1:])

    splash()

    #
    # CI
    #

    if options.ci:
        for device_name in cicd_config["ci_allow_list"]:
            if device_name not in _DEVICE_LIST:
                flush_print(
                    f"{device_name} in CICD config but not {_DEVICE_FOLDER}!")
                exit(1)
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}")
            command = f"./chef.py -cbr -d {device_name} -t {options.build_target}"
            flush_print(f"Building {command}", with_border=True)
            shell.run_cmd(command)
            bundle(options.build_target, device_name)
        exit(0)

    #
    # CI Linux
    #

    if options.ci_linux:
        for device_name in _DEVICE_LIST:
            if device_name in _EXCLUDE_DEVICE_FROM_LINUX_CI:
                continue
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}")
            command = f"./chef.py -br -d {device_name} -t linux"
            flush_print(f"Building {command}", with_border=True)
            shell.run_cmd(command)
            bundle("linux", device_name)
        exit(0)

    #
    # CD
    #

    if options.build_all:
        if options.build_include and options.build_exclude:
            flush_print(
                "Error. --build_include and --build_exclude are mutually exclusive options.")
            exit(1)
        flush_print("Building all chef examples")
        archive_prefix = "/workspace/artifacts/"
        archive_suffix = ".tar.gz"
        failed_builds = []
        for device_name in _DEVICE_LIST:
            for platform, label_args in cicd_config["cd_platforms"].items():
                for label, args in label_args.items():
                    archive_name = f"{label}-{device_name}"
                    if options.build_exclude and re.search(options.build_exclude, archive_name):
                        continue
                    elif options.build_include and not re.search(options.build_include, archive_name):
                        continue
                    if options.dry_run:
                        flush_print(archive_name)
                        continue
                    command = f"./chef.py -cbr -d {device_name} -t {platform} "
                    command += " ".join(args)
                    flush_print(f"Building {command}", with_border=True)
                    shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}")
                    try:
                        shell.run_cmd(command)
                    except RuntimeError as build_fail_error:
                        failed_builds.append((device_name, platform, "build"))
                        flush_print(str(build_fail_error))
                        if not options.keep_going:
                            exit(1)
                        continue
                    try:
                        bundle(platform, device_name)
                    except FileNotFoundError as bundle_fail_error:
                        failed_builds.append((device_name, platform, "bundle"))
                        flush_print(str(bundle_fail_error))
                        if not options.keep_going:
                            exit(1)
                        continue
                    os.makedirs(archive_prefix, exist_ok=True)
                    archive_full_name = archive_prefix + archive_name + archive_suffix
                    flush_print(
                        f"Adding build output to archive {archive_full_name}")
                    if os.path.exists(archive_full_name):
                        os.remove(archive_full_name)
                    with tarfile.open(archive_full_name, "w:gz") as tar:
                        tar.add(_CD_STAGING_DIR, arcname=".")
        if len(failed_builds) == 0:
            flush_print("No build failures", with_border=True)
        else:
            flush_print("Logging build failures", with_border=True)
            for failed_build in failed_builds:
                fail_log = f"""\
                Device: {failed_build[0]},
                Platform: {failed_build[1]},
                Phase: {failed_build[2]}"""
                flush_print(unwrap_cmd(fail_log))
        exit(0)

    #
    # Platform Folder
    #

    flush_print(f"Target is set to {options.sample_device_type_name}")
    global gen_dir
    gen_dir = (
        f"{_CHEF_SCRIPT_PATH}/out/{options.sample_device_type_name}/zap-generated/")

    flush_print("Setting up environment...")
    if options.build_target == "esp32":
        if config['esp32']['IDF_PATH'] is None:
            flush_print(
                'Path for esp32 SDK was not found. Make sure esp32.IDF_PATH is set on your config.yaml file')
            exit(1)
        shell.run_cmd(f'source {config["esp32"]["IDF_PATH"]}/export.sh')
    elif options.build_target == "nrfconnect":
        if config['nrfconnect']['ZEPHYR_BASE'] is None:
            flush_print(
                'The path for nrfconnect SDK was not found. Make sure nrfconnect.ZEPHYR_BASE is set on your config.yaml file. This is typically <NCS INSTALL PATH>/ncs/vX.X.X/zephyr')
            exit(1)
        if config['nrfconnect']['ZEPHYR_SDK_INSTALL_DIR'] is None:
            flush_print(
                'The path for nrfconnect toolchain was not found. Make sure nrfconnect.ZEPHYR_SDK_INSTALL_DIR is set on your config.yaml file. This is typically <NCS INSTALL PATH>/ncs/toolchains/vX.X.X/opt/zephyr-sdk')
            exit(1)
        zephyr_sdk_dir = config['nrfconnect']['ZEPHYR_SDK_INSTALL_DIR']
        shell.run_cmd("export ZEPHYR_TOOLCHAIN_VARIANT=zephyr")
        shell.run_cmd(f"export ZEPHYR_SDK_INSTALL_DIR={zephyr_sdk_dir}")
        shell.run_cmd(
            f"export ZEPHYR_BASE={config['nrfconnect']['ZEPHYR_BASE']}")
        shell.run_cmd(
            f'source {config["nrfconnect"]["ZEPHYR_BASE"]}/zephyr-env.sh')
        # QUIRK:
        # When the Zephyr SDK is installed as a part of the NCS toolchain, the build system will use
        # build tools from the NCS toolchain, but it will not update the PATH and LD_LIBRARY_PATH
        # and hence the build will fail. This ideally, should be handled automatically by the NCS
        # build system but until it is fixed, set the variables manually.
        ncs_toolchain_dir = os.path.abspath(f"{zephyr_sdk_dir}/../..")
        if os.path.exists(os.path.join(ncs_toolchain_dir, 'manifest.json')):
            shell.run_cmd(f"export PATH=$PATH:{ncs_toolchain_dir}/usr/bin")
            shell.run_cmd(
                f"export PATH=$PATH:{ncs_toolchain_dir}/usr/local/bin")
            shell.run_cmd(
                f"export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:{ncs_toolchain_dir}/usr/lib")
            shell.run_cmd(
                f"export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:{ncs_toolchain_dir}/usr/local/lib")
    elif options.build_target == "linux":
        pass
    elif options.build_target == "silabs-thread":
        flush_print('Path to gecko sdk is configured within Matter.')
        if 'SILABS_BOARD' not in config['silabs-thread'] or config['silabs-thread']['SILABS_BOARD'] is None:
            flush_print(
                'SILABS_BOARD was not configured. Make sure silabs-thread.SILABS_BOARD is set on your config.yaml file')
            exit(1)
        silabs_board = config['silabs-thread']['SILABS_BOARD']
    elif options.build_target == "ameba":
        if config['ameba']['AMEBA_SDK'] is None:
            flush_print(
                'Path for Ameba SDK was not found. Make sure AMEBA_SDK is set on your config.yaml file')
            exit(1)
        if config['ameba']['MATTER_SDK'] is None:
            flush_print(
                'Path for Matter SDK was not found. Make sure MATTER_SDK is set on your config.yaml file')
            exit(1)
        if (config['ameba']['MODEL'] != 'D' and config['ameba']['MODEL'] != 'Z2'):
            flush_print("Ameba Model is not recognized, please input D or Z2")
            exit(1)
    elif options.build_target == "telink":
        if config['telink']['ZEPHYR_BASE'] is None:
            flush_print(
                'Path for Telink SDK was not found. Make sure Telink_SDK is set on your config.yaml file')
            exit(1)
        if config['telink']['ZEPHYR_SDK_INSTALL_DIR'] is None:
            flush_print(
                'Path for Telink toolchain was not found. Make sure Telink toolchain is set on your config.yaml file')
            exit(1)
        shell.run_cmd("export ZEPHYR_TOOLCHAIN_VARIANT=zephyr")
        shell.run_cmd(
            f"export ZEPHYR_SDK_INSTALL_DIR={config['telink']['ZEPHYR_SDK_INSTALL_DIR']}")
        shell.run_cmd(
            f"export ZEPHYR_BASE={config['telink']['ZEPHYR_BASE']}")
        shell.run_cmd(
            f'source {config["telink"]["ZEPHYR_BASE"]}/zephyr-env.sh')
    else:
        flush_print(f"Target {options.build_target} not supported")

    shell.run_cmd(f"source {_REPO_BASE_PATH}/scripts/activate.sh")

    #
    # Toolchain update
    #

    if options.do_update_toolchain:
        if options.build_target == "esp32":
            flush_print("ESP32 toolchain update not supported. Skipping")
        elif options.build_target == "nrfconnect":
            flush_print("Updating toolchain")
            shell.run_cmd(
                f"cd {_REPO_BASE_PATH} && python3 scripts/setup/nrfconnect/update_ncs.py --update")
        elif options.build_target == "silabs-thread":
            flush_print("Silabs-thread toolchain not supported. Skipping")
        elif options.build_target == "linux":
            flush_print("Linux toolchain update not supported. Skipping")
        elif options.build_target == "Ameba":
            flush_print("Ameba toolchain update not supported. Skipping")
        elif options.build_target == "telink":
            flush_print("Telink toolchain update not supported. Skipping")
    #
    # Clean environment
    #
    if options.do_clean:
        if options.build_target == "esp32":
            shell.run_cmd(f"rm -f {_CHEF_SCRIPT_PATH}/esp32/sdkconfig")
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/esp32")
            shell.run_cmd(f"rm -rf {_CHEF_SCRIPT_PATH}/esp32/build")
            shell.run_cmd("idf.py fullclean")

    #
    # Cluster customization
    #

    if options.do_run_gui:
        flush_print("Starting ZAP GUI editor")
        shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/devices")
        shell.run_cmd(
            f"{_REPO_BASE_PATH}/scripts/tools/zap/run_zaptool.sh {options.sample_device_type_name}.zap")

    if options.do_run_zap:
        flush_print("Running ZAP script to generate artifacts")
        shell.run_cmd(f"rm -rf {gen_dir}")
        shell.run_cmd(f"mkdir -p {gen_dir}")
        shell.run_cmd(
            f"{_REPO_BASE_PATH}/scripts/tools/zap/generate.py "
            f"{_CHEF_SCRIPT_PATH}/devices/{options.sample_device_type_name}.zap -o {gen_dir}")

    #
    # Setup environment
    #
    if options.do_rpc:
        flush_print("RPC PW enabled")
        if options.build_target == "esp32":
            shell.run_cmd(
                f"export SDKCONFIG_DEFAULTS={_CHEF_SCRIPT_PATH}/esp32/sdkconfig_rpc.defaults")
            shell.run_cmd(
                f"[ -f {_CHEF_SCRIPT_PATH}/esp32/sdkconfig ] || cp "
                f"{_CHEF_SCRIPT_PATH}/esp32/sdkconfig_rpc.defaults {_CHEF_SCRIPT_PATH}/esp32/sdkconfig")
        else:
            flush_print(f"RPC PW on {options.build_target} not supported")

    else:
        flush_print("RPC PW disabled")
        if (options.build_target == "esp32"):
            shell.run_cmd(
                f"export SDKCONFIG_DEFAULTS={_CHEF_SCRIPT_PATH}/esp32/sdkconfig.defaults")
            shell.run_cmd(
                f"[ -f {_CHEF_SCRIPT_PATH}/esp32/sdkconfig ] || cp "
                f"{_CHEF_SCRIPT_PATH}/esp32/sdkconfig.defaults {_CHEF_SCRIPT_PATH}/esp32/sdkconfig")

    #
    # Menuconfig
    #

    if options.do_menuconfig:
        if options.build_target == "esp32":
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/esp32")
            shell.run_cmd("idf.py menuconfig")
        elif options.build_target == "nrfconnect":
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/nrfconnect")
            shell.run_cmd("west build -t menuconfig")
        elif (options.build_target == "silabs-thread") or (options.build_target == "silabs-wifi"):
            flush_print(
                "Menuconfig not available on Silabs-thread target. Skipping")
        elif options.build_target == "linux":
            flush_print("Menuconfig not available on Linux target. Skipping")
        elif options.build_target == "Ameba":
            flush_print("Menuconfig not available on Ameba target. Skipping")
        elif options.build_target == "telink":
            flush_print("Menuconfig not available on Telink target. Skipping")

    #
    # Build
    #

    if options.do_build:
        sw_ver_string = ""

        if options.do_automated_test_stamp:
            branch = ""
            for branch_text in shell.run_cmd("git branch", return_cmd_output=True).split("\n"):
                match_texts = re.findall(r"\* (.*)", branch_text)
                if match_texts:
                    branch = match_texts[0]
                    break
            commit_id = shell.run_cmd(
                "git rev-parse HEAD", return_cmd_output=True).replace("\n", "")
            sw_ver_string = f"""{branch}:{commit_id}"""
            # 64 bytes space could only contain 63 bytes string + 1 byte EOS.
            if len(sw_ver_string) >= 64:
                truncated_sw_ver_string = f"""{branch[:22]}:{commit_id}"""
                flush_print(
                    f"Truncate the software version string from \"{sw_ver_string}\" to "
                    f"\"{truncated_sw_ver_string}\" due to 64 bytes limitation")
                sw_ver_string = truncated_sw_ver_string

        flush_print(f"Software Version String: \"{sw_ver_string}\"")
        flush_print(
            f"Product ID 0x{options.pid:02X} / Vendor ID 0x{options.vid:02X}")
        flush_print("Building...")

        shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}")

        if options.build_target in "esp32 ameba telink".split():
            with open("project_include.cmake", "w") as f:
                f.write(textwrap.dedent(f"""\
                        set(CONFIG_DEVICE_VENDOR_ID {options.vid})
                        set(CONFIG_DEVICE_PRODUCT_ID {options.pid})
                        set(CONFIG_DEVICE_PRODUCT_NAME \"{options.pname}\")
                        set(CONFIG_ENABLE_PW_RPC {"1" if options.do_rpc else "0"})
                        set(SAMPLE_NAME {options.sample_device_type_name})
                        set(CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING \"{sw_ver_string}\")"""))

        if options.build_target == "esp32":
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/esp32")
            if options.enable_ipv4:
                if sys.platform == "darwin":
                    shell.run_cmd(
                        "sed -i '' 's/CONFIG_DISABLE_IPV4=y/#\\ CONFIG_DISABLE_IPV4\\ is\\ not\\ set/g' sdkconfig ")
                else:
                    shell.run_cmd(
                        "sed -i 's/CONFIG_DISABLE_IPV4=y/#\\ CONFIG_DISABLE_IPV4\\ is\\ not\\ set/g' sdkconfig ")
            else:
                if sys.platform == "darwin":
                    shell.run_cmd(
                        "sed -i '' 's/#\\ CONFIG_DISABLE_IPV4\\ is\\ not\\ set/CONFIG_DISABLE_IPV4=y/g' sdkconfig ")
                    shell.run_cmd(
                        "sed -i '' 's/CONFIG_LWIP_IPV4=y/#\\ CONFIG_LWIP_IPV4\\ is\\ not\\ set/g' sdkconfig ")
                else:
                    shell.run_cmd(
                        "sed -i 's/#\\ CONFIG_DISABLE_IPV4\\ is\\ not\\ set/CONFIG_DISABLE_IPV4=y/g' sdkconfig ")
                    shell.run_cmd(
                        "sed -i 's/CONFIG_LWIP_IPV4=y/#\\ CONFIG_LWIP_IPV4\\ is\\ not\\ set/g' sdkconfig ")

            shell.run_cmd("idf.py build")
            shell.run_cmd("idf.py build flashing_script")
            shell.run_cmd(
                f"(cd build/ && tar cJvf $(git rev-parse HEAD)-{options.sample_device_type_name}.tar.xz "
                f"--files-from=chip-shell.flashbundle.txt)")
            shell.run_cmd(
                f"cp build/$(git rev-parse HEAD)-{options.sample_device_type_name}.tar.xz {_CHEF_SCRIPT_PATH}")
        elif options.build_target == "nrfconnect":
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/nrfconnect")
            nrf_build_cmds = ["west build -b nrf52840dk/nrf52840"]
            if options.do_clean:
                nrf_build_cmds.append("-p always")
            nrf_build_cmds.append("--sysbuild")
            nrf_build_cmds.append("--")
            if options.do_rpc:
                nrf_build_cmds.append("-DOVERLAY_CONFIG=rpc.overlay")
            nrf_build_cmds.append(
                f"-DCONFIG_CHIP_DEVICE_VENDOR_ID={options.vid}")
            nrf_build_cmds.append(
                f"-DCONFIG_CHIP_DEVICE_PRODUCT_ID={options.pid}")
            nrf_build_cmds.append(
                f"-DCONFIG_CHIP_DEVICE_PRODUCT_NAME='\"{options.pname}\"'")
            nrf_build_cmds.append(
                f"-DCONFIG_CHEF_DEVICE_TYPE='\"{options.sample_device_type_name}\"'")
            nrf_build_cmds.append(
                f"-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING='\"{sw_ver_string}\"'")

            shell.run_cmd(" ".join(nrf_build_cmds))

        elif options.build_target == "silabs-thread":
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/silabs")
            if options.do_clean:
                shell.run_cmd(f"rm -rf out/{options.sample_device_type_name}")
            efr32_cmd_args = []
            efr32_cmd_args.append(
                f'{_REPO_BASE_PATH}/scripts/examples/gn_silabs_example.sh')
            efr32_cmd_args.append('./')
            efr32_cmd_args.append(f'out/{options.sample_device_type_name}')
            efr32_cmd_args.append(f'{silabs_board}')
            efr32_cmd_args.append(
                f'\'sample_name=\"{options.sample_device_type_name}\"\'')
            if sw_ver_string:
                efr32_cmd_args.append(
                    f'\'chip_device_config_device_software_version_string=\"{sw_ver_string}\"\'')
            efr32_cmd_args.append('enable_openthread_cli=true')
            if options.do_rpc:
                efr32_cmd_args.append('chip_build_libshell=false')
                efr32_cmd_args.append('\'import("//with_pw_rpc.gni")\'')
            else:
                efr32_cmd_args.append('chip_build_libshell=true')
            shell.run_cmd(" ".join(efr32_cmd_args))
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}")

        elif options.build_target == "ameba":
            if config['ameba']['MODEL'] == 'D':
                shell.run_cmd(
                    f"cd {config['ameba']['AMEBA_SDK']}/project/realtek_amebaD_va0_example/GCC-RELEASE")
                if options.do_clean:
                    shell.run_cmd("rm -rf out")
                shell.run_cmd(
                    f"./build.sh {config['ameba']['MATTER_SDK']} ninja "
                    f"{config['ameba']['AMEBA_SDK']}/project/realtek_amebaD_va0_example/GCC-RELEASE/out chef-app")
                shell.run_cmd("ninja -C out")
            elif config['ameba']['MODEL'] == 'Z2':
                shell.run_cmd(
                    f"cd {config['ameba']['AMEBA_SDK']}/project/realtek_amebaz2_v0_example/GCC-RELEASE")
                shell.run_cmd("rm -f project_include.mk")
                cmd = f"{config['ameba']['AMEBA_SDK']}/project/realtek_amebaz2_v0_example/GCC-RELEASE/project_include.mk"
                with open(cmd, "w") as f:
                    f.write(textwrap.dedent(f"""\
                        SAMPLE_NAME = {options.sample_device_type_name}
                        CHEF_FLAGS =
                        CHEF_FLAGS += -DCONFIG_DEVICE_VENDOR_ID={options.vid}
                        CHEF_FLAGS += -DCONFIG_DEVICE_PRODUCT_ID={options.pid}
                        CHEF_FLAGS += -DCHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING=\"{options.pid}\"
                        """
                                            ))
                if options.do_clean:
                    shell.run_cmd("make clean")
                shell.run_cmd("make chef")
                shell.run_cmd("make is")
        elif options.build_target == "telink":
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/telink")
            telink_build_cmds = ["west build -b tlsr9518adk80d"]
            if options.do_clean:
                telink_build_cmds.append("-p always")
            if options.do_rpc:
                telink_build_cmds.append("-- -DOVERLAY_CONFIG=rpc.overlay")
            shell.run_cmd(" ".join(telink_build_cmds))

        elif options.build_target == "linux":
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/linux")

            linux_args = []
            if options.do_rpc:
                linux_args.append('import("//with_pw_rpc.gni")')
            linux_args.extend([
                'import("//build_overrides/chip.gni")',
                'import("${chip_root}/config/standalone/args.gni")',
                'chip_shell_cmd_server = false',
                'chip_build_libshell = true',
                'chip_enable_openthread = false',
                'chip_generate_link_map_file = true',
                'chip_config_network_layer_ble = false',
                'chip_device_project_config_include = "<CHIPProjectAppConfig.h>"',
                'chip_project_config_include = "<CHIPProjectAppConfig.h>"',
                'chip_system_project_config_include = "<SystemProjectConfig.h>"',
                'chip_project_config_include_dirs = [ "${chip_root}/examples/chef/linux/include" ]',
                'chip_project_config_include_dirs += [ "${chip_root}/config/standalone" ]',
                (f'target_defines = ["CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID={options.vid}", '
                 f'"CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID={options.pid}", '
                 f'"CONFIG_ENABLE_PW_RPC={int(options.do_rpc)}", '
                 f'"CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME=\\"{str(options.pname)}\\""]'),
            ])

            uname_resp = shell.run_cmd("uname -m", return_cmd_output=True)
            if "aarch" not in uname_resp and "arm" not in uname_resp:
                if options.cpu_type == "arm64":
                    if "SYSROOT_AARCH64" not in shell.env:
                        flush_print(
                            "SYSROOT_AARCH64 env variable not set. "
                            "AARCH64 toolchain needed for cross-compiling for arm64.")
                        exit(1)
                    shell.env["PKG_CONFIG_PATH"] = (
                        f'{shell.env["SYSROOT_AARCH64"]}/lib/aarch64-linux-gnu/pkgconfig')
                    linux_args.append('target_cpu="arm64"')
                    linux_args.append('is_clang=true')
                    linux_args.append('chip_crypto="mbedtls"')
                    linux_args.append(
                        f'sysroot="{shell.env["SYSROOT_AARCH64"]}"')

                elif options.cpu_type == "arm":
                    if "SYSROOT_ARMHF" not in shell.env:
                        flush_print(
                            "SYSROOT_ARMHF env variable not set. "
                            "ARMHF toolchain needed for cross-compiling for arm.")
                        exit(1)
                    shell.env["PKG_CONFIG_PATH"] = (
                        f'{shell.env["SYSROOT_ARMHF"]}/lib/arm-linux-gnueabihf/pkgconfig')
                    linux_args.append('target_cpu="arm"')
                    linux_args.append('is_clang=true')
                    linux_args.append('chip_crypto="mbedtls"')
                    linux_args.append(
                        f'sysroot="{shell.env["SYSROOT_ARMHF"]}"')

            if options.cpu_type == "x64":
                uname_resp = shell.run_cmd("uname -m", return_cmd_output=True)
                if "x64" not in uname_resp and "x86_64" not in uname_resp:
                    flush_print(
                        f"Unable to cross compile for x64 on {uname_resp}")
                    exit(1)
            if options.enable_ipv4:
                linux_args.append("chip_inet_config_enable_ipv4=true")
            else:
                linux_args.append("chip_inet_config_enable_ipv4=false")

            if options.enable_lit_icd:
                linux_args.append("chip_enable_icd_server = true")
                linux_args.append("chip_icd_report_on_active_mode = true")
                linux_args.append("chip_enable_icd_lit = true")
                linux_args.append("chip_enable_icd_dsls = true")
                if options.icd_subscription_resumption:
                    options.icd_persist_subscription = True
                    linux_args.append("chip_subscription_timeout_resumption = true")
                if options.icd_persist_subscription:
                    linux_args.append("chip_persist_subscriptions = true")

            if sw_ver_string:
                linux_args.append(
                    f'chip_device_config_device_software_version_string = "{sw_ver_string}"')

            with open(f"{_CHEF_SCRIPT_PATH}/linux/args.gni", "w") as f:
                f.write("\n".join(linux_args))
            with open(f"{_CHEF_SCRIPT_PATH}/linux/sample.gni", "w") as f:
                f.write(textwrap.dedent(f"""\
                        sample_zap_file = "{options.sample_device_type_name}.zap"
                        sample_name = "{options.sample_device_type_name}"
                        """))
            if options.do_clean:
                shell.run_cmd("rm -rf out")
            shell.run_cmd("gn gen --add-export-compile-commands=\"*\" out")
            shell.run_cmd("ninja -C out")

    #
    # Compilation DB TODO
    #

    #
    # Flash
    #

    if options.do_flash:
        flush_print("Flashing target")
        if options.build_target == "esp32":
            if config['esp32']['TTY'] is None:
                flush_print(
                    'The path for the serial enumeration for esp32 is not set. Make sure esp32.TTY is set on your config.yaml file')
                exit(1)
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/esp32")
            if options.do_erase:
                shell.run_cmd(
                    f"idf.py -p {config['esp32']['TTY']} erase-flash")
            shell.run_cmd(f"idf.py -p {config['esp32']['TTY']} flash")
        elif options.build_target == "nrfconnect":
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/nrfconnect")
            if options.do_erase:
                shell.run_cmd("west flash --erase")
            else:
                shell.run_cmd("west flash")
        elif (options.build_target == "silabs-thread") or (options.build_target == "silabs-wifi"):
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/efr32")
            shell.run_cmd(
                f"python3 out/{options.sample_device_type_name}/{silabs_board}/matter-silabs-chef-example.flash.py")

            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}")
        elif (options.build_target == "ameba"):
            if config['ameba']['MODEL'] == 'D':
                shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/ameba")
                shell.run_cmd(
                    f"cd {config['ameba']['AMEBA_SDK']}/tools/AmebaD/Image_Tool_Linux")
                shell.run_cmd((
                    f"{config['ameba']['AMEBA_SDK']}/tools/AmebaD/Image_Tool_Linux/flash.sh "
                    f"{config['ameba']['TTY']} {config['ameba']['AMEBA_SDK']}"
                    f"/project/realtek_amebaD_va0_example/GCC-RELEASE/out"
                ), raise_on_returncode=False)
            else:
                shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/ameba")
                shell.run_cmd(
                    f"cd {config['ameba']['AMEBA_SDK']}/tools/AmebaZ2/Image_Tool_Linux")
                shell.run_cmd((
                    f"{config['ameba']['AMEBA_SDK']}/tools/AmebaZ2/Image_Tool_Linux/flash.sh "
                    f"{config['ameba']['TTY']} {config['ameba']['AMEBA_SDK']}"
                    f"/project/realtek_amebaz2_v0_example/GCC-RELEASE/application_is/Debug/bin"
                ), raise_on_returncode=False)

    #
    # Terminal interaction
    #

    if options.do_interact:
        flush_print("Starting terminal...")
        if options.build_target == "esp32":
            if config['esp32']['TTY'] is None:
                flush_print(
                    'The path for the serial enumeration for esp32 is not set. '
                    'Make sure esp32.TTY is set on your config.yaml file')
                exit(1)
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/esp32")
            shell.run_cmd(f"idf.py -p {config['esp32']['TTY']} monitor")
        elif options.build_target == "nrfconnect":
            if config['nrfconnect']['TTY'] is None:
                flush_print(
                    'The path for the serial enumeration for nordic is not set. '
                    'Make sure nrfconnect.TTY is set on your config.yaml file')
                exit(1)
            shell.run_cmd("killall screen")
            shell.run_cmd(f"screen {config['nrfconnect']['TTY']} 115200")
        elif (options.build_target == "silabs-thread"):
            if config['silabs-thread']['TTY'] is None:
                flush_print(
                    'The path for the serial enumeration for silabs-thread is not set. '
                    'Make sure silabs-thread.TTY is set on your config.yaml file')
                exit(1)

            shell.run_cmd("killall screen")
            shell.run_cmd(
                f"screen {config['silabs-thread']['TTY']} 115200 8-N-1")
        elif options.build_target == "linux":
            flush_print(
                f"{_CHEF_SCRIPT_PATH}/linux/out/{options.sample_device_type_name}")
            shell.run_cmd(
                f"{_CHEF_SCRIPT_PATH}/linux/out/{options.sample_device_type_name}")
        elif options.build_target == "ameba":
            if config['ameba']['TTY'] is None:
                flush_print(
                    'The path for the serial enumeration for ameba is not set. Make sure ameba.TTY is set on your config.yaml file')
                exit(1)
            if config['ameba']['MODEL'] == 'D':
                shell.run_cmd("killall screen")
                shell.run_cmd(f"screen {config['ameba']['TTY']} 115200")
            else:
                flush_print("Ameba Z2 image has not been flashed yet")
        elif options.build_target == "telink":
            if config['telink']['TTY'] is None:
                flush_print(
                    'The path for the serial enumeration for telink is not set. '
                    'Make sure telink.TTY is set on your config.yaml file')
                exit(1)
            shell.run_cmd("killall screen")
            shell.run_cmd(f"screen {config['telink']['TTY']} 115200")
    #
    # RPC Console
    #
    if options.do_rpc_console:
        if options.build_target == "esp32":
            shell.run_cmd(
                f"python3 -m chip_rpc.console --device {config['esp32']['TTY']}")
        elif (options.build_target == "silabs-thread"):
            if (sys.platform == "linux") or (sys.platform == "linux2"):
                if (config['silabs-thread']['TTY'] is None):
                    flush_print(
                        'The path for the serial enumeration for silabs-thread is not set. '
                        'Make sure silabs-thread.TTY is set on your config.yaml file')
                    exit(1)
                shell.run_cmd(
                    f"python3 -m chip_rpc.console --device {config['silabs-thread']['TTY']} -b 115200")
            elif sys.platform == "darwin":
                if (config['silabs-thread']['CU'] is None):
                    flush_print(
                        'The path for the serial enumeration for silabs-thread is not set. '
                        'Make sure silabs-thread.CU is set on your config.yaml file')
                    exit(1)
                shell.run_cmd(
                    f"python3 -m chip_rpc.console --device {config['silabs-thread']['CU']} -b 115200")

    flush_print("Done")
    return 0


if __name__ == '__main__':
    sys.exit(main())
