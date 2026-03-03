#!/usr/bin/env python3

"""
Download and setup SLT (Silicon Labs Tools) CLI for Matter development.

This script downloads and installs the SLT CLI tool which is used to manage
Silicon Labs development tools required for Matter development.
"""

import argparse
import logging
import os
import re
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


def get_install_done_marker_path():
    """Return path to marker file indicating install-packages has completed successfully."""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    return os.path.join(script_dir, ".install-packages-done")


def get_files_slt_dir():
    """Return path to chip-build-efr32 files-slt directory."""
    repo_root = get_repo_root()
    return os.path.join(
        repo_root, "integrations", "docker", "images", "stage-2", "chip-build-efr32", "files-slt"
    )


def parse_version_from_slt(file_path):
    """Extract dependency version from a .slt file (version = \"X\" in [dependency] block). Returns None if missing."""
    if not file_path or not os.path.isfile(file_path):
        return None
    pattern = re.compile(r'version\s*=\s*"([^"]+)"')
    try:
        with open(file_path, "r") as f:
            content = f.read()
    except OSError:
        return None
    matches = pattern.findall(content)
    for m in matches:
        if m != "0" and "." in m:
            return m.split("@")[0].strip()
    return None


def version_tuple(version_str):
    """Convert version string to tuple of integers for comparison (e.g. 2025.12.1-alpha -> (2025, 12, 1))."""
    if not version_str:
        return ()
    main = version_str.split("-")[0].split("+")[0]
    parts = []
    for p in main.split("."):
        try:
            parts.append(int(p))
        except ValueError:
            parts.append(0)
    return tuple(parts)


def version_greater(a, b):
    """Return True if version string a is greater than b."""
    ta = version_tuple(a)
    tb = version_tuple(b)
    return ta > tb


def read_install_done_versions():
    """Read simplicity_sdk_version and wiseconnect_version from .install-packages-done. Returns None if missing."""
    path = get_install_done_marker_path()
    if not os.path.isfile(path):
        return None
    versions = {}
    try:
        with open(path, "r") as f:
            for line in f:
                line = line.strip()
                if "=" in line:
                    key, _, value = line.partition("=")
                    versions[key.strip()] = value.strip()
    except OSError:
        return None
    simplicity = versions.get("simplicity_sdk_version")
    wiseconnect = versions.get("wiseconnect_version")
    if simplicity is None or wiseconnect is None:
        return None
    return {"simplicity_sdk_version": simplicity, "wiseconnect_version": wiseconnect}


def check_skip_if_up_to_date():
    """
    If .install-packages-done exists, compare its versions to sisdk-pkg.slt and wiseconnect-pkg.slt.
    If both .slt requested versions are not greater than installed, exit 0 (skip execution).
    """
    marker_path = get_install_done_marker_path()
    if not os.path.isfile(marker_path):
        return

    done = read_install_done_versions()
    if done is None:
        return

    files_slt_dir = get_files_slt_dir()
    sisdk_slt = os.path.join(files_slt_dir, "sisdk-pkg.slt")
    wiseconnect_slt = os.path.join(files_slt_dir, "wiseconnect-pkg.slt")

    slt_simplicity = parse_version_from_slt(sisdk_slt)
    slt_wiseconnect = parse_version_from_slt(wiseconnect_slt)

    if slt_simplicity is None:
        logger.error("Could not parse version from %s", sisdk_slt)
        sys.exit(1)
    if slt_wiseconnect is None:
        logger.error("Could not parse version from %s", wiseconnect_slt)
        sys.exit(1)

    need_simplicity = version_greater(slt_simplicity, done["simplicity_sdk_version"])
    need_wiseconnect = version_greater(slt_wiseconnect, done["wiseconnect_version"])

    if not need_simplicity and not need_wiseconnect:
        logger.info(
            "Installed SDK versions (%s, %s) are already >= requested in .slt (%s, %s). Skipping.",
            done["simplicity_sdk_version"],
            done["wiseconnect_version"],
            slt_simplicity,
            slt_wiseconnect,
        )
        sys.exit(0)


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
            # Check for path traversal vulnerabilities before extracting
            for member in zObject.infolist():
                resolved_path = os.path.realpath(os.path.join(tools_folder_path, member.filename))
                if not resolved_path.startswith(os.path.realpath(tools_folder_path)):
                    logger.error("Zip file contains unsafe path: %s", member.filename)
                    sys.exit(1)
            # If all paths are safe, extract
            zObject.extractall(path=tools_folder_path)
        os.remove(slt_zip_path)
        make_executable(slt_cli_path)
        logger.info("SLT CLI installed at %s", slt_cli_path)
        return slt_cli_path
    except Exception as e:
        logger.error("Failed to download/extract slt-cli: %s", e)
        sys.exit(1)


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


def get_pkg_manifest_paths():
    """Return paths to sisdk-pkg.lock and wiseconnect-pkg.slt from chip-build-efr32 files-slt."""
    repo_root = get_repo_root()
    files_slt_dir = os.path.join(
        repo_root, "integrations", "docker", "images", "stage-2", "chip-build-efr32", "files-slt"
    )
    return [
        os.path.join(files_slt_dir, "wiseconnect-pkg.slt"),
        os.path.join(files_slt_dir, "sisdk-pkg.lock"),
    ]


def install_sdk_packages(slt_cli_path):
    """Install packages from sisdk-pkg.lock and wiseconnect-pkg.slt."""
    for pkg_path in get_pkg_manifest_paths():
        if not os.path.isfile(pkg_path):
            logger.error("Package manifest not found at %s", pkg_path)
            sys.exit(1)

    for pkg_path in get_pkg_manifest_paths():
        install_cmd = [slt_cli_path, "install", "-f", pkg_path]
        try:
            logger.info("Installing packages from %s...", os.path.basename(pkg_path))
            subprocess.run(install_cmd, check=True)
            logger.info("Packages from %s installed successfully", os.path.basename(pkg_path))
        except subprocess.CalledProcessError as e:
            logger.error("Failed to install packages from %s: %s", pkg_path, e)
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


SILABS_SDK_LINK_NAMES = ("simplicity_sdk", "wifi_sdk")


def is_git_submodule_checkout(dir_path):
    """Return True if dir_path is a directory that is a git submodule checkout (e.g. from older installs)."""
    if not dir_path or not os.path.isdir(dir_path) or os.path.islink(dir_path):
        return False
    git_file = os.path.join(dir_path, ".git")
    if not os.path.isfile(git_file):
        return False
    try:
        with open(git_file, "r") as f:
            return f.read().strip().startswith("gitdir:")
    except OSError:
        return False


def get_silabs_paths_as_submodules(repo_root):
    """Return which of third_party/silabs/simplicity_sdk and wifi_sdk exist as submodule checkouts on disk."""
    found = []
    for name in SILABS_SDK_LINK_NAMES:
        path = os.path.join(repo_root, "third_party", "silabs", name)
        if is_git_submodule_checkout(path):
            found.append(name)
    return found


def check_silabs_not_submodules(repo_root):
    """Exit with instructions if simplicity_sdk or wifi_sdk exist as submodule checkouts (e.g. from older installs)."""
    submodule_names = get_silabs_paths_as_submodules(repo_root)
    if not submodule_names:
        return

    paths = [os.path.join("third_party", "silabs", n) for n in submodule_names]
    logger.error(
        "The following paths are submodule checkouts (e.g. from older installs): %s. "
        "This script expects to create symlinks here, not use submodules.",
        ", ".join(paths),
    )
    logger.error(
        "Remove the submodules (e.g. git submodule deinit third_party/silabs/simplicity_sdk third_party/silabs/wifi_sdk; "
        "git rm third_party/silabs/simplicity_sdk third_party/silabs/wifi_sdk), then re-run this script."
    )
    sys.exit(1)


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


def parse_key_from_file(file_path, key):
    """Return the value for the first line 'key: value' in file_path, or None if missing."""
    if not file_path or not os.path.isfile(file_path):
        return None
    prefix = key + ": "
    try:
        with open(file_path, "r") as f:
            for line in f:
                line = line.strip()
                if line.startswith(prefix):
                    return line[len(prefix):].strip()
    except OSError:
        pass
    return None


def get_installed_sdk_versions(repo_root):
    """Read sdk_version from simplicity_sdk.slcs and version from wiseconnect.slce under third_party/silabs.
    Exits with an error if either file or key is missing.
    """
    silabs = os.path.join(repo_root, "third_party", "silabs")
    simplicity_slcs = os.path.join(silabs, "simplicity_sdk", "simplicity_sdk.slcs")
    wiseconnect_slce = os.path.join(silabs, "wifi_sdk", "wiseconnect.slce")

    simplicity_sdk_version = parse_key_from_file(simplicity_slcs, "sdk_version")
    wiseconnect_version = parse_key_from_file(wiseconnect_slce, "version")

    missing = []
    if simplicity_sdk_version is None:
        missing.append("sdk_version from %s" % simplicity_slcs)
    if wiseconnect_version is None:
        missing.append("version from %s" % wiseconnect_slce)
    if missing:
        logger.error("Could not read required version fields: %s", "; ".join(missing))
        sys.exit(1)

    return {
        "simplicity_sdk_version": simplicity_sdk_version,
        "wiseconnect_version": wiseconnect_version,
    }


def write_install_done_marker(versions):
    """Write .install-packages-done with installed SDK versions."""
    marker_path = get_install_done_marker_path()
    lines = [f"{key}={value}" for key, value in versions.items()]
    try:
        with open(marker_path, "w") as f:
            f.write("\n".join(lines))
            f.write("\n")
    except OSError as e:
        logger.error("Could not write install-done marker: %s", e)
        sys.exit(1)


def setup_slt_environment(verbose=False):
    """Main function to setup SLT CLI and install required packages."""
    setup_logging(verbose)

    check_skip_if_up_to_date()

    slt_cli_path = download_slt_cli()
    update_slt_cli(slt_cli_path)
    install_sdk_packages(slt_cli_path)

    repo_root = get_repo_root()
    check_silabs_not_submodules(repo_root)

    simplicity_sdk_path = slt_where(slt_cli_path, "simplicity-sdk/2025.12.1-alpha")
    wiseconnect_path = slt_where(slt_cli_path, "wiseconnect")
    create_sdk_symlinks(simplicity_sdk_path, wiseconnect_path)

    versions = get_installed_sdk_versions(repo_root)
    write_install_done_marker(versions)

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
