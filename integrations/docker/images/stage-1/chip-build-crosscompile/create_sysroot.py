#!/usr/bin/env -S uv run
# /// script
# requires-python = ">=3.11"
# dependencies = [
#     "click",
#     "coloredlogs",
#     "tqdm",
# ]
# ///

import glob
import logging
import os
import shutil
import subprocess
import sys
import tempfile

import click
import coloredlogs
from tqdm import tqdm

logger = logging.getLogger(__name__)


def check_prerequisites(install_prereqs):
    """Checks if 'debootstrap' is installed.

    If install_prereqs is True, attempts to install it using apt-get.
    Exits the script if debootstrap is not found.
    """
    if install_prereqs:
        logger.info("Attempting to install debootstrap...")
        subprocess.run(["sudo", "apt-get", "update"], check=True)
        subprocess.run(
            ["sudo", "apt-get", "install", "-y", "debootstrap"], check=True
        )

    if not shutil.which("debootstrap"):
        logger.error("Cannot find 'debootstrap'. Please install it or use -i.")
        sys.exit(1)


def run_stage_1(arch, suite, mirror, full_dir, packages_str):
    """Stage 1: Runs debootstrap to create a full minimal rootfs.

    Downloads and extracts base system and specified packages in foreign mode.
    Changes ownership of the created directory to the current user.
    """
    logger.info(
        f"=== Stage 1: Running debootstrap for {arch} in {full_dir} ==="
    )
    logger.info(f"Cleaning up full directory {full_dir} ...")
    if os.path.exists(full_dir):
        subprocess.run(["sudo", "rm", "-rf", full_dir], check=True)
    os.makedirs(full_dir)

    subprocess.run(
        [
            "sudo",
            "debootstrap",
            "--foreign",
            f"--arch={arch}",
            "--components=main,universe",
            f"--include={packages_str}",
            suite,
            full_dir,
            mirror,
        ],
        check=True,
    )

    logger.info(f"Changing ownership of {full_dir} to current user ...")
    import grp
    import pwd

    user = os.environ.get("USER")
    if not user:
        user = pwd.getpwuid(os.getuid()).pw_name

    group = grp.getgrgid(os.getgid()).gr_name
    subprocess.run(
        ["sudo", "chown", "-R", f"{user}:{group}", full_dir], check=True
    )


def extract_debs(full_dir, sysroot_dir):
    """Extracts additional packages from the full rootfs cache.

    Extracts each .deb into a clean temporary directory first, and then safely
    merges the contents into the sysroot using rsync --keep-dirlinks to preserve
    merged-usr symlinks and avoid destructive overwrites.
    """
    logger.info("Copying packages to temporary directory...")
    with tempfile.TemporaryDirectory() as tmp_deb_dir:
        deb_files = glob.glob(
            os.path.join(full_dir, "var/cache/apt/archives/*.deb")
        )
        for deb in deb_files:
            shutil.copy(deb, tmp_deb_dir)

        logger.info("Extracting additional packages...")
        extracted_debs = glob.glob(os.path.join(tmp_deb_dir, "*.deb"))
        abs_sysroot_dir = os.path.abspath(sysroot_dir)

        for deb in tqdm(extracted_debs, desc="Extracting packages"):
            logger.debug(f"Extracting {os.path.basename(deb)} ...")
            with tempfile.TemporaryDirectory() as tmp_extract_dir:
                subprocess.run(
                    ["ar", "x", os.path.abspath(deb)],
                    cwd=tmp_extract_dir,
                    check=True,
                )

                data_tar = None
                for f in os.listdir(tmp_extract_dir):
                    if f.startswith("data.tar"):
                        data_tar = f
                        break

                if data_tar:
                    # 1. Extract into a clean staging directory
                    staging_dir = os.path.join(tmp_extract_dir, "staging")
                    os.makedirs(staging_dir)
                    subprocess.run(
                        ["tar", "xf", data_tar, "-C", staging_dir],
                        cwd=tmp_extract_dir,
                        check=True,
                    )

                    # 2. Safely merge into sysroot preserving directory symlinks
                    subprocess.run(
                        [
                            "rsync",
                            "-a",
                            "--keep-dirlinks",
                            f"{staging_dir}/",
                            f"{abs_sysroot_dir}/",
                        ],
                        check=True,
                    )
                else:
                    logger.warning(
                        f"No data.tar.* found in {os.path.basename(deb)}"
                    )


def ensure_lib_symlink(sysroot_dir):
    """Ensures lib exists and is a symlink to usr/lib.

    Handles cases where it was overwritten as a directory or completely deleted.
    """
    lib_path = os.path.join(sysroot_dir, "lib")
    usr_lib_path = os.path.join(sysroot_dir, "usr", "lib")

    if os.path.islink(lib_path):
        target = os.readlink(lib_path)
        if target == "usr/lib":
            logger.info(f"Symlink {lib_path} -> usr/lib is correct.")
            return
        logger.warning(
            f"Symlink {lib_path} points to {target}, expected usr/lib. Fixing..."
        )
        os.unlink(lib_path)
        os.symlink("usr/lib", lib_path)
        return

    if os.path.isdir(lib_path):
        logger.warning(
            f"Conflict detected: {lib_path} is a directory. Merging into {usr_lib_path} ..."
        )
        os.makedirs(usr_lib_path, exist_ok=True)
        for item in os.listdir(lib_path):
            src = os.path.join(lib_path, item)
            subprocess.run(["cp", "-a", src, usr_lib_path], check=True)

        logger.info(f"Removing {lib_path} directory ...")
        shutil.rmtree(lib_path)

        logger.info(f"Recreating symlink {lib_path} -> usr/lib ...")
        os.symlink("usr/lib", lib_path)
        return

    if os.path.exists(lib_path):
        logger.warning(
            f"Conflict detected: {lib_path} exists but is not a dir or symlink. Removing..."
        )
        os.unlink(lib_path)

    logger.info(f"Creating missing symlink {lib_path} -> usr/lib ...")
    os.symlink("usr/lib", lib_path)


def cleanup_sysroot(sysroot_dir):
    """Cleans up unneeded directories in the sysroot.

    Keeps only 'usr' and 'lib' (which points to 'usr/lib').
    """
    logger.info(f"Cleaning up unneeded directories in {sysroot_dir} ...")
    for item in os.listdir(sysroot_dir):
        if item not in ["usr", "lib"]:
            path = os.path.join(sysroot_dir, item)
            logger.info(f"Deleting unneeded item: {path}")
            if os.path.isdir(path) and not os.path.islink(path):
                shutil.rmtree(path)
            else:
                os.unlink(path)

    # Clean up large unneeded subdirectories inside usr/lib to save space
    # Remove executables as we don't need them for the build and they just confuse `cmake`
    excludes = [
        "usr/lib/*/dri",
        "usr/lib/firmware",
        "usr/lib/git-core",
        "usr/lib/modules",
        "usr/lib/ssl/private",
        "usr/lib/systemd",

        "usr/bin",
        "usr/sbin"
    ]
    for pattern in excludes:
        full_pattern = os.path.join(sysroot_dir, pattern)
        for path in glob.glob(full_pattern):
            logger.info(f"Deleting excluded large item: {path}")
            if os.path.isdir(path) and not os.path.islink(path):
                shutil.rmtree(path)
            else:
                os.unlink(path)


def fix_symlinks(sysroot_dir):
    """Fixes absolute symlinks in the sysroot to make them relative.

    Iterates over all symlinks, and if they point to an absolute path,
    converts them to be relative to the sysroot directory.
    Skips special filesystems like /proc, /sys, /dev.
    """
    logger.info(f"Fixing absolute symlinks in {sysroot_dir} ...")
    abs_sysroot_dir = os.path.abspath(sysroot_dir)
    for root, dirs, files in os.walk(sysroot_dir):
        for name in files + dirs:
            path = os.path.join(root, name)
            if os.path.islink(path):
                target = os.readlink(path)
                if target.startswith("/"):
                    if (
                        target.startswith("/proc/")
                        or target.startswith("/sys/")
                        or target.startswith("/dev/")
                    ):
                        logger.info(
                            f"Skipping special filesystem link {path} -> {target}"
                        )
                        continue

                    abs_target = os.path.join(
                        abs_sysroot_dir, target.lstrip("/")
                    )
                    rel_target = os.path.relpath(
                        abs_target, os.path.dirname(path)
                    )

                    logger.info(
                        f"Updating link {path}: {target} -> {rel_target}"
                    )
                    os.unlink(path)
                    os.symlink(rel_target, path)


def run_stage_2(full_dir, sysroot_dir):
    """Stage 2: Orchestrates extraction, cleanup, and symlink fixup.

    Creates the clean sysroot directory, copies base files from the full rootfs,
    and calls helper functions to extract packages, clean up, and fix symlinks.
    """
    logger.info(f"=== Stage 2: Creating sysroot in {sysroot_dir} ===")

    if not os.path.isdir(full_dir):
        logger.error(
            f"Error: Stage 1 output directory {full_dir} does not exist. Run Stage 1 first or enable it."
        )
        sys.exit(1)

    logger.info(f"Cleaning up sysroot directory {sysroot_dir} ...")
    if os.path.exists(sysroot_dir):
        subprocess.run(["rm", "-rf", sysroot_dir], check=True)
    os.makedirs(sysroot_dir)

    logger.info("Pre-creating merged-usr base structure ...")
    for d in ["bin", "lib", "sbin"]:
        os.makedirs(os.path.join(sysroot_dir, f"usr/{d}"), exist_ok=True)
        symlink_path = os.path.join(sysroot_dir, d)
        if not os.path.exists(symlink_path):
            os.symlink(f"usr/{d}", symlink_path)
            logger.info(f"Created symlink {d} -> usr/{d} in {sysroot_dir}")

    logger.info(f"Copying base system files from {full_dir} ...")
    os.makedirs(os.path.join(sysroot_dir, "usr/include"), exist_ok=True)
    subprocess.run(
        [
            "cp",
            "-a",
            os.path.join(full_dir, "usr/include/."),
            os.path.join(sysroot_dir, "usr/include/"),
        ],
        check=True,
    )
    subprocess.run(
        [
            "cp",
            "-a",
            os.path.join(full_dir, "usr/lib/."),
            os.path.join(sysroot_dir, "usr/lib/"),
        ],
        check=True,
    )

    extract_debs(full_dir, sysroot_dir)
    ensure_lib_symlink(sysroot_dir)
    cleanup_sysroot(sysroot_dir)
    fix_symlinks(sysroot_dir)


@click.command()
@click.option(
    "-a",
    "--arch",
    required=True,
    type=click.Choice(["arm64", "armhf"]),
    help="Architecture",
)
@click.option(
    "-s",
    "--skip-debootstrap",
    is_flag=True,
    help="Skip Stage 1 (Debootstrap) and start at Stage 2",
)
@click.option(
    "-i",
    "--install-prereqs",
    is_flag=True,
    help="Attempt to install prerequisites (debootstrap)",
)
def main(arch, skip_debootstrap, install_prereqs):
    """Main entry point. Parses arguments and runs the stages."""
    coloredlogs.install(level='INFO', fmt='%(asctime)s %(levelname)s %(message)s')
    suite = "noble"
    version = "24.04"
    dir_arch = "aarch64" if arch == "arm64" else arch

    if arch == "arm64" or arch == "armhf":
        mirror = "http://ports.ubuntu.com/ubuntu-ports"
    else:
        mirror = "http://archive.ubuntu.com/ubuntu"

    full_dir = f"ubuntu-{version}-{dir_arch}-full"
    sysroot_dir = f"ubuntu-{version}-{dir_arch}-sysroot"

    packages = [
        "ffmpeg",
        "gstreamer1.0-plugins-base",
        "gstreamer1.0-plugins-good",
        "gstreamer1.0-plugins-ugly",
        "gstreamer1.0-tools",
        "libavahi-client-dev",
        "libavcodec-dev",
        "libavformat-dev",
        "libavutil-dev",
        "libcairo2-dev",
        "libcurl4-openssl-dev",
        "libdbus-1-dev",
        "libgirepository1.0-dev",
        "libglib2.0-dev",
        "libgstreamer1.0-0",
        "libgstreamer1.0-dev",
        "libgstreamer-plugins-base1.0-dev",
        "libpcsclite-dev",
        "libreadline-dev",
        "libsdl2-dev",
        "libssl-dev",
    ]
    packages_str = ",".join(packages)

    check_prerequisites(install_prereqs)

    if not skip_debootstrap:
        run_stage_1(arch, suite, mirror, full_dir, packages_str)
    else:
        logger.info("Skipping debootstrap step as requested.")
        if not os.path.isdir(full_dir):
            logger.error(
                f"Error: Stage 1 output directory {full_dir} does not exist. Cannot skip debootstrap."
            )
            sys.exit(1)

    run_stage_2(full_dir, sysroot_dir)

    logger.info("DONE")
    logger.info(f"Sysroot created in {sysroot_dir}")


if __name__ == "__main__":
    main()
