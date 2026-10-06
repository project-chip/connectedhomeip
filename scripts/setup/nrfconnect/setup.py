#!/usr/bin/env python3

#
# Copyright (c) 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

"""Setup helper for nRF Connect SDK Matter development environment."""

import argparse
import os
import platform
import shutil
import stat
import subprocess
import sys
import urllib.error
import urllib.request
from abc import ABC, abstractmethod
from dataclasses import dataclass
from pathlib import Path

import yaml

DIVIDER = "=" * 72
ENVIRONMENT_DIR = Path(".environment")
SDK_INSTALL_SUBPATH = ENVIRONMENT_DIR / "nrfconnect/sdk"
NRFCONNECT_ENV_STATE = ENVIRONMENT_DIR / "nrfconnect" / "env_state.env"
NRFCONNECT_ENV_STATE_CMD = ENVIRONMENT_DIR / "nrfconnect" / "env_state.cmd"
NCS_MATTER_SUBMODULE = Path("third_party/nrfconnect/ncs-matter")
ZEPHYRRC_EXCLUDED_PREFIXES = {
    "sh": ("export PYTHONHOME=", "export PYTHONPATH="),
    "cmd": ("SET PYTHONHOME=", "SET PYTHONPATH="),
}


def prompt_yes_no(message: str, default: bool = False) -> bool:
    suffix = " [Y/n]: " if default else " [y/N]: "
    try:
        reply = input(message + suffix).strip().lower()
    except EOFError:
        reply = ""

    if not reply:
        return default
    return reply in ("y", "yes")


def get_chip_root() -> Path:
    return next(filter(lambda p: (p / "SPECIFICATION_VERSION").is_file(), Path(__file__).parents))


def get_ncs_matter_addon_path(chip_root: Path) -> Path:
    addon_path = chip_root / NCS_MATTER_SUBMODULE
    if not addon_path.is_dir():
        raise RuntimeError(
            f"ncs-matter add-on not found at {addon_path}. "
            "Run the checkout submodules phase first or initialize the submodule manually.")
    return addon_path.resolve()


def get_nrf_revision(chip_root: Path) -> str:
    west_yml = get_ncs_matter_addon_path(chip_root) / "west.yml"
    try:
        with open(west_yml, encoding="utf-8") as west_yml_file:
            manifest = yaml.safe_load(west_yml_file)
    except OSError as exc:
        raise RuntimeError(f"Unable to read ncs-matter manifest at {west_yml}.") from exc

    for project in manifest.get("manifest", {}).get("projects", []):
        if project.get("name") == "nrf":
            revision = project.get("revision")
            if revision:
                return revision

    raise RuntimeError("Unable to find the nrf project revision in ncs-matter west.yml.")


def resolve_sdk_install_dir(chip_root: Path, sdk_path: Path | None) -> Path:
    if sdk_path is not None:
        return sdk_path.resolve()
    return (chip_root / SDK_INSTALL_SUBPATH).resolve()


def get_nrfconnect_setup_dir(chip_root: Path) -> Path:
    return chip_root / "scripts" / "setup" / "nrfconnect"


def get_activate_script(chip_root: Path) -> Path:
    setup_dir = get_nrfconnect_setup_dir(chip_root)
    if sys.platform == "win32":
        return (setup_dir / "activate.bat").resolve()
    return (setup_dir / "activate.sh").resolve()


def get_deactivate_script(chip_root: Path) -> Path:
    setup_dir = get_nrfconnect_setup_dir(chip_root)
    if sys.platform == "win32":
        return (setup_dir / "deactivate.bat").resolve()
    return (setup_dir / "deactivate.sh").resolve()


def format_deactivate_command(deactivate_script: Path) -> str:
    if sys.platform == "win32":
        return f'call "{deactivate_script}"'
    return f'source "{deactivate_script}"'


def find_nrfutil_lock_files(sdk_install_dir: Path) -> list[Path]:
    toolchains_dir = sdk_install_dir / "toolchains"
    if not toolchains_dir.is_dir():
        return []
    return sorted(toolchains_dir.glob("*/nrfutil/home/locked"))


def is_nrfconnect_environment_active(chip_root: Path, sdk_install_dir: Path) -> bool:
    if (chip_root / NRFCONNECT_ENV_STATE).is_file():
        return True

    if (chip_root / NRFCONNECT_ENV_STATE_CMD).is_file():
        return True

    if find_nrfutil_lock_files(sdk_install_dir):
        return True

    nrfutil_home = os.environ.get("NRFUTIL_HOME")
    return bool(nrfutil_home and Path(nrfutil_home).joinpath("locked").is_file())


def ensure_nrfconnect_environment_inactive(chip_root: Path, sdk_install_dir: Path) -> None:
    if not is_nrfconnect_environment_active(chip_root, sdk_install_dir):
        return

    deactivate_script = get_deactivate_script(chip_root)
    print("nRF Connect environment is already active.", file=sys.stderr)
    print("Deactivate it first with:", file=sys.stderr)
    print(f"  {format_deactivate_command(deactivate_script)}", file=sys.stderr)
    sys.exit(1)


def get_zephyr_base(chip_root: Path, sdk_install_dir: Path) -> Path:
    ncs_version = get_nrf_revision(chip_root)
    zephyr_base = sdk_install_dir / ncs_version / "zephyr"
    if not zephyr_base.is_dir():
        raise RuntimeError(
            f"Zephyr tree not found at {zephyr_base}. "
            "Run setup first to download the nRF Connect SDK.")
    return zephyr_base.resolve()


def is_sdk_installed(sdk_install_dir: Path, ncs_version: str) -> bool:
    return (sdk_install_dir / ncs_version / "zephyr").is_dir()


def is_toolchain_installed(sdk_install_dir: Path, ncs_version: str) -> bool:
    process = subprocess.run(
        [
            "nrfutil",
            "sdk-manager",
            "toolchain",
            "list",
            "--install-dir",
            str(sdk_install_dir),
        ],
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    return any(
        line.split()[0] == ncs_version
        for line in process.stdout.splitlines()
        if line.strip() and not line.startswith("Version")
    )


def _write_zephyrrc_script(
    sdk_install_dir: Path,
    ncs_version: str,
    script_type: str,
    filename: str,
) -> Path:
    zephyrrc_path = sdk_install_dir / ncs_version / filename
    process = subprocess.run(
        [
            "nrfutil",
            "sdk-manager",
            "toolchain",
            "env",
            "--ncs-version",
            ncs_version,
            "--install-dir",
            str(sdk_install_dir),
            "--as-script",
            script_type,
        ],
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )

    # PYTHONHOME and PYTHONPATH from the NCS toolchain conflict with the Matter
    # pigweed virtual environment used by scripts/activate.sh.
    excluded_prefixes = ZEPHYRRC_EXCLUDED_PREFIXES[script_type]
    lines = [
        line for line in process.stdout.splitlines()
        if line and not line.startswith(excluded_prefixes)
    ]
    zephyrrc_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote {zephyrrc_path}")
    return zephyrrc_path


def write_zephyrrc(sdk_install_dir: Path, ncs_version: str) -> Path:
    if sys.platform == "win32":
        return _write_zephyrrc_script(sdk_install_dir, ncs_version, "cmd", ".zephyrrc.cmd")
    return _write_zephyrrc_script(sdk_install_dir, ncs_version, "sh", ".zephyrrc")


def update_ncs_workspace(chip_root: Path, sdk_install_dir: Path, ncs_version: str) -> None:
    update_ncs_script = chip_root / "scripts" / "setup" / "nrfconnect" / "update_ncs.py"
    if not update_ncs_script.is_file():
        raise RuntimeError(f"update_ncs.py not found at {update_ncs_script}")

    command = [
        "nrfutil",
        "sdk-manager",
        "toolchain",
        "launch",
        "--ncs-version",
        ncs_version,
        "--install-dir",
        str(sdk_install_dir),
        "--",
        sys.executable,
        str(update_ncs_script),
        "--update",
    ]
    print(f"Running: {' '.join(command)}")
    subprocess.run(command, cwd=chip_root, check=True, env=os.environ.copy())


@dataclass
class SetupContext:
    auto_download: bool = False
    sdk_install_dir: Path | None = None
    skip_bootstrap: bool = False


class SetupPhase(ABC):
    @property
    @abstractmethod
    def title(self) -> str:
        pass

    @abstractmethod
    def run(self, context: SetupContext) -> bool:
        """Run the phase. Return True when the phase completed successfully."""

    def print_header(self, phase_num: int, total_phases: int) -> None:
        print()
        print(DIVIDER)
        print(f"Phase {phase_num}/{total_phases}: {self.title}")
        print(DIVIDER)
        print()


def get_os_name() -> str:
    if sys.platform == "darwin":
        return "macOS"
    if sys.platform.startswith("linux"):
        return "Linux"
    if sys.platform == "win32":
        return "Windows"
    return sys.platform


class CheckoutSubmodulesPhase(SetupPhase):
    PLATFORM = "nrfconnect"

    @property
    def title(self) -> str:
        return "Checking out submodules"

    def run(self, context: SetupContext) -> bool:
        del context

        chip_root = get_chip_root()
        checkout_script = chip_root / "scripts" / "checkout_submodules.py"
        if not checkout_script.is_file():
            raise RuntimeError(f"checkout script not found at {checkout_script}")

        command = [
            sys.executable,
            str(checkout_script),
            "--allow-changing-global-git-config",
            "--shallow",
            "--platform",
            self.PLATFORM,
        ]
        print(f"Running: {' '.join(command)}")
        subprocess.run(command, cwd=chip_root, check=True)
        return True


class NrfutilInstallPhase(SetupPhase):
    DOWNLOAD_PAGE = "https://www.nordicsemi.com/Products/Development-tools/nRF-Util"
    ARTIFACTORY_BASE = (
        "https://files.nordicsemi.com/artifactory/swtools/external/nrfutil/executables")

    @property
    def title(self) -> str:
        return "Checking nrfutil installation"

    def run(self, context: SetupContext) -> bool:
        version = self._get_version()
        if version:
            print(f"nrfutil is installed: {version}")
            return True

        print("nrfutil was not found in PATH.")
        print(f"Download it from: {self.DOWNLOAD_PAGE}")
        print()

        if sys.platform == "win32":
            print("On Windows you can also install nrfutil with:")
            print("  winget install nrfutil")
            print()

        download = context.auto_download or prompt_yes_no(
            "Download and install nrfutil automatically?")
        if not download:
            raise RuntimeError(
                "nrfutil is required. Install it manually and re-run this script.")

        url, executable_name = self._get_download_info()
        install_dir = self._get_install_dir()
        self._download(install_dir, url, executable_name)

        version = self._get_version()
        if not version:
            print()
            for line in self._get_path_addition_instructions(install_dir):
                print(line)
            raise RuntimeError(
                "nrfutil was downloaded, but it is still not available in PATH.")

        print(f"nrfutil installed successfully: {version}")
        print()
        for line in self._get_path_addition_instructions(install_dir):
            print(line)
        return True

    @staticmethod
    def _get_host_arch() -> str:
        machine = platform.machine().lower()
        if machine in ("x86_64", "amd64"):
            return "x86_64"
        if machine in ("aarch64", "arm64"):
            return "aarch64"
        raise RuntimeError(f"Unsupported CPU architecture for nrfutil: {platform.machine()}")

    def _get_download_info(self) -> tuple[str, str]:
        if sys.platform == "darwin":
            return (
                f"{self.ARTIFACTORY_BASE}/universal-apple-darwin/nrfutil",
                "nrfutil",
            )

        if sys.platform.startswith("linux"):
            arch = self._get_host_arch()
            return (
                f"{self.ARTIFACTORY_BASE}/{arch}-unknown-linux-gnu/nrfutil",
                "nrfutil",
            )

        if sys.platform == "win32":
            return (
                f"{self.ARTIFACTORY_BASE}/x86_64-pc-windows-msvc/nrfutil.exe",
                "nrfutil.exe",
            )

        raise RuntimeError(f"Unsupported operating system: {sys.platform}")

    @staticmethod
    def _get_install_dir() -> Path:
        if sys.platform == "win32":
            local_app_data = os.environ.get("LOCALAPPDATA")
            if not local_app_data:
                raise RuntimeError("LOCALAPPDATA is not set; cannot choose an install directory.")
            return Path(local_app_data) / "nrfutil" / "bin"

        return Path.home() / ".local" / "bin"

    @staticmethod
    def _get_path_addition_instructions(install_dir: Path) -> list[str]:
        install_dir_str = str(install_dir)

        if sys.platform == "win32":
            return [
                f"Add {install_dir_str} to your PATH using System Properties > Environment Variables,",
                "or run the following in PowerShell for the current session:",
                f'  $env:PATH = "{install_dir_str};$env:PATH"',
            ]

        return [
            f"Add {install_dir_str} to your PATH, for example:",
            f'  export PATH="{install_dir_str}:$PATH"',
            "",
            "To persist the change, add the export line to your shell profile "
            "(~/.bashrc, ~/.zshrc, or equivalent).",
        ]

    @staticmethod
    def _add_install_dir_to_path(install_dir: Path) -> None:
        install_dir_str = str(install_dir)
        path_entries = os.environ.get("PATH", "").split(os.pathsep)
        if install_dir_str not in path_entries:
            os.environ["PATH"] = os.pathsep.join([install_dir_str, *path_entries])

    def _download(self, install_dir: Path, url: str, executable_name: str) -> Path:
        install_dir.mkdir(parents=True, exist_ok=True)
        destination = install_dir / executable_name

        print(f"Downloading nrfutil from {url}")
        print(f"Installing to {destination}")

        try:
            with urllib.request.urlopen(url) as response, open(destination, "wb") as output_file:
                shutil.copyfileobj(response, output_file)
        except urllib.error.URLError as exc:
            raise RuntimeError(f"Failed to download nrfutil from {url}: {exc}") from exc

        if sys.platform != "win32":
            destination.chmod(
                destination.stat().st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)

        self._add_install_dir_to_path(install_dir)
        return destination

    @staticmethod
    def _get_version() -> str | None:
        if not shutil.which("nrfutil"):
            return None

        process = subprocess.run(
            ["nrfutil", "--version"],
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        if process.returncode != 0:
            return None

        return process.stdout.strip().splitlines()[0]


class NrfutilModulesPhase(SetupPhase):
    MODULES = ("device", "sdk-manager")

    @property
    def title(self) -> str:
        return "Installing nrfutil device and sdk-manager modules"

    def run(self, context: SetupContext) -> bool:
        del context

        print(f"Installing the latest nrfutil {' and '.join(self.MODULES)} modules...")
        self._install_modules()

        for module in self.MODULES:
            print(f"Installed {self._get_module_version(module)}")
        return True

    def _install_modules(self) -> None:
        subprocess.run(
            ["nrfutil", "install", *self.MODULES],
            check=True,
        )

    @staticmethod
    def _get_module_version(module: str) -> str:
        process = subprocess.run(
            ["nrfutil", module, "--version"],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        return process.stdout.strip().splitlines()[0]


class DownloadNrfconnectSDKPhase(SetupPhase):
    @property
    def title(self) -> str:
        return "Downloading nRF Connect SDK"

    def run(self, context: SetupContext) -> bool:
        chip_root = get_chip_root()
        install_dir = resolve_sdk_install_dir(chip_root, context.sdk_install_dir)
        ncs_version = get_nrf_revision(chip_root)

        print(f"Using SDK install directory: {install_dir}")
        print(f"Required nRF Connect SDK version: {ncs_version}")

        sdk_installed = is_sdk_installed(install_dir, ncs_version)
        toolchain_installed = is_toolchain_installed(install_dir, ncs_version)

        if sdk_installed:
            print(f"nRF Connect SDK {ncs_version} is already installed.")
        if toolchain_installed:
            print(f"Toolchain for {ncs_version} is already installed.")

        if sdk_installed and toolchain_installed:
            return True

        install_dir.mkdir(parents=True, exist_ok=True)

        if not sdk_installed:
            command = [
                "nrfutil",
                "sdk-manager",
                "install",
                ncs_version,
                "--install-dir",
                str(install_dir),
            ]
            print(f"Running: {' '.join(command)}")
            subprocess.run(command, check=True)
            return True

        command = [
            "nrfutil",
            "sdk-manager",
            "toolchain",
            "install",
            "--ncs-version",
            ncs_version,
            "--install-dir",
            str(install_dir),
        ]
        print(f"Running: {' '.join(command)}")
        subprocess.run(command, check=True)
        return True


class FinalizeEnvironmentPhase(SetupPhase):
    @property
    def title(self) -> str:
        return "Finalizing environment"

    def run(self, context: SetupContext) -> bool:
        chip_root = get_chip_root()
        sdk_install_dir = resolve_sdk_install_dir(chip_root, context.sdk_install_dir)
        ncs_version = get_nrf_revision(chip_root)
        zephyr_base = get_zephyr_base(chip_root, sdk_install_dir)

        os.environ["ZEPHYR_BASE"] = str(zephyr_base)
        scripts_path = str(zephyr_base / "scripts")
        path_entries = os.environ.get("PATH", "").split(os.pathsep)
        if scripts_path not in path_entries:
            os.environ["PATH"] = os.pathsep.join([scripts_path, *path_entries])

        print(f"ZEPHYR_BASE set to {zephyr_base}")

        write_zephyrrc(sdk_install_dir, ncs_version)
        update_ncs_workspace(chip_root, sdk_install_dir, ncs_version)
        return True


class ZephyrBasePhase(SetupPhase):
    @property
    def title(self) -> str:
        return "Checking ZEPHYR_BASE environment variable"

    def run(self, context: SetupContext) -> bool:
        del context

        zephyr_base = os.getenv("ZEPHYR_BASE")
        if not zephyr_base:
            raise RuntimeError("ZEPHYR_BASE is not set after finalizing the environment.")

        resolved = Path(zephyr_base).resolve()
        if not resolved.is_dir():
            raise RuntimeError(
                f"ZEPHYR_BASE is set to {zephyr_base}, but the path does not exist.")

        print(f"ZEPHYR_BASE is set to {resolved}")
        return True


class BootstrapPhase(SetupPhase):
    PLATFORM = "nrfconnect"

    @property
    def title(self) -> str:
        return "Running Matter bootstrap"

    def run(self, context: SetupContext) -> bool:
        del context

        chip_root = get_chip_root()
        bootstrap_script = chip_root / "scripts" / "bootstrap.sh"
        if not bootstrap_script.is_file():
            raise RuntimeError(f"bootstrap script not found at {bootstrap_script}")

        command = [
            "bash",
            str(bootstrap_script),
            "-p",
            self.PLATFORM,
        ]
        print(f"Running: {' '.join(command)}")
        subprocess.run(command, cwd=chip_root, check=True, env=os.environ.copy())
        return True


def _run_in_activated_env(chip_root: Path, script: str) -> None:
    activate_script = get_activate_script(chip_root)
    if not activate_script.is_file():
        raise RuntimeError(f"activate script not found at {activate_script}")

    if sys.platform == "win32":
        command = f'call "{activate_script}" && {script}'
        print(f"Running: cmd /c \"{command}\"")
        subprocess.run(["cmd", "/c", command], cwd=chip_root, check=True)
        return

    command = f'NRFCONNECT_SKIP_ENV_STATE=1 source "{activate_script}" && {script}'
    print(f"Running: bash -c '{command}'")
    subprocess.run(["bash", "-c", command], cwd=chip_root, check=True)


class InstallWestPhase(SetupPhase):
    WEST_REQUIREMENT = "west>=1.4.0"

    @property
    def title(self) -> str:
        return "Installing west in the activated environment"

    def run(self, context: SetupContext) -> bool:
        del context

        chip_root = get_chip_root()
        pip_command = "python -m pip install" if sys.platform == "win32" else "python3 -m pip install"
        _run_in_activated_env(chip_root, f"{pip_command} '{self.WEST_REQUIREMENT}'")
        _run_in_activated_env(chip_root, "west --version")
        return True


def _launch_post_activation_shell(chip_root: Path, activate_script: Path) -> None:
    build_hint = "./scripts/build/build_examples.py --target nrf-nrf54lm20dk-light build"
    if sys.platform == "win32":
        build_hint = "python scripts/build/build_examples.py --target nrf-nrf54lm20dk-light build"
        command = (
            f"call \"{activate_script}\" && "
            "echo. && "
            "echo You can now build a sample, for example: && "
            f"echo   {build_hint} && "
            "echo."
        )
        print(f"Running: cmd /k \"{command}\"")
        subprocess.run(["cmd", "/k", command], cwd=chip_root)
        return

    command = (
        f"source {activate_script} && "
        "echo && "
        "echo 'You can now build a sample, for example:' && "
        f"echo '  {build_hint}' && "
        "echo && "
        "exec bash"
    )
    print(f"Running: bash -c '{command}'")
    subprocess.run(["bash", "-c", command], cwd=chip_root)


class LaunchActivatedShellPhase(SetupPhase):
    @property
    def title(self) -> str:
        return "Launching activated shell"

    def run(self, context: SetupContext) -> bool:
        del context

        chip_root = get_chip_root()
        activate_script = get_activate_script(chip_root)
        if not activate_script.is_file():
            raise RuntimeError(f"activate script not found at {activate_script}")
        _launch_post_activation_shell(chip_root, activate_script)
        return True


def get_setup_phases() -> list[SetupPhase]:
    return [
        CheckoutSubmodulesPhase(),
        NrfutilInstallPhase(),
        NrfutilModulesPhase(),
        DownloadNrfconnectSDKPhase(),
        FinalizeEnvironmentPhase(),
        ZephyrBasePhase(),
    ]


def get_post_setup_phases(context: SetupContext) -> list[SetupPhase]:
    phases: list[SetupPhase] = []
    if not context.skip_bootstrap:
        phases.append(BootstrapPhase())
    phases.append(InstallWestPhase())
    phases.append(LaunchActivatedShellPhase())
    return phases


def cleanup_environment(chip_root: Path) -> None:
    environment_dir = chip_root / ENVIRONMENT_DIR
    if not environment_dir.exists():
        print(f"No {ENVIRONMENT_DIR} directory found at {environment_dir}. Nothing to clean up.")
        return

    print(f"This will permanently remove {environment_dir} and all downloaded SDK and toolchain files.")
    print("Matter development environment will be removed. And the bootstrap script must be run again.")
    print("This action cannot be undone.")
    if not prompt_yes_no("Do you want to continue?"):
        print("Cleanup cancelled.")
        return

    print(f"Removing {environment_dir}...")
    shutil.rmtree(environment_dir)
    print("Cleanup completed.")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Set up the nRF Connect SDK environment for Matter development.")
    parser.add_argument(
        "-y", "--yes",
        help="Automatically download nrfutil if it is missing.",
        action="store_true",
    )
    parser.add_argument(
        "--sdk-path",
        type=Path,
        help="Use a custom SDK install directory instead of .environment/nrfconnect/sdk. "
        "Validates the required SDK and toolchain are present, installing them if missing.",
    )
    parser.add_argument(
        "--cleanup",
        help="Remove the .environment directory with downloaded SDK and toolchain files.",
        action="store_true",
    )
    parser.add_argument(
        "--skip-bootstrap",
        help="Skip running scripts/bootstrap.sh after setup completes.",
        action="store_true",
    )
    args = parser.parse_args()

    chip_root = get_chip_root()
    sdk_path = args.sdk_path.expanduser() if args.sdk_path else None
    sdk_install_dir = resolve_sdk_install_dir(chip_root, sdk_path)

    ensure_nrfconnect_environment_inactive(chip_root, sdk_install_dir)

    if args.cleanup:
        cleanup_environment(chip_root)
        return

    context = SetupContext(
        auto_download=args.yes,
        sdk_install_dir=sdk_path,
        skip_bootstrap=args.skip_bootstrap,
    )
    phases = get_setup_phases()
    post_setup_phases = get_post_setup_phases(context)
    total_phases = len(phases) + len(post_setup_phases)

    print(f"nRF Connect SDK setup ({get_os_name()})")

    try:
        for phase_num, phase in enumerate(phases, start=1):
            phase.print_header(phase_num, total_phases)
            phase.run(context)
    except (RuntimeError, subprocess.CalledProcessError) as exc:
        print()
        print(f"ERROR: {exc}")
        sys.exit(1)

    print()
    print(DIVIDER)
    print("Setup completed.")
    print(DIVIDER)

    for phase_num, phase in enumerate(post_setup_phases, start=len(phases) + 1):
        phase.print_header(phase_num, total_phases)
        try:
            phase.run(context)
        except (RuntimeError, subprocess.CalledProcessError) as exc:
            print()
            print(f"ERROR: {exc}")
            sys.exit(1)


if __name__ == "__main__":
    main()
