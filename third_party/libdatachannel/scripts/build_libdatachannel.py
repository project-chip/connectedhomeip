#!/usr/bin/env python3
import os
import subprocess
import sys
import optparse

def main(argv):
    parser = optparse.OptionParser()
    parser.add_option('--target_cpu_type', type=str, default='x64')

    #get arguments
    options, _ = parser.parse_args(argv)
    target_cpu_type = options.target_cpu_type
    print(f"Target CPU: {target_cpu_type}")

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
        "-B",
        f"build-{target_cpu_type}",
        "-DUSE_GNUTLS=0",
        "-DUSE_NICE=0",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_CXX_FLAGS=-Wno-shadow",
        "-DBUILD_SHARED_LIBS=OFF"
    ]

    if target_cpu_type.startswith('arm'):
        sysroot_aarch64 = SysRootPath('SYSROOT_AARCH64')
        cmake_cmd = [
            "cmake",
            "-B",
            f"build-{target_cpu_type}",
            "-G", "Ninja",
            "-DUSE_GNUTLS=0",
            "-DUSE_NICE=0",
            "-DCMAKE_BUILD_TYPE=Release",
            "-DCMAKE_CXX_FLAGS=-Wno-shadow",
            "-DBUILD_SHARED_LIBS=OFF",
            "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
            "-DCMAKE_SYSTEM_NAME=Linux",
            "-DCMAKE_SYSTEM_PROCESSOR=aarch64",
            "-DCMAKE_C_COMPILER=clang",
            "-DCMAKE_CXX_COMPILER=clang++",
            "-DCMAKE_C_COMPILER_TARGET=aarch64-linux-gnu",
            "-DCMAKE_CXX_COMPILER_TARGET=aarch64-linux-gnu",
            f"-DCMAKE_SYSROOT={sysroot_aarch64}",
        ]

    print(f"Running: {' '.join(cmake_cmd)}")
    result = subprocess.run(cmake_cmd, check=True)

    # Change to build directory
    build_dir = os.path.join(repo_dir, f"build-{target_cpu_type}")
    os.chdir(build_dir)
    print(f"Changed directory to: {build_dir}")

    # Build with Make
    build_cmd = ["make", f"-j{os.cpu_count()}"]
    if target_cpu_type.startswith('arm'):
        #build with ninja
        build_cmd = ["ninja", "-C", "."]

    print(f"Running: {' '.join(build_cmd)}")
    result = subprocess.run(build_cmd, check=True)

    print("libdatachannel build complete.")
    print(f"Artifacts are located in: {build_dir}")

    return 0

def SysRootPath(name):
    if name not in os.environ:
        raise Exception('Missing environment variable "%s"' % name)
    return os.environ[name]


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
