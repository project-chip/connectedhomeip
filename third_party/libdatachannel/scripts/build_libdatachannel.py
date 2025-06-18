#!/usr/bin/env python3

import os
import subprocess

import click


@click.command()
@click.option("--clang", is_flag=True, default=False, help="If specified, use clang instead of gcc")
@click.option("--output-dir", default="build", help="Output directory for libdatachannel build")
@click.option("--cross-compile-cpu-type", default=None, help="CPU type for cross compilation if needed")
def main(clang: bool, output_dir: str, cross_compile_cpu_type: str | None):
    # figure out paths for building
    script_dir = os.path.dirname(os.path.abspath(__file__))
    repo_dir = os.path.join(script_dir, "..", "repo")
    build_dir = os.path.join(repo_dir, output_dir)

    # Generate build files in ./build
    cmake_cmd = [
        "cmake",
        "-B",
        build_dir,
        "-DUSE_GNUTLS=0",
        "-DUSE_NICE=0",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_CXX_FLAGS=-Wno-shadow",
        "-DBUILD_SHARED_LIBS=OFF",
    ]

    if clang:
        cmake_cmd.extend(
            [
                "-DCMAKE_C_COMPILER=clang",
                "-DCMAKE_CXX_COMPILER=clang++",
            ]
        )
    else:
        cmake_cmd.extend(
            [
                "-DCMAKE_C_COMPILER=gcc",
                "-DCMAKE_CXX_COMPILER=g++",
            ]
        )

    if cross_compile_cpu_type and cross_compile_cpu_type == "arm64":
        sysroot_aarch64 = SysRootPath("SYSROOT_AARCH64")
        cmake_cmd.extend(
            [
                "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
                "-DCMAKE_SYSTEM_NAME=Linux",
                "-DCMAKE_SYSTEM_PROCESSOR=aarch64",
                "-DCMAKE_C_COMPILER_TARGET=aarch64-linux-gnu",
                "-DCMAKE_CXX_COMPILER_TARGET=aarch64-linux-gnu",
                f"-DCMAKE_SYSROOT={sysroot_aarch64}",
            ]
        )

    print(f"Running: {' '.join(cmake_cmd)}")
    subprocess.run(cmake_cmd, cwd=repo_dir, check=True)

    # Build with Make
    make_cmd = ["make", "-j%d" % os.cpu_count()]
    print(f"Running: {' '.join(make_cmd)}")
    subprocess.run(make_cmd, cwd=build_dir, check=True)

    print("libdatachannel build complete.")
    print(f"Artifacts are located in: {build_dir}")


def SysRootPath(name):
    if name not in os.environ:
        raise Exception(f'Missing environment variable "{name}"')
    return os.environ[name]


if __name__ == "__main__":
    main()
