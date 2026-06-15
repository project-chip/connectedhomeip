# Copyright (c) 2021 Project CHIP Authors
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

import logging
import os
import re
import shutil
import time
from pathlib import Path
from enum import Enum, auto

from runner.runner import Runner

from .builder import Builder, BuilderOutput, OutDirLock, lock_output_dir
from .gn import GnBuilder

log = logging.getLogger(__name__)


class BouffalolabApp(Enum):
    LIGHT = auto()
    CONTACT = auto()

    def ExampleName(self):
        if self == BouffalolabApp.LIGHT:
            return 'lighting-app'
        if self == BouffalolabApp.CONTACT:
            return 'contact-sensor-app'
        raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self, chip_name):
        if self == BouffalolabApp.LIGHT:
            return ('chip-%s-lighting-example' % chip_name)
        if self == BouffalolabApp.CONTACT:
            return ('chip-%s-contact-sensor-example' % chip_name)
        raise Exception('Unknown app type: %r' % self)


class BouffalolabBoard(Enum):
    BL602DK = auto()
    BL616DK = auto()
    BL616CLDK = auto()
    BL704LDK = auto()
    BL706DK = auto()
    BL602_NIGHT_LIGHT = auto()
    BL706_NIGHT_LIGHT = auto()

    def GnArgName(self):

        if self == BouffalolabBoard.BL602DK:
            return 'BL602DK'
        if self == BouffalolabBoard.BL616DK:
            return 'BL616DK'
        if self == BouffalolabBoard.BL616CLDK:
            return 'BL616CLDK'
        if self == BouffalolabBoard.BL704LDK:
            return 'BL704LDK'
        if self == BouffalolabBoard.BL706DK:
            return 'BL706DK'
        if self == BouffalolabBoard.BL602_NIGHT_LIGHT:
            return 'BL602-NIGHT-LIGHT'
        if self == BouffalolabBoard.BL706_NIGHT_LIGHT:
            return 'BL706-NIGHT-LIGHT'
        raise Exception('Unknown board #: %r' % self)


class BouffalolabThreadType(Enum):
    NONE = auto()
    THREAD_FTD = auto()
    THREAD_MTD = auto()


def _AutoOtaEnabled():
    value = os.environ.get("BOUFFALOLAB_AUTO_OTA", "1").strip().lower()
    return value not in ("0", "false", "no", "off")


def _ProjectConfigPath(chip_root, app, chip_name):
    config_dir = "bflb" if chip_name in ("bl616", "bl616cl") else chip_name
    return os.path.join(chip_root, "examples", app.ExampleName(), "bouffalolab", config_dir, "CHIPProjectConfig.h")


def _RunAutoOta(builder, mode, app, chip_name, **kwargs):
    if not _AutoOtaEnabled():
        log.info("Bouffalo auto OTA generation disabled by BOUFFALOLAB_AUTO_OTA")
        return

    script = os.path.join(builder.chip_dir, "scripts", "tools", "bouffalolab", "bouffalolab_ota_image.py")
    cmd = [
        "python3",
        script,
        "--mode",
        mode,
        "--project-config",
        _ProjectConfigPath(builder.chip_dir, app, chip_name),
    ]
    for key, value in kwargs.items():
        cmd.extend(["--%s" % key.replace("_", "-"), value])
    builder._Execute(cmd, title="Generating Bouffalo Matter OTA image")


def _MatterOtaOutputs(output_dir):
    ota_dir = Path(output_dir) / "ota_images"
    if not ota_dir.is_dir():
        return []
    return sorted(ota_dir.glob("*.matter"))


class _BouffalolabGnBuilder(GnBuilder):

    def __init__(self,
                 root: str,
                 runner: Runner,
                 output_dir_lock: OutDirLock,
                 app: BouffalolabApp = BouffalolabApp.LIGHT,
                 board: BouffalolabBoard = BouffalolabBoard.BL616DK,
                 enable_rpcs: bool = False,
                 module_type: str = "BL706C-22",
                 baudrate=2000000,
                 enable_shell: bool = False,
                 enable_cdc: bool = False,
                 enable_rotating_device_id: bool = False,
                 enable_mfd: bool = False,
                 enable_ethernet: bool = False,
                 enable_wifi: bool = False,
                 enable_thread_type: BouffalolabThreadType = BouffalolabThreadType.NONE,
                 enable_heap_monitoring: bool = False,
                 use_matter_openthread: bool = False,
                 enable_easyflash: bool = False,
                 enable_littlefs: bool = False,
                 enable_pds: bool = False,
                 enable_debug_coredump: bool = False,
                 ):

        if module_type == 'BL602':
            bouffalo_chip = 'bl602'
        elif module_type == 'BL704L':
            bouffalo_chip = 'bl702l'
        elif "BL70" in module_type:
            bouffalo_chip = 'bl702'
        elif module_type == "BL616":
            bouffalo_chip = "bl616"
        else:
            raise Exception(f"module_type {module_type} is not supported")

        super().__init__(
            root=os.path.join(root, 'examples',
                              app.ExampleName(), 'bouffalolab', bouffalo_chip),
            runner=runner,
            output_dir_lock=output_dir_lock
        )

        self.argsOpt = []
        self.chip_name = bouffalo_chip
        self.app = app
        self.board = board

        toolchain = os.path.join(root, os.path.split(os.path.realpath(__file__))[0], '../../../config/bouffalolab/toolchain')
        toolchain = f'custom_toolchain="{toolchain}:riscv_gcc"'
        if toolchain:
            self.argsOpt.append(toolchain)

        self.argsOpt.append(f'board="{self.board.GnArgName()}"')
        self.argsOpt.append(f'baudrate="{baudrate}"')

        enable_thread = enable_thread_type != BouffalolabThreadType.NONE

        if not (enable_wifi or enable_thread or enable_ethernet):
            # decide default connectivity for each chip
            if bouffalo_chip == "bl602":
                enable_wifi, enable_thread, enable_ethernet = True, False, False
            elif bouffalo_chip == "bl702":
                enable_wifi, enable_thread, enable_ethernet = False, True, False
            elif bouffalo_chip == "bl702l":
                enable_wifi, enable_thread, enable_ethernet = False, True, False
            elif bouffalo_chip == "bl616":
                # contact-sensor on bl616 only supports WiFi
                if enable_pds:
                    enable_wifi = True
                else:
                    raise Exception("Must select one of wifi and thread to build.")

        if [enable_wifi, enable_thread, enable_ethernet].count(True) > 1:
            raise Exception('Currently, only one of wifi, thread and ethernet supports.')

        # hardware connectivity support check
        if bouffalo_chip == "bl602":
            if enable_ethernet or enable_thread:
                raise Exception(f"SoC {bouffalo_chip} does NOT support connectivity Ethernet/Thread.")
        elif bouffalo_chip == "bl702":
            self.argsOpt.append(f'module_type="{module_type}"')
            if board != BouffalolabBoard.BL706DK:
                if enable_ethernet or enable_wifi:
                    raise Exception(f"Board {self.board.GnArgName()} does NOT support connectivity Ethernet/Wi-Fi.")
        elif bouffalo_chip == "bl702l":
            if enable_ethernet or enable_wifi:
                raise Exception(f"SoC {bouffalo_chip} does NOT support connectivity Ethernet/Wi-Fi currently.")
        elif bouffalo_chip == "bl616":
            sdk_path = os.path.join(root, os.path.split(os.path.realpath(__file__))[
                                    0], '../../../third_party/bouffalolab/repo_bouffalo_sdk/VERSION')
            x, y, z = self.extract_sdk_version(sdk_path)
            self.argsOpt.append(f'app_ver_x={x}')
            self.argsOpt.append(f'app_ver_y={y}')
            self.argsOpt.append(f'app_ver_z={z}')

        if enable_thread:
            chip_mdns = "platform"
        elif enable_ethernet or enable_wifi:
            chip_mdns = "minimal"

        self.argsOpt.append(f'chip_enable_ethernet={str(enable_ethernet).lower()}')
        self.argsOpt.append(f'chip_enable_wifi={str(enable_wifi).lower()}')
        self.argsOpt.append(f'chip_enable_thread={str(enable_thread).lower()}')

        # for enable_ethernet, do not need ble for commissioning
        self.argsOpt.append(f'chip_config_network_layer_ble={str(enable_wifi or enable_thread).lower()}')

        self.argsOpt.append(f'chip_mdns="{chip_mdns}"')
        self.argsOpt.append(f'chip_inet_config_enable_ipv4={str(enable_ethernet or enable_wifi).lower()}')

        if enable_easyflash and enable_littlefs:
            raise Exception("Only one of easyflash and littlefs can be enabled.")
        if bouffalo_chip == "bl616":
            if enable_easyflash:
                raise Exception("BL616 doesn't support easyflash.")
            enable_littlefs = True
        else:
            if not enable_easyflash and not enable_littlefs:
                log.fatal('*' * 80)
                log.fatal('littlefs is added to support for flash storage access.')
                log.fatal('Please consider and select one of easyflash and littlefs to use.')
                log.fatal('*' * 80)
                raise Exception("None of easyflash and littlefs select to build.")
        self.argsOpt.append(f'bouffalo_sdk_component_easyflash_enabled={"false" if enable_littlefs else "true"}')

        if enable_thread:

            self.argsOpt.append('chip_system_config_use_openthread_inet_endpoints=true')
            self.argsOpt.append('chip_with_lwip=false')
            self.argsOpt.append('openthread_project_core_config_file="openthread-core-proj-config.h"')

            if enable_thread_type == BouffalolabThreadType.THREAD_FTD:
                self.argsOpt.append('chip_openthread_ftd=true')
            else:
                self.argsOpt.append('chip_openthread_ftd=false')

            if not use_matter_openthread:
                if bouffalo_chip in {"bl702", "bl702l"}:
                    self.argsOpt.append('openthread_package_version="7e32165be"')
                    self.argsOpt.append(
                        'openthread_root="//third_party/connectedhomeip/third_party/bouffalolab/repo/components/network/thread/openthread"')
                else:
                    self.argsOpt.append('openthread_package_version="ed6235304"')
                    self.argsOpt.append(
                        'openthread_root="//third_party/connectedhomeip/third_party/bouffalolab/repo_bouffalo_sdk/components/wireless/thread/openthread"')

        if enable_cdc and enable_rpcs:
            raise Exception("Bouffalo USB CDC is not supported with RPC; RPC uses UART.")

        if enable_cdc:
            if bouffalo_chip != "bl702":
                raise Exception(f'SoC {bouffalo_chip} does NOT support USB CDC')
            if enable_ethernet:
                raise Exception(f'SoC {bouffalo_chip} can NOT have both of USB CDC and Ethernet functions together.')

            self.argsOpt.append('enable_cdc_module=true')

        if enable_rpcs:
            self.argsOpt.append('import("//with_pw_rpc.gni")')
        elif enable_shell:
            self.argsOpt.append('chip_build_libshell=true')

        if enable_rotating_device_id:
            self.argsOpt.append('chip_enable_additional_data_advertising=true')
            self.argsOpt.append('chip_enable_rotating_device_id=true')

        if enable_mfd:
            self.argsOpt.append("chip_enable_factory_data=true")

        if enable_pds:
            self.argsOpt.append("enable_pds=true")

        self.argsOpt.append(f"enable_heap_monitoring={str(enable_heap_monitoring).lower()}")
        if enable_debug_coredump:
            self.argsOpt.append("enable_debug_coredump=true")
            self.argsOpt.append(f"coredump_binary_id={int(time.time())}")

        self.argsOpt.append("chip_generate_link_map_file=true")

        try:
            self.argsOpt.append('bouffalolab_sdk_root="%s"' % os.environ['BOUFFALOLAB_SDK_ROOT'])
        except KeyError as err:
            self.print_enviroment_error()
            raise err

    def print_enviroment_error(self):
        log.fatal('*' * 80)
        log.error('Flashtool is not installed, or environment variable BOUFFALOLAB_SDK_ROOT is not exported.')
        log.fatal('\tPlease make sure Bouffalo Lab SDK installs as below:')
        log.fatal('\t\t./integrations/docker/images/stage-2/chip-build-bouffalolab/setup.sh')

        log.fatal('\tPlease make sure BOUFFALOLAB_SDK_ROOT exports before building as below:')
        log.fatal('\t\texport BOUFFALOLAB_SDK_ROOT="your install path"')
        log.fatal('*' * 80)

    def extract_sdk_version(self, filepath):
        pattern = r'PROJECT_SDK_VERSION\s+"([^"]+)"'

        try:
            with open(filepath, encoding='utf-8') as f:
                content = f.read()
            match = re.search(pattern, content)
            if match:
                ver = tuple(int(v) for v in match.group(1).strip().split("."))
                if len(ver) != 3:
                    raise Exception('Invalid version format')
                return ver
            raise Exception('Invalid version format')
        except Exception as err:
            log.error("Failed to extract SDK version: %s", err)
            return (2, 1, 0)

    def GnBuildArgs(self):
        args = super().GnBuildArgs()
        args.extend(self.argsOpt)
        return args

    @lock_output_dir
    def build_outputs(self):
        extensions = ["out"]
        if self.options.enable_link_map_file:
            extensions.append("out.map")
        for ext in extensions:
            name = f"{self.app.AppNamePrefix(self.chip_name)}.{ext}"
            yield BuilderOutput(os.path.join(self.output_dir, name), name)
        for image in _MatterOtaOutputs(self.output_dir):
            yield BuilderOutput(str(image), os.path.join('ota_images', image.name))

    @lock_output_dir
    def PreBuildCommand(self):
        os.system("rm -rf {}/config".format(self.output_dir))
        os.system("rm -rf {}/ota_images".format(self.output_dir))
        os.system("rm -rf {}".format(os.path.join(self.output_dir, 'boot2*.bin')))
        os.system("rm -rf {}".format(os.path.join(self.output_dir, '%s*' % self.app.AppNamePrefix(self.chip_name))))

    @lock_output_dir
    def PostBuildCommand(self):

        if self.chip_name in ["bl616"]:
            abs_path_fw = os.path.join(self.output_dir, self.app.AppNamePrefix(self.chip_name) + ".raw")
        else:
            abs_path_fw = os.path.join(self.output_dir, self.app.AppNamePrefix(self.chip_name) + ".bin")

        if os.path.isfile(abs_path_fw):
            target_dir = self.output_dir.replace(self.chip_dir, "").strip("/")

            path_fw = os.path.join(target_dir, self.app.AppNamePrefix(self.chip_name) + ".bin")
            path_flash_script = os.path.join(target_dir, self.app.AppNamePrefix(self.chip_name) + ".flash.py")

            _RunAutoOta(
                self,
                "iot-sdk",
                self.app,
                self.chip_name,
                flash_script=os.path.join(self.output_dir, self.app.AppNamePrefix(self.chip_name) + ".flash.py"),
            )

            log.info("*" * 80)

            log.info("Firmware is built out at: %s", path_fw)
            log.info("Command to generate ota image: ")
            log.info("./%s --build-ota --vendor-id <vendor id> --product-id <product id> "
                     "--version <version> --version-str <version string> "
                     "--digest-algorithm <digest algorithm>", path_flash_script)
            log.info("Command to generate and sign ota image: ")
            log.info("./%s --build-ota --vendor-id <vendor id> --product-id <product id> "
                     "--version <version> --version-str <version string> "
                     "--digest-algorithm <digest algorithm> --sk <private key>", path_flash_script)
            log.info("*" * 80)


def BouffalolabBuilder(root, runner,
                       module_type: str = "BL706C-22",
                       enable_wifi: bool = False,
                       enable_ethernet: bool = False,
                       enable_thread_type: BouffalolabThreadType = BouffalolabThreadType.NONE,
                       enable_shell: bool = False,
                       enable_mfd: bool = False,
                       enable_easyflash: bool = False,
                       enable_rotating_device_id: bool = False,
                       enable_heap_monitoring: bool = False,
                       enable_debug_coredump: bool = False,
                       app: BouffalolabApp = BouffalolabApp.LIGHT,
                       **kwargs):
    """Factory: BL616/BL616CL → cmake-hybrid bflb builder; all other chips → GN builder."""
    if module_type in ("BL616", "BL616CL"):
        if enable_easyflash:
            raise Exception("BL616 bflb build does not support easyflash; use -littlefs instead.")
        enable_thread = enable_thread_type != BouffalolabThreadType.NONE
        enable_thread_ftd = enable_thread_type != BouffalolabThreadType.THREAD_MTD
        return BouffalolabBflbBuilder(
            root=root, runner=runner,
            app=app,
            module_type=module_type,
            enable_wifi=enable_wifi,
            enable_thread=enable_thread,
            enable_ethernet=enable_ethernet,
            enable_shell=enable_shell,
            enable_mfd=enable_mfd,
            enable_thread_ftd=enable_thread_ftd,
            enable_rotating_device_id=enable_rotating_device_id,
            enable_heap_monitoring=enable_heap_monitoring,
            enable_debug_coredump=enable_debug_coredump,
        )
    return _BouffalolabGnBuilder(
        root=root, runner=runner,
        app=app,
        module_type=module_type,
        enable_wifi=enable_wifi,
        enable_ethernet=enable_ethernet,
        enable_thread_type=enable_thread_type,
        enable_shell=enable_shell,
        enable_mfd=enable_mfd,
        enable_easyflash=enable_easyflash,
        enable_rotating_device_id=enable_rotating_device_id,
        enable_heap_monitoring=enable_heap_monitoring,
        enable_debug_coredump=enable_debug_coredump,
        **kwargs
    )


class BouffalolabBflbBuilder(Builder):
    """BL616 cmake-hybrid builder for examples/lighting-app/bouffalolab/.

    Wraps the Makefile-driven cmake+ninja build.  Exactly one of enable_wifi,
    enable_thread, enable_ethernet must be True.

    Supported make variables (map 1-to-1 from Makefile):
      CONFIG_WIFI / CONFIG_THREAD / CONFIG_ETHERNET  — network interface
      CONFIG_SHELL   — Matter interactive shell
      CONFIG_MFD     — factory data (chip_enable_factory_data)
      OT_FTD         — 1=FTD (default), 0=MTD  (only when CONFIG_THREAD=y)
    """

    def __init__(self,
                 root,
                 runner,
                 app: BouffalolabApp = BouffalolabApp.LIGHT,
                 module_type: str = "BL616",
                 enable_wifi: bool = False,
                 enable_thread: bool = False,
                 enable_ethernet: bool = False,
                 enable_shell: bool = False,
                 enable_mfd: bool = True,
                 enable_thread_ftd: bool = True,
                 enable_rotating_device_id: bool = False,
                 enable_heap_monitoring: bool = False,
                 enable_debug_coredump: bool = False,
                 enable_pds: bool = False,
                 ):
        if app == BouffalolabApp.CONTACT and not enable_wifi:
            raise Exception('contact-sensor-app bflb build only supports WiFi.')
        if [enable_wifi, enable_thread, enable_ethernet].count(True) != 1:
            raise Exception(
                'Exactly one of wifi/thread/ethernet must be enabled for the bflb build.')

        app_dir = {
            BouffalolabApp.LIGHT: 'lighting-app',
            BouffalolabApp.CONTACT: 'contact-sensor-app',
        }[app]

        bflb_dir = os.path.join(
            root, 'examples', app_dir, 'bouffalolab')
        super().__init__(root=bflb_dir, runner=runner)

        self.app = app
        self.chip_name = module_type.lower()
        self.board_name = {
            'bl616': 'bl616dk',
            'bl616cl': 'bl616cldk',
        }.get(self.chip_name, "%sdk" % self.chip_name)
        self.enable_wifi = enable_wifi
        self.enable_thread = enable_thread
        self.enable_ethernet = enable_ethernet
        self.enable_shell = enable_shell
        self.enable_mfd = enable_mfd
        self.enable_thread_ftd = enable_thread_ftd
        self.enable_rotating_device_id = enable_rotating_device_id
        self.enable_heap_monitoring = enable_heap_monitoring
        self.enable_debug_coredump = enable_debug_coredump

    def _make_args(self):
        args = [
            'CHIP=%s' % self.chip_name,
            'BOARD=%s' % self.board_name,
            'CROSS_COMPILE=%s' % self._cross_compile_prefix(),
            'CONFIG_WIFI=%s' % ('y' if self.enable_wifi else 'n'),
            'CONFIG_THREAD=%s' % ('y' if self.enable_thread else 'n'),
            'CONFIG_ETHERNET=%s' % ('y' if self.enable_ethernet else 'n'),
            'CONFIG_SHELL=%s' % ('y' if self.enable_shell else 'n'),
            'CONFIG_MFD=%s' % ('y' if self.enable_mfd else 'n'),
            'CONFIG_CHIP_ROTATING_DEVICE_ID=%s' % ('y' if self.enable_rotating_device_id else 'n'),
            'CONFIG_CHIP_HEAP_MONITOR=%s' % ('y' if self.enable_heap_monitoring else 'n'),
            'CONFIG_COREDUMP=%s' % ('y' if self.enable_debug_coredump else 'n'),
        ]
        if self.enable_thread and not self.enable_thread_ftd:
            args.append('OT_FTD=0')
        return args

    def _cross_compile_prefix(self):
        sdk_root = os.environ.get('BOUFFALOLAB_SDK_ROOT')
        prefixes = []
        if sdk_root:
            prefixes.extend([
                os.path.join(sdk_root, 'toolchain', 't-head-riscv', 'gcc_t-head_v2.6.1', 'bin', 'riscv64-unknown-elf-'),
                os.path.join(sdk_root, 'toolchain', 'riscv', 'Linux', 'bin', 'riscv64-unknown-elf-'),
            ])
        for prefix in prefixes:
            if os.path.isfile(prefix + 'gcc'):
                return prefix
        return 'riscv64-unknown-elf-'

    def _toolchain_path(self):
        prefix = self._cross_compile_prefix()
        if os.path.isabs(prefix):
            toolchain_bin = os.path.dirname(prefix.rstrip(os.sep))
            return '%s%s%s' % (toolchain_bin, os.pathsep, os.environ.get('PATH', ''))
        return os.environ.get('PATH', '')

    def _build_path(self):
        base_path = self._toolchain_path()
        ninja_path = shutil.which('ninja', path=base_path)
        if not ninja_path:
            return base_path

        tools_dir = os.path.join(self.output_dir, 'tools')
        os.makedirs(tools_dir, exist_ok=True)
        wrapper_path = os.path.join(tools_dir, 'ninja')
        with open(wrapper_path, 'w') as wrapper:
            wrapper.write('#!/bin/sh\n')
            wrapper.write('unset MAKEFLAGS\n')
            wrapper.write('exec "%s" "$@"\n' % ninja_path)
        os.chmod(wrapper_path, 0o755)
        return '%s%s%s' % (tools_dir, os.pathsep, base_path)

    def _sdk_build_dir(self):
        return os.path.join('build', self.identifier)

    def generate(self):
        # cmake configuration is invoked by the Makefile; no separate step needed.
        pass

    def _build(self):
        # The runner runs from the Matter repo root.  -C self.root changes to
        # examples/lighting-app/bouffalolab/ where $(abspath .) evaluates correctly.
        # BUILD_DIR is target-specific and relative to that dir:
        #   cmake binary dir = examples/<app>/bouffalolab/build/<target>/
        #   SDK output dir   = examples/<app>/bouffalolab/build/<target>/build_out/
        # self.output_dir (set by the framework) is out/<target> and is used in
        # _post_build() for symlinks so the output layout matches GN builds.
        build_dir = self._sdk_build_dir()
        stale_shared_build_dir = os.path.join(self.root, 'build', 'build_out')
        if os.path.isdir(stale_shared_build_dir):
            shutil.rmtree(stale_shared_build_dir)
        log.info('Building %s cmake-hybrid bflb at %s', self.chip_name, self.root)
        self._Execute(
            ['env', 'PATH=%s' % self._build_path(),
             'make', '-C', self.root, 'BUILD_DIR=%s' % build_dir] + self._make_args(),
            title='Building %s bflb %s' % (self.chip_name, self.app.ExampleName()),
        )
        self._post_build()

    def _post_build(self):
        """Symlink SDK outputs into self.output_dir (out/<target>) with GN-style
        names, then generate chip-<chip>-<app>.flash.py next to them.
        """
        chip_name = self.chip_name
        sdk_proj_name = {
            BouffalolabApp.LIGHT: 'chip-bflb-lighting-example',
            BouffalolabApp.CONTACT: 'chip-bflb-contact-sensor-example',
        }[self.app]
        # SDK produces: self.root/<build_dir>/build_out/<proj_name>_<chip>.{bin,elf}
        sdk_out = os.path.join(self.root, self._sdk_build_dir(), 'build_out')
        sdk_bin = os.path.join(sdk_out, '%s_%s.bin' % (sdk_proj_name, chip_name))
        if not os.path.isfile(sdk_bin):
            return

        # GN-style name: chip-<chip>-<app-example>.{bin,elf}
        app_prefix = self.app.AppNamePrefix(chip_name)

        # Create symlinks in self.output_dir (out/<target>) so copy-artifacts
        # and users find GN-consistent names alongside the GN builder layout.
        os.makedirs(self.output_dir, exist_ok=True)
        for ext in ('bin', 'elf'):
            gn_name = '%s.%s' % (app_prefix, ext)
            gn_link = os.path.join(self.output_dir, gn_name)
            sdk_file = os.path.join(sdk_out, '%s_%s.%s' % (sdk_proj_name, chip_name, ext))
            try:
                if os.path.islink(gn_link) or os.path.isfile(gn_link):
                    os.remove(gn_link)
                os.symlink(sdk_file, gn_link)
            except OSError:
                pass

        # Generate flash.py that references the GN-style bin name.
        matter_root = os.path.dirname(os.path.dirname(os.path.dirname(self.root)))
        gen_script = os.path.join(matter_root, 'scripts', 'flashing', 'gen_flashing_script.py')
        flash_py_name = '%s.flash.py' % app_prefix
        flash_py_path = os.path.join(self.output_dir, flash_py_name)

        flash_args = [
            'python3', gen_script, 'bouffalolab',
            '--chipname', chip_name,
            '--baudrate', '2000000',
            '--application', os.path.join(self.output_dir, '%s.bin' % app_prefix),
            '--output', flash_py_path,
        ]
        if chip_name in ('bl616', 'bl616cl'):
            config_dir = os.path.join(self.output_dir, 'config')
            board_config = {
                'bl616': ('bl_factory_params_IoTKitA_auto.dts', 'boot2_bl616_isp_release_v8.1.8.bin'),
                'bl616cl': ('bl_factory_params_IoTKitA_auto.dts', 'boot2_bl616cl_isp_release_v8.2.1.bin'),
            }[chip_name]
            flash_args.extend([
                '--config', os.path.join(self.output_dir, 'flash_prog_cfg.ini'),
                '--pt', os.path.join(config_dir, 'partition_cfg_4M.toml'),
                '--dts', os.path.join(config_dir, board_config[0]),
                '--boot2', os.path.join(config_dir, board_config[1]),
            ])

        self._Execute(flash_args, title='Generating flash.py wrapper')

        flashing_dir = os.path.join(matter_root, 'scripts', 'flashing')
        for runtime in ('bouffalolab_firmware_utils.py', 'firmware_utils.py'):
            shutil.copy2(os.path.join(flashing_dir, runtime), self.output_dir)

        bl616_config = {
            'bl616': ('bl616dk', 'boot2_bl616_isp_release_v8.1.8.bin'),
            'bl616cl': ('bl616cldk', 'boot2_bl616cl_isp_release_v8.2.1.bin'),
        }
        if chip_name in bl616_config:
            board_name, boot2_name = bl616_config[chip_name]
            config_dir = os.path.join(self.output_dir, 'config')
            os.makedirs(config_dir, exist_ok=True)
            board_config_dir = os.path.join(
                matter_root, 'third_party', 'bouffalolab', 'repo_bouffalo_sdk', 'bsp', 'board', board_name, 'config')
            shutil.copy2(
                os.path.join(matter_root, 'examples', 'platform', 'bouffalolab', 'bflb', 'flash_config',
                             'partition_cfg_4M.toml'),
                config_dir)
            for config_file in ('bl_factory_params_IoTKitA_auto.dts', boot2_name):
                shutil.copy2(os.path.join(board_config_dir, config_file), config_dir)

        rel = os.path.relpath(flash_py_path, matter_root)
        gn_bin_rel = os.path.relpath(os.path.join(self.output_dir, '%s.bin' % app_prefix), matter_root)
        log.info('*' * 80)
        log.info('Firmware:     %s', gn_bin_rel)
        log.info('Flash script: %s', rel)
        log.info('Flash to device:  python3 %s --port /dev/ttyUSB0', rel)
        log.info('*' * 80)

        sdk_ota_dir = os.path.join(self.root, self._sdk_build_dir(), 'build_out', 'ota_images')
        if os.path.isdir(sdk_ota_dir):
            out_ota_dir = os.path.join(self.output_dir, 'ota_images')
            if os.path.isdir(out_ota_dir):
                shutil.rmtree(out_ota_dir)
            os.makedirs(out_ota_dir, exist_ok=True)
            for image in sorted(Path(sdk_ota_dir).glob("*.matter")):
                target = os.path.join(out_ota_dir, os.path.basename(image))
                try:
                    if os.path.islink(target) or os.path.isfile(target):
                        os.remove(target)
                    os.symlink(image, target)
                except OSError:
                    shutil.copy2(image, target)

    def build_outputs(self):
        # Yield GN-consistent names from self.output_dir (out/<target>).
        # Symlinks are created in _post_build() so these files always exist.
        chip_name = self.chip_name
        app_prefix = self.app.AppNamePrefix(chip_name)
        for ext in ('bin', 'elf'):
            name = '%s.%s' % (app_prefix, ext)
            yield BuilderOutput(os.path.join(self.output_dir, name), name)
        for image in _MatterOtaOutputs(self.output_dir):
            yield BuilderOutput(str(image), os.path.join('ota_images', image.name))
