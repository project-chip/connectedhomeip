#!/usr/bin/env python3

"""
Download and setup SLT (Silicon Labs Tools) CLI for Matter development.

This script downloads and installs the SLT CLI tool which is used to manage
Silicon Labs development tools required for Matter development.
"""

import argparse
import logging
import os
import stat
import subprocess
import sys
from zipfile import ZipFile

try:
    import dload
except ImportError:
    logging.error("dload package is required. Install it with: pip install dload")
    sys.exit(1)


def setup_logging(verbose=False):
    """Configure logging level and format based on verbosity setting."""
    level = logging.DEBUG if verbose else logging.INFO
    logging.basicConfig(level=level, format='[%(levelname)s] %(message)s')


def get_platform_vars():
    """Set platform-specific variables and URLs for SLT CLI download. Linux and macOS only."""
    platform = sys.platform
    if platform == "darwin":
        platform_name = "mac"
    elif platform == "linux":
        platform_name = "linux"
    else:
        logging.error(f"ERROR: Platform {platform} is not supported (Linux and macOS only)")
        sys.exit(1)

    slt_cli_url = f"https://www.silabs.com/documents/public/software/slt-cli-1.0.1-{platform_name}-x64.zip"
    return platform_name, slt_cli_url


def get_tools_path():
    """Return the script directory as the download location."""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.makedirs(script_dir, exist_ok=True)
    return script_dir


def make_executable(path):
    """Make a file executable on Unix-like systems."""
    if path and os.path.exists(path):
        try:
            st = os.stat(path)
            os.chmod(path, st.st_mode | stat.S_IEXEC)
        except OSError as e:
            logging.warning(f"Could not make {path} executable: {e}")
    else:
        logging.warning(f"Path {path} does not exist to make executable.")


def find_slt_in_path():
    """Return path to slt if found in PATH, else None."""
    try:
        result = subprocess.run(
            ["which", "slt"],
            capture_output=True,
            text=True,
            check=False,
        )
        if result.returncode == 0 and result.stdout.strip():
            return result.stdout.strip()
    except (subprocess.SubprocessError, FileNotFoundError):
        pass
    return None


def download_slt_cli():
    """Download and extract SLT CLI tool to script directory."""
    platform_name, slt_cli_url = get_platform_vars()
    tools_folder_path = get_tools_path()
    slt_cli_path = os.path.join(tools_folder_path, "slt")

    if os.path.isfile(slt_cli_path):
        logging.info(f"SLT CLI already exists at {slt_cli_path}")
        return slt_cli_path

    logging.info("Downloading and unzipping slt-cli...")
    slt_zip_path = os.path.join(tools_folder_path, "slt.zip")
    try:
        dload.save(slt_cli_url, slt_zip_path)
        with ZipFile(slt_zip_path, 'r') as zObject:
            zObject.extractall(path=tools_folder_path)
        os.remove(slt_zip_path)
        make_executable(slt_cli_path)
        logging.info(f"SLT CLI installed at {slt_cli_path}")
        return slt_cli_path
    except Exception as e:
        logging.error(f"Failed to download/extract slt-cli: {e}")
        sys.exit(1)


def ensure_slt_available():
    """Ensure SLT CLI is available, either from PATH or by downloading."""
    slt_in_path = find_slt_in_path()
    if slt_in_path:
        logging.info(f"SLT already installed on system: {slt_in_path}")
        return slt_in_path
    return download_slt_cli()


def update_slt_cli(slt_cli_path):
    """Update SLT CLI to latest version."""
    update_cmd = [slt_cli_path, "update", "--self"]
    try:
        logging.info("Updating SLT CLI to latest version...")
        subprocess.run(update_cmd, check=True)
        logging.info("SLT CLI updated successfully")
    except subprocess.CalledProcessError as e:
        logging.warning(f"Failed to update slt-cli: {e}")
    except FileNotFoundError:
        logging.warning(f"SLT CLI not found at {slt_cli_path}, skipping update")


def get_sdk_pkg_path():
    """Return the path to sdk-pkg.slt file."""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    return os.path.join(script_dir, "sdk-pkg.slt")


def install_sdk_packages(slt_cli_path):
    """Install packages from sdk-pkg.slt file."""
    sdk_pkg_path = get_sdk_pkg_path()
    if not os.path.isfile(sdk_pkg_path):
        logging.error(f"sdk-pkg.slt not found at {sdk_pkg_path}")
        sys.exit(1)

    install_cmd = [slt_cli_path, "install", "-f", sdk_pkg_path]
    try:
        logging.info("Installing packages from sdk-pkg.slt...")
        subprocess.run(install_cmd, check=True)
        logging.info("Packages installed successfully")
    except subprocess.CalledProcessError as e:
        logging.error(f"Failed to install packages: {e}")
        sys.exit(1)


def slt_where(slt_cli_path, package):
    """Run 'slt where <package>' and return the path, or None if not found."""
    try:
        result = subprocess.run(
            [slt_cli_path, "where", package],
            capture_output=True,
            text=True,
            check=False,
        )
        if result.returncode == 0 and result.stdout.strip():
            return result.stdout.strip()
    except (subprocess.SubprocessError, FileNotFoundError):
        pass
    return None


def get_sdk_paths_gni_path():
    """Return the path to the SDK paths .gni file."""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    return os.path.join(script_dir, "sdk_paths.gni")


def _gni_escape(path):
    """Escape a path for use inside a GN double-quoted string."""
    if not path:
        return ""
    return path.replace("\\", "\\\\").replace('"', '\\"')


def write_sdk_paths_reference(slt_cli_path):
    """Run slt where for simplicity-sdk and wiseconnect and write paths to sdk_paths.gni."""
    simplicity_sdk_path = slt_where(slt_cli_path, "simplicity-sdk")
    wiseconnect_path = slt_where(slt_cli_path, "wiseconnect")

    if not simplicity_sdk_path:
        logging.warning("simplicity-sdk path not found, not written to reference file")
    if not wiseconnect_path:
        logging.warning("wiseconnect path not found, not written to reference file")

    gni_path = get_sdk_paths_gni_path()
    ef = _gni_escape(simplicity_sdk_path or "")
    wf = _gni_escape(wiseconnect_path or "")
    try:
        with open(gni_path, "w") as f:
            f.write("# SDK paths for efr32_sdk.gni (generated by install-packages.py).\n")
            f.write("# Run: python3 scripts/setup/silabs/install-packages.py\n\n")
            f.write(f'_efr32_sdk_root_from_setup = "{ef}"\n')
            f.write(f'_wifi_sdk_root_from_setup = "{wf}"\n')
        logging.info(f"SDK paths written to {gni_path}")
    except OSError as e:
        logging.error(f"Failed to write reference file {gni_path}: {e}")
        sys.exit(1)


def setup_slt_environment(verbose=False):
    """Main function to setup SLT CLI and install required packages."""
    setup_logging(verbose)

    slt_cli_path = ensure_slt_available()
    update_slt_cli(slt_cli_path)
    install_sdk_packages(slt_cli_path)
    write_sdk_paths_reference(slt_cli_path)

    return slt_cli_path


def main():
    parser = argparse.ArgumentParser(
        description="Download and setup SLT (Silicon Labs Tools) CLI for Matter development."
    )
    parser.add_argument(
        '--verbose', '-v',
        action='store_true',
        help='Enable verbose (debug) logging'
    )
    args = parser.parse_args()

    slt_path = setup_slt_environment(verbose=args.verbose)
    logging.info(f"\nSLT CLI setup completed successfully")
    logging.info(f"SLT CLI location: {slt_path}")


if __name__ == "__main__":
    main()
