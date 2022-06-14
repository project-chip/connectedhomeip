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

import hashlib
import json
import optparse
import os
import shutil
import sys
import tarfile
import textwrap
from typing import Any, Dict, Sequence

import yaml

import constants
import stateful_shell

TermColors = constants.TermColors

shell = stateful_shell.StatefulShell()

_CHEF_SCRIPT_PATH = os.path.abspath(os.path.dirname(__file__))
_REPO_BASE_PATH = os.path.join(_CHEF_SCRIPT_PATH, "../../")
_DEVICE_FOLDER = os.path.join(_CHEF_SCRIPT_PATH, "devices")
_DEVICE_LIST = [file[:-4] for file in os.listdir(_DEVICE_FOLDER) if file.endswith(".zap")]
_CHEF_ZZZ_ROOT = os.path.join(_CHEF_SCRIPT_PATH, "zzz_generated")
_CI_DEVICE_MANIFEST_NAME = "INPUTMD5.txt"
_CI_ZAP_MANIFEST_NAME = "ZAPSHA.txt"
_CICD_CONFIG_FILE_NAME = os.path.join(_CHEF_SCRIPT_PATH, "cicd_meta.json")
_CI_ALLOW_LIST = ["lighting-app"]

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
        config["nrfconnect"]["TTY"] = None
        config["esp32"]["IDF_PATH"] = os.environ.get('IDF_PATH')
        config["esp32"]["TTY"] = None
        config["silabs-thread"]["GECKO_SDK"] = f"{_REPO_BASE_PATH}third_party/efr32_sdk/repo"
        config["silabs-thread"]["TTY"] = None
        config["silabs-thread"]["CU"] = None

        flush_print(yaml.dump(config))
        yaml.dump(config, configStream)
        configStream.close()

    return config


def check_python_version() -> None:
    if sys.version_info[0] < 3:
        flush_print('Must use Python 3. Current version is ' +
                    str(sys.version_info[0]))
        exit(1)


def check_zap() -> str:
    """Produces SHA of ZAP submodule for current HEAD.

    Returns:
      SHA of zap submodule.
    """
    shell.run_cmd(f"cd {_REPO_BASE_PATH}")
    branch = shell.run_cmd("git rev-parse --abbrev-ref HEAD",
                           return_cmd_output=True)
    branch = branch.replace("\n", "")
    command = f"git ls-tree {branch} third_party/zap/repo"
    zap_commit = shell.run_cmd(command, return_cmd_output=True)
    zap_commit = zap_commit.split(" ")[2]
    zap_commit = zap_commit[:zap_commit.index("\t")]
    flush_print(f"Found zap commit: {zap_commit}")
    return zap_commit


def generate_device_manifest(
        write_manifest_file: bool = False) -> Dict[str, Any]:
    """Produces dictionary containing md5 of device dir zap files.

    Args:
        write_manifest_file: Serialize manifest in tree.
    Returns:
        Dict containing MD5 of device dir zap files.
    """
    ci_manifest = {"devices": {}}
    devices_manifest = ci_manifest["devices"]
    zap_sha = check_zap()
    ci_manifest["zap_commit"] = zap_sha
    for device_name in _DEVICE_LIST:
        device_file_path = os.path.join(_DEVICE_FOLDER, device_name + ".zap")
        with open(device_file_path, "rb") as device_file:
            device_file_data = device_file.read()
        device_file_md5 = hashlib.md5(device_file_data).hexdigest()
        devices_manifest[device_name] = device_file_md5
        flush_print(f"Current digest for {device_name} : {device_file_md5}")
        if write_manifest_file:
            device_zzz_dir = os.path.join(_CHEF_ZZZ_ROOT, device_name)
            device_zzz_md5_file = os.path.join(device_zzz_dir, _CI_DEVICE_MANIFEST_NAME)
            with open(device_zzz_md5_file, "w+") as md5_file:
                md5_file.write(device_file_md5)
            device_zzz_zap_sha_file = os.path.join(device_zzz_dir, _CI_ZAP_MANIFEST_NAME)
            with open(device_zzz_zap_sha_file, "w+") as zap_sha_file:
                zap_sha_file.write(zap_sha)
    return ci_manifest


def load_cicd_config() -> Dict[str, Any]:
    with open(_CICD_CONFIG_FILE_NAME) as config_file:
        config = json.loads(config_file.read())
    for platform_name, platform_config in config.items():
        has_build_dir = "build_dir" in platform_config
        has_plat_label = "platform_label" in platform_config
        if not has_build_dir or not has_plat_label:
            flush_print(f"{platform_name} CICD config missing build_dir or platform_label")
            exit(1)
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
        border = ('-' * 64) + '\n'
        to_print = f"{border}{to_print}\n{border}"
    print(to_print, flush=True)


def main(argv: Sequence[str]) -> None:
    check_python_version()
    config = load_config()
    cicd_config = load_cicd_config()

    #
    # Build environment switches
    #

    if sys.platform == "win32":
        flush_print('Windows is currently not supported. Use Linux or MacOS platforms')
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
            silabs-thread

        Device Types:
            {deviceTypes}

        Notes:
        - Whenever you change a device type, make sure to also use options -zbe
        - Be careful if you have more than one device connected. The script assumes you have only one device connected and might flash the wrong one\
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
    parser.add_option("-m", "--menuconfig", help="runs menuconfig on platforms that support it",
                      action="store_true", dest="do_menuconfig")
    parser.add_option("", "--bootstrap_zap", help="installs zap dependencies",
                      action="store_true", dest="do_bootstrap_zap")
    parser.add_option("-z", "--zap", help="runs zap to generate data model & interaction model artifacts",
                      action="store_true", dest="do_run_zap")
    parser.add_option("-g", "--zapgui", help="runs zap GUI display to allow editing of data model",
                      action="store_true", dest="do_run_gui")
    parser.add_option("-d", "--device", dest="sample_device_type_name",
                      help="specifies device type. Default is lighting. See info above for supported device types", metavar="TARGET", default="lighting")
    parser.add_option("-t", "--target", type='choice',
                      action='store',
                      dest="build_target",
                      help="specifies target platform. Default is esp32. See info below for currently supported target platforms",
                      choices=['nrfconnect', 'esp32', 'linux', 'silabs-thread'],
                      metavar="TARGET",
                      default="esp32")
    parser.add_option("-r", "--rpc", help="enables Pigweed RPC interface. Enabling RPC disables the shell interface. Your sdkconfig configurations will be reverted to default. Default is PW RPC off. When enabling or disabling this flag, on the first build force a clean build with -c", action="store_true", dest="do_rpc")
    parser.add_option("-v", "--vid", dest="vid", type=int,
                      help="specifies the Vendor ID. Default is 0xFFF1", metavar="VID", default=0xFFF1)
    parser.add_option("-p", "--pid", dest="pid", type=int,
                      help="specifies the Product ID. Default is 0x8000", metavar="PID", default=0x8000)
    parser.add_option("", "--rpc_console", help="Opens PW RPC Console",
                      action="store_true", dest="do_rpc_console")
    parser.add_option("-y", "--tty", help="Enumerated USB tty/serial interface enumerated for your physical device. E.g.: /dev/ACM0",
                      dest="tty", metavar="TTY", default=None)
    parser.add_option("", "--generate_zzz", help="Populates zzz_generated/chef/<DEVICE_TYPE>/zap-generated with output of ZAP tool for every device in examples/chef/devices. If this flag is set, all other arguments are ignored except for --bootstrap_zap and --validate_zzz.",
                      dest="generate_zzz", action="store_true")
    parser.add_option("", "--validate_zzz", help="Checks if cached ZAP output needs to be regenrated, for use in CI. If this flag is set, all other arguments are ignored.",
                      dest="validate_zzz", action="store_true")
    parser.add_option("", "--use_zzz", help="Use pre generated output from the ZAP tool found in the zzz_generated folder. Used to decrease execution time of CI/CD jobs",
                      dest="use_zzz", action="store_true")
    parser.add_option("", "--build_all", help="For use in CD only. Builds and bundles all chef examples for the specified platform. Uses --use_zzz. Chef exits after completion.",
                      dest="build_all", action="store_true")
    parser.add_option(
        "", "--ci", help="Builds Chef examples defined in _CI_ALLOW_LIST. Uses --use_zzz. Uses specified target from -t. Chef exits after completion.", dest="ci", action="store_true")

    options, _ = parser.parse_args(argv)

    splash()

    #
    # Validate zzz_generated
    #

    if options.validate_zzz:
        flush_print(f"Validating\n{_CHEF_ZZZ_ROOT}\n",
                    with_border=True)
        fix_instructions = textwrap.dedent("""\
        Cached files out of date!
        Please:
          ./scripts/bootstrap.sh
          source ./scripts/activate.sh
          cd ./third_party/zap/repo
          npm install
          cd ../../..
          ./examples/chef/chef.py --generate_zzz
          git add examples/chef/zzz_generated
        Ensure you are running with the latest version of ZAP from master!""")
        ci_manifest = generate_device_manifest()
        current_zap = ci_manifest["zap_commit"]
        for device, device_md5 in ci_manifest["devices"].items():
            zzz_dir = os.path.join(_CHEF_ZZZ_ROOT, device)
            device_zap_sha_file = os.path.join(zzz_dir, _CI_ZAP_MANIFEST_NAME)
            device_md5_file = os.path.join(zzz_dir, _CI_DEVICE_MANIFEST_NAME)
            help_msg = f"{device}: {fix_instructions}"
            if not os.path.exists(device_zap_sha_file):
                flush_print(f"ZAP VERSION MISSING {help_msg}")
                exit(1)
            else:
                with open(device_zap_sha_file) as zap_file:
                    output_cached_zap_sha = zap_file.read()
                if output_cached_zap_sha != current_zap:
                    flush_print(f"ZAP VERSION MISMATCH {help_msg}")
                    exit(1)
            if not os.path.exists(device_md5_file):
                flush_print(f"INPUT MD5 MISSING {help_msg}")
                exit(1)
            else:
                with open(device_md5_file) as md5_file:
                    output_cached_md5 = md5_file.read()
                if output_cached_md5 != device_md5:
                    flush_print(f"INPUT MD5 MISMATCH {help_msg}")
                    exit(1)
        flush_print("Cached ZAP output is up to date!")
        exit(0)

    #
    # ZAP bootstrapping
    #

    if options.do_bootstrap_zap:
        if sys.platform == "linux" or sys.platform == "linux2":
            flush_print("Installing ZAP OS package dependencies")
            install_deps_cmd = textwrap.dedent("""\
            sudo apt-get install node node-yargs npm
            libpixman-1-dev libcairo2-dev libpango1.0-dev node-pre-gyp
            libjpeg9-dev libgif-dev node-typescript""")
            install_deps_cmd = install_deps_cmd.replace("\n", " ")
            shell.run_cmd(install_deps_cmd)
        if sys.platform == "darwin":
            flush_print("Installation of ZAP OS packages not supported on MacOS")
        if sys.platform == "win32":
            flush_print(
                "Installation of ZAP OS packages not supported on Windows")

        flush_print("Running NPM to install ZAP Node.JS dependencies")
        shell.run_cmd(
            f"cd {_REPO_BASE_PATH}/third_party/zap/repo/ && npm install")

    #
    # Populate zzz_generated
    #

    if options.generate_zzz:
        flush_print(f"Cleaning {_CHEF_ZZZ_ROOT}")
        if not os.path.exists(_CHEF_ZZZ_ROOT):
            flush_print(f"{_CHEF_ZZZ_ROOT} doesn't exist; creating")
            os.mkdir(_CHEF_ZZZ_ROOT)
        else:
            flush_print(f"Deleting and recreating existing {_CHEF_ZZZ_ROOT}")
            shutil.rmtree(_CHEF_ZZZ_ROOT)
            os.mkdir(_CHEF_ZZZ_ROOT)
        flush_print(f"Generating files in {_CHEF_ZZZ_ROOT} for all devices")
        for device_name in _DEVICE_LIST:
            flush_print(f"Generating files for {device_name}")
            device_out_dir = os.path.join(_CHEF_ZZZ_ROOT,
                                          device_name,
                                          "zap-generated")
            os.makedirs(device_out_dir)
            shell.run_cmd(textwrap.dedent(f"""\
            {_REPO_BASE_PATH}/scripts/tools/zap/generate.py \
            {_CHEF_SCRIPT_PATH}/devices/{device_name}.zap -o {device_out_dir}"""))
            shell.run_cmd(f"touch {device_out_dir}/af-gen-event.h")
        generate_device_manifest(write_manifest_file=True)
        exit(0)

    #
    # CI
    #

    if options.ci:
        for device_name in [d for d in _DEVICE_LIST if d in _CI_ALLOW_LIST]:
            if options.build_target == "nrfconnect":
                shell.run_cmd("export GNUARMEMB_TOOLCHAIN_PATH=\"$PW_ARM_CIPD_INSTALL_DIR\"")
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}")
            command = f"./chef.py -cbr --use_zzz -d {device_name} -t {options.build_target}"
            flush_print(f"Building {command}", with_border=True)
            shell.run_cmd(command)
            # TODO call per-platform bundle function for extra validation
        exit(0)

    #
    # Build all
    #

    if options.build_all:
        flush_print("Building all chef examples")
        archive_prefix = "/workspace/artifacts/"
        archive_suffix = ".tar.gz"
        os.makedirs(archive_prefix, exist_ok=True)
        for device_name in _DEVICE_LIST:
            for platform, platform_meta in cicd_config.items():
                directory = platform_meta['build_dir']
                label = platform_meta['platform_label']
                output_dir = os.path.join(_CHEF_SCRIPT_PATH, directory)
                command = f"./chef.py -cbr --use_zzz -d {device_name} -t {platform}"
                flush_print(f"Building {command}", with_border=True)
                shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}")
                shell.run_cmd("export GNUARMEMB_TOOLCHAIN_PATH=\"$PW_ARM_CIPD_INSTALL_DIR\"")
                shell.run_cmd(command)
                # TODO Needs to call per-platform bundle function
                archive_name = f"{label}-{device_name}"
                archive_full_name = archive_prefix + archive_name + archive_suffix
                flush_print(f"Adding build output to archive {archive_full_name}")
                with tarfile.open(archive_full_name, "w:gz") as tar:
                    tar.add(output_dir, arcname=".")
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
            flush_print('Path for esp32 SDK was not found. Make sure esp32.IDF_PATH is set on your config.yaml file')
            exit(1)
        plat_folder = os.path.normpath(f"{_CHEF_SCRIPT_PATH}/esp32")
        shell.run_cmd(f'source {config["esp32"]["IDF_PATH"]}/export.sh')
    elif options.build_target == "nrfconnect":
        if config['nrfconnect']['ZEPHYR_BASE'] is None:
            flush_print('Path for nrfconnect SDK was not found. Make sure nrfconnect.ZEPHYR_BASE is set on your config.yaml file')
            exit(1)
        plat_folder = os.path.normpath(f"{_CHEF_SCRIPT_PATH}/nrfconnect")
        shell.run_cmd(f'source {config["nrfconnect"]["ZEPHYR_BASE"]}/zephyr-env.sh')
        shell.run_cmd("export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb")
    elif options.build_target == "linux":
        pass
    elif options.build_target == "silabs-thread":
        flush_print('Path to gecko sdk is configured within Matter.')
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
            f"{_REPO_BASE_PATH}/scripts/tools/zap/generate.py {_CHEF_SCRIPT_PATH}/devices/{options.sample_device_type_name}.zap -o {gen_dir}")
        # af-gen-event.h is not generated
        shell.run_cmd(f"touch {gen_dir}/af-gen-event.h")

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
            flush_print("Menuconfig not available on Silabs-thread target. Skipping")
        elif options.build_target == "linux":
            flush_print("Menuconfig not available on Linux target. Skipping")

    #
    # Build
    #

    if options.do_build:
        if options.use_zzz:
            flush_print("Using pre-generated ZAP output")
            zzz_dir = os.path.join(_CHEF_SCRIPT_PATH,
                                   "zzz_generated",
                                   options.sample_device_type_name,
                                   "zap-generated")
            if not os.path.exists(zzz_dir):
                flush_print(textwrap.dedent(f"""\
                You have specified --use_zzz
                for device {options.sample_device_type_name}
                which does not exist in the cached ZAP output.
                To cache ZAP output for this device:
                ensure {options.sample_device_type_name}.zap
                is placed in {_DEVICE_FOLDER}
                run chef with the option --generate_zzz
                """))
                exit(1)
            shutil.rmtree(gen_dir, ignore_errors=True)
            shutil.copytree(zzz_dir, gen_dir)
        flush_print("Building...")
        if options.do_rpc:
            flush_print("RPC PW enabled")
            if options.build_target == "esp32":
                shell.run_cmd(
                    f"export SDKCONFIG_DEFAULTS={_CHEF_SCRIPT_PATH}/esp32/sdkconfig_rpc.defaults")
            else:
                flush_print(f"RPC PW on {options.build_target} not supported")

        else:
            flush_print("RPC PW disabled")
        if (options.build_target == "esp32"):
            shell.run_cmd(
                f"export SDKCONFIG_DEFAULTS={_CHEF_SCRIPT_PATH}/esp32/sdkconfig.defaults")

        flush_print(
            f"Product ID 0x{options.pid:02X} / Vendor ID 0x{options.vid:02X}")
        shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}")

        if (options.build_target == "esp32") or (options.build_target == "nrfconnect"):
            with open("project_include.cmake", "w") as f:
                f.write(textwrap.dedent(f"""\
                        set(CONFIG_DEVICE_VENDOR_ID {options.vid})
                        set(CONFIG_DEVICE_PRODUCT_ID {options.pid})
                        set(CONFIG_ENABLE_PW_RPC {"1" if options.do_rpc else "0"})
                        set(SAMPLE_NAME {options.sample_device_type_name})"""))

        if options.build_target == "esp32":
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/esp32")
            if options.do_clean:
                shell.run_cmd(f"rm -f {_CHEF_SCRIPT_PATH}/esp32/sdkconfig")
                shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/esp32")
                shell.run_cmd(f"rm -rf {_CHEF_SCRIPT_PATH}/esp32/build")
                shell.run_cmd("idf.py fullclean")
            shell.run_cmd("idf.py build")
        elif options.build_target == "nrfconnect":
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/nrfconnect")
            nrf_build_cmds = ["west build -b nrf52840dk_nrf52840"]
            if options.do_clean:
                nrf_build_cmds.append("-p always")
            if options.do_rpc:
                nrf_build_cmds.append("-- -DOVERLAY_CONFIG=rpc.overlay")
            shell.run_cmd(" ".join(nrf_build_cmds))

        elif options.build_target == "silabs-thread":
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/efr32")
            if options.do_clean:
                shell.run_cmd(f"rm -rf out/{options.sample_device_type_name}")
            shell.run_cmd(
                f"""{_REPO_BASE_PATH}/scripts/examples/gn_efr32_example.sh ./ out/{options.sample_device_type_name} BRD4186A \'sample_name=\"{options.sample_device_type_name}\"\' enable_openthread_cli=true chip_build_libshell=true \'{'import("//with_pw_rpc.gni")' if options.do_rpc else ""}\'""")
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}")

        elif options.build_target == "linux":
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/linux")
            with open(f"{_CHEF_SCRIPT_PATH}/linux/args.gni", "w") as f:
                f.write(textwrap.dedent(f"""\
                        import("//build_overrides/chip.gni")
                        import("${{chip_root}}/config/standalone/args.gni")
                        chip_shell_cmd_server = false
                        target_defines = ["CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID={options.vid}", "CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID={options.pid}", "CONFIG_ENABLE_PW_RPC={'1' if options.do_rpc else '0'}"]
                        """))
            with open(f"{_CHEF_SCRIPT_PATH}/linux/sample.gni", "w") as f:
                f.write(textwrap.dedent(f"""\
                        sample_zap_file = "{options.sample_device_type_name}.zap"
                        sample_name = "{options.sample_device_type_name}"
                        """))
            if options.do_clean:
                shell.run_cmd(f"rm -rf out")
            if options.do_rpc:
                shell.run_cmd("gn gen out --args='import(\"//with_pw_rpc.gni\")'")
            else:
                shell.run_cmd("gn gen out --args=''")
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
                flush_print('The path for the serial enumeration for esp32 is not set. Make sure esp32.TTY is set on your config.yaml file')
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
            shell.run_cmd(f"python3 out/{options.sample_device_type_name}/BRD4186A/chip-efr32-chef-example.flash.py")

            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}")

    #
    # Terminal interaction
    #

    if options.do_interact:
        flush_print("Starting terminal...")
        if options.build_target == "esp32":
            if config['esp32']['TTY'] is None:
                flush_print('The path for the serial enumeration for esp32 is not set. Make sure esp32.TTY is set on your config.yaml file')
                exit(1)
            shell.run_cmd(f"cd {_CHEF_SCRIPT_PATH}/esp32")
            shell.run_cmd(f"idf.py -p {config['esp32']['TTY']} monitor")
        elif options.build_target == "nrfconnect":
            if config['nrfconnect']['TTY'] is None:
                flush_print('The path for the serial enumeration for nordic is not set. Make sure nrfconnect.TTY is set on your config.yaml file')
                exit(1)
            shell.run_cmd("killall screen")
            shell.run_cmd(f"screen {config['nrfconnect']['TTY']} 115200")
        elif (options.build_target == "silabs-thread"):
            if config['silabs-thread']['TTY'] is None:
                flush_print(
                    'The path for the serial enumeration for silabs-thread is not set. Make sure silabs-thread.TTY is set on your config.yaml file')
                exit(1)

            shell.run_cmd("killall screen")
            shell.run_cmd(f"screen {config['silabs-thread']['TTY']} 115200 8-N-1")
        elif options.build_target == "linux":
            flush_print(
                f"{_CHEF_SCRIPT_PATH}/linux/out/{options.sample_device_type_name}")
            shell.run_cmd(
                f"{_CHEF_SCRIPT_PATH}/linux/out/{options.sample_device_type_name}")

    #
    # RPC Console
    #
    if options.do_rpc_console:
        if options.build_target == "esp32":
            shell.run_cmd(
                f"python3 -m chip_rpc.console --device {config['esp32']['TTY']}")
        elif (options.build_target == "silabs-thread"):
            if (sys.platform == "linux") or (sys.platform == "linux2"):
                if(config['silabs-thread']['TTY'] is None):
                    flush_print(
                        'The path for the serial enumeration for silabs-thread is not set. Make sure silabs-thread.TTY is set on your config.yaml file')
                    exit(1)
                shell.run_cmd(f"python3 -m chip_rpc.console --device {config['silabs-thread']['TTY']} -b 115200")
            elif sys.platform == "darwin":
                if(config['silabs-thread']['CU'] is None):
                    flush_print(
                        'The path for the serial enumeration for silabs-thread is not set. Make sure silabs-thread.CU is set on your config.yaml file')
                    exit(1)
                shell.run_cmd(f"python3 -m chip_rpc.console --device {config['silabs-thread']['CU']} -b 115200")

    flush_print("Done")


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
