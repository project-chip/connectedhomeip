#!/usr/bin/env python3
import os
import subprocess
import sys


def main():
    # Get the script directory
    script_dir = os.path.dirname(os.path.abspath(__file__))

    # Set the repo directory
    repo_dir = os.path.join(script_dir, "..", "repo")

    # Change to the repo directory
    os.chdir(repo_dir)
    print(f"Changed directory to: {repo_dir}")

    # Generate build files in ./build
    cmake_cmd = [
        "cmake",
        "-B", "build",
        "-DUSE_GNUTLS=0",
        "-DUSE_NICE=0",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_CXX_FLAGS=-Wno-shadow",
    ]

    print(f"Running: {' '.join(cmake_cmd)}")
    result = subprocess.run(cmake_cmd, check=True)

    # Change to build directory
    build_dir = os.path.join(repo_dir, "build")
    os.chdir(build_dir)
    print(f"Changed directory to: {build_dir}")

    # Build with Make
    make_cmd = ["make", "-j2"]
    print(f"Running: {' '.join(make_cmd)}")
    result = subprocess.run(make_cmd, check=True)

    print("libdatachannel build complete.")
    print(f"Artifacts are located in: {build_dir}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
