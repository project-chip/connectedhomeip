#!/usr/bin/env python3

#
# Copyright (c) 2023-2026 Project CHIP Authors
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

import argparse
import os
import subprocess
import sys


def fetch_telink_ble_sdk(repo_url, commit_hash):
    """
    Fetch Telink BLE SDK from specified repository and commit.

    Args:
        repo_url: Git repository URL for the BLE SDK
        commit_hash: Specific commit hash to checkout
    """
    zephyr_base = os.getenv("ZEPHYR_BASE")
    if not zephyr_base:
        raise RuntimeError("ZEPHYR_BASE environment variable not set")

    # fetch_sdk.sh is located in modules/hal/telink/hal_v2 under ZEPHYR_BASE
    sdk_script_dir = os.path.join(zephyr_base, "../modules/hal/telink/hal_v2")
    sdk_script = os.path.join(sdk_script_dir, "fetch_sdk.sh")

    print(f"ZEPHYR_BASE: {zephyr_base}")
    print(f"Looking for fetch_sdk.sh at: {sdk_script}")

    # Change to the directory containing fetch_sdk.sh
    os.chdir(sdk_script_dir)
    print(f"Current directory: {os.getcwd()}")
    print(f"Directory contents: {os.listdir('.')}")

    # Check if fetch_sdk.sh exists
    if not os.path.exists("fetch_sdk.sh"):
        raise RuntimeError(f"fetch_sdk.sh not found in {sdk_script_dir}")

    # Execute fetch_sdk.sh
    print(f"Fetching Telink BLE SDK with commit {commit_hash}...")
    # subprocess.run(["chmod", "+x", "fetch_sdk.sh"], check=True)
    subprocess.run(["./fetch_sdk.sh", repo_url, commit_hash], check=True)

    print("Telink BLE SDK fetch completed.")


def update_zephyr(remote_url, commit_hash):
    """
    Update Zephyr repository to specific commit.

    Args:
        remote_url: Git repository URL for Zephyr
        commit_hash: Specific commit hash to checkout
    """
    zephyr_base = os.getenv("ZEPHYR_BASE")
    if not zephyr_base:
        raise RuntimeError("ZEPHYR_BASE environment variable not set")

    remote_name = 'custom'

    # Check if remote already exists, if so, update it
    result = subprocess.run(['git', '-C', zephyr_base, 'remote', 'get-url', remote_name],
                            capture_output=True, text=True)
    if result.returncode == 0:
        # Remote exists, update its URL
        print(f"Remote '{remote_name}' already exists, updating URL...")
        subprocess.run(['git', '-C', zephyr_base, 'remote', 'set-url', remote_name, remote_url], check=True)
    else:
        # Remote doesn't exist, add it
        print(f"Adding remote: {remote_url}")
        subprocess.run(['git', '-C', zephyr_base, 'remote', 'add', remote_name, remote_url], check=True)

    # Fetch updates from remote
    # If commit_hash is a PR ref (e.g. pull/782/head), fetch it explicitly
    # since it lives under refs/pull/*/head and is not fetched by default.
    print(f"Fetching from remote: {remote_name}")
    if commit_hash.startswith('pull/') or commit_hash.startswith('refs/pull/'):
        ref = commit_hash if commit_hash.startswith('refs/') else f'refs/{commit_hash}'
        command = ['git', '-C', zephyr_base, 'fetch', remote_name, ref]
        subprocess.run(command, check=True)
        # Resolve the fetched ref to a commit hash for checkout
        result = subprocess.run(['git', '-C', zephyr_base, 'rev-parse', 'FETCH_HEAD'],
                                capture_output=True, text=True, check=True)
        checkout_target = result.stdout.strip()
    else:
        command = ['git', '-C', zephyr_base, 'fetch', remote_name]
        subprocess.run(command, check=True)
        checkout_target = commit_hash

    # Checkout the specified commit in detached HEAD state to avoid
    # disrupting any branch the user may currently have checked out.
    print(f"Checking out commit: {checkout_target}")
    command = ['git', '-C', zephyr_base, 'checkout', '--detach', checkout_target]
    subprocess.run(command, check=True)

    # Update west modules
    print("Updating west modules...")
    command = ['west', 'update', '-n', '-f', 'smart']
    subprocess.run(command, check=True)

    # Fetch blobs
    print("Fetching hal_telink blobs...")
    command = ['west', 'blobs', 'fetch', 'hal_telink']
    subprocess.run(command, check=True)


def main():
    """Main entry point for the script."""
    parser = argparse.ArgumentParser(
        description='Update Telink Zephyr to specific revision and fetch BLE SDK.'
    )
    parser.add_argument("remote",
                        nargs='?',
                        default="https://github.com/telink-semi/tl_zephyr",
                        help="New remote URL for the Zephyr repository.")
    parser.add_argument("hash",
                        help="Update Telink Zephyr to specific revision.")
    parser.add_argument("--sdk-repo",
                        default="https://github.com/telink-semi/tl_ble_sdk_zephyr.git",
                        help="Repository URL for Telink BLE SDK (default: tl_ble_sdk_zephyr.git)")
    parser.add_argument("--sdk-hash",
                        default="f50d422d780efb73af939650ef7b8c6bf5a0b99b",
                        help="Specific commit hash for BLE SDK")
    parser.add_argument("--skip-sdk",
                        action="store_true",
                        help="Skip fetching Telink BLE SDK")

    args = parser.parse_args()

    try:
        # Set ZEPHYR_BASE environment variable
        zephyr_base = os.getenv("ZEPHYR_BASE")
        if not zephyr_base:
            zephyr_base = os.getenv("TELINK_ZEPHYR_BASE")
            if zephyr_base:
                os.environ['ZEPHYR_BASE'] = zephyr_base

        if not zephyr_base:
            raise RuntimeError(
                "No ZEPHYR_BASE or TELINK_ZEPHYR_BASE environment variable found. "
                "Please set to a zephyr repository path."
            )

        # Update Zephyr repository
        print("=" * 60)
        print("Step 1: Updating Zephyr repository...")
        print("=" * 60)
        update_zephyr(args.remote, args.hash)

        # Fetch BLE SDK (unless skipped)
        if not args.skip_sdk:
            print("\n" + "=" * 60)
            print("Step 2: Fetching Telink BLE SDK...")
            print("=" * 60)
            fetch_telink_ble_sdk(args.sdk_repo, args.sdk_hash)
        else:
            print("\nSkipping BLE SDK fetch as requested.")

        print("\n" + "=" * 60)
        print("All operations completed successfully!")
        print("=" * 60)

    except (RuntimeError, subprocess.CalledProcessError) as e:
        print(f"\nError: {e}")
        sys.exit(1)


if __name__ == '__main__':
    main()
