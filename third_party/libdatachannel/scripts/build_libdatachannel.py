#!/usr/bin/env python3

import os
import subprocess

import click


@click.command()
@click.option('--clang', is_flag=True, default=False)
def main(clang: bool):
    # figure out paths for building
    script_dir = os.path.dirname(os.path.abspath(__file__))
    repo_dir = os.path.join(script_dir, "..", "repo")
    compiler_subdir = "clang" if clang else "gcc"
    build_dir = os.path.join(repo_dir, "build", compiler_subdir)

    # Generate build files in ./build
    cmake_cmd = [
        "cmake",
        "-B", f"build/{compiler_subdir}",
        "-DUSE_GNUTLS=0",
        "-DUSE_NICE=0",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_CXX_FLAGS=-Wno-shadow",
        "-DBUILD_SHARED_LIBS=OFF"
    ]

    if clang:
        cmake_cmd.extend([
            "-DCMAKE_C_COMPILER=clang",
            "-DCMAKE_CXX_COMPILER=clang++",
        ])
    else:
        cmake_cmd.extend([
            "-DCMAKE_C_COMPILER=gcc",
            "-DCMAKE_CXX_COMPILER=g++",
        ])

    print(f"Running: {' '.join(cmake_cmd)}")
    subprocess.run(cmake_cmd, cwd=repo_dir, check=True)

    # Build with Make
    make_cmd = ["make", "-j%d" % os.cpu_count()]
    print(f"Running: {' '.join(make_cmd)}")
    subprocess.run(make_cmd, cwd=build_dir,  check=True)

    print("libdatachannel build complete.")
    print(f"Artifacts are located in: {build_dir}")


if __name__ == "__main__":
    main()
