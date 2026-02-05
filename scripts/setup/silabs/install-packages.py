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

logger = logging.getLogger(__name__)

try:
    import dload
except ImportError:
    logger.error("dload package is required. Install it with: pip install dload")
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
        logger.error("Platform %s is not supported (Linux and macOS only)", platform)
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
            logger.warning("Could not make %s executable: %s", path, e)
    else:
        logger.warning("Path %s does not exist to make executable.", path)


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
        logger.info("SLT CLI already exists at %s", slt_cli_path)
        return slt_cli_path

    logger.info("Downloading and unzipping slt-cli...")
    slt_zip_path = os.path.join(tools_folder_path, "slt.zip")
    try:
        dload.save(slt_cli_url, slt_zip_path)
        with ZipFile(slt_zip_path, 'r') as zObject:
            zObject.extractall(path=tools_folder_path)
        os.remove(slt_zip_path)
        make_executable(slt_cli_path)
        logger.info("SLT CLI installed at %s", slt_cli_path)
        return slt_cli_path
    except Exception as e:
        logger.error("Failed to download/extract slt-cli: %s", e)
        sys.exit(1)


def ensure_slt_available():
    """Ensure SLT CLI is available, either from PATH or by downloading."""
    slt_in_path = find_slt_in_path()
    if slt_in_path:
        logger.info("SLT already installed on system: %s", slt_in_path)
        return slt_in_path
    return download_slt_cli()


def update_slt_cli(slt_cli_path):
    """Update SLT CLI to latest version."""
    update_cmd = [slt_cli_path, "update", "--self"]
    try:
        logger.info("Updating SLT CLI to latest version...")
        subprocess.run(update_cmd, check=True)
        logger.info("SLT CLI updated successfully")
    except subprocess.CalledProcessError as e:
        logger.warning("Failed to update slt-cli: %s", e)
    except FileNotFoundError:
        logger.warning("SLT CLI not found at %s, skipping update", slt_cli_path)


def get_sdk_pkg_path():
    """Return the path to sdk-pkg.slt file."""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    return os.path.join(script_dir, "sdk-pkg.slt")


def install_sdk_packages(slt_cli_path):
    """Install packages from sdk-pkg.slt file."""
    sdk_pkg_path = get_sdk_pkg_path()
    if not os.path.isfile(sdk_pkg_path):
        logger.error("sdk-pkg.slt not found at %s", sdk_pkg_path)
        sys.exit(1)

    install_cmd = [slt_cli_path, "install", "-f", sdk_pkg_path]
    try:
        logger.info("Installing packages from sdk-pkg.slt...")
        subprocess.run(install_cmd, check=True)
        logger.info("Packages installed successfully")
    except subprocess.CalledProcessError as e:
        logger.error("Failed to install packages: %s", e)
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


def get_repo_root():
    """Return the repository root (three levels up from this script)."""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    return os.path.normpath(os.path.join(script_dir, "..", "..", ".."))


def create_sdk_symlinks(simplicity_sdk_path, wiseconnect_path):
    """Create symlinks: third_party/silabs/simplicity_sdk and wifi_sdk to SLT SDK locations."""
    repo_root = get_repo_root()
    silabs_dir = os.path.join(repo_root, "third_party", "silabs")

    def create_symlink(target_path, link_name):
        if not target_path or not os.path.isdir(target_path):
            logger.warning("Target path does not exist or is not a directory: %s", target_path)
            return
        link_path = os.path.join(silabs_dir, link_name)
        try:
            os.makedirs(silabs_dir, exist_ok=True)
            if os.path.lexists(link_path):
                if os.path.islink(link_path):
                    current = os.path.realpath(link_path)
                    if os.path.realpath(target_path) == current:
                        logger.info("Symlink already up to date: %s", link_path)
                        return
                    os.remove(link_path)
                else:
                    logger.warning("Path exists and is not a symlink, skipping: %s", link_path)
                    return
            os.symlink(target_path, link_path)
            logger.info("Created symlink %s -> %s", link_path, target_path)
        except OSError as e:
            logger.warning("Could not create symlink %s: %s", link_path, e)

    create_symlink(simplicity_sdk_path, "simplicity_sdk")
    create_symlink(wiseconnect_path, "wifi_sdk")


def setup_slt_environment(verbose=False):
    """Main function to setup SLT CLI and install required packages."""
    setup_logging(verbose)

    slt_cli_path = ensure_slt_available()
    update_slt_cli(slt_cli_path)
    install_sdk_packages(slt_cli_path)

    simplicity_sdk_path = slt_where(slt_cli_path, "simplicity-sdk")
    wiseconnect_path = slt_where(slt_cli_path, "wiseconnect")
    create_sdk_symlinks(simplicity_sdk_path, wiseconnect_path)

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
    logger.info("\nSLT CLI setup completed successfully")
    logger.info("SLT CLI location: %s", slt_path)


if __name__ == "__main__":
    main()
