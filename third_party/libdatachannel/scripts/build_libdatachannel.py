#!/usr/bin/env python3

import os
import shlex
import subprocess
from pathlib import Path

import click

# figure out paths for building
repo_dir = Path(__file__).resolve().parents[1] / "repo"


@click.command()
@click.option("--clang", is_flag=True, default=False, help="If specified, use clang instead of gcc")
@click.option("--build-dir", default=repo_dir / "build", show_default=True,
              help="Build directory for libdatachannel build")
@click.option("--cross-compile-cpu-type", default=None, help="CPU type for cross compilation if needed")
@click.option("--target-cc", default=None, help="C compiler for cross compilation (from args.gn)")
@click.option("--target-cxx", default=None, help="C++ compiler for cross compilation (from args.gn)")
def main(clang: bool, build_dir: str, cross_compile_cpu_type: str | None,
         target_cc: str | None, target_cxx: str | None):

    # Generate build files in build_dir
    cmake_cmd = [
        "cmake",
        "-S",
        str(repo_dir),
        "-B",
        str(build_dir),
        "-DUSE_GNUTLS=0",
        "-DUSE_NICE=0",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_CXX_FLAGS=-Wno-shadow",
    ]

    # Default compilers
    c_compiler = 'gcc'
    cxx_compiler = 'g++'
    if clang:
        c_compiler = 'clang'
        cxx_compiler = 'clang++'

    # Handle cross compilation
    if cross_compile_cpu_type:
        # Map GN cpu type to CMake processor
        cpu_to_processor = {
            "arm": "arm",
            "arm64": "aarch64",
            "x64": "x86_64",
            "x86": "i686",
        }
        processor = cpu_to_processor.get(cross_compile_cpu_type, cross_compile_cpu_type)

        cmake_cmd.extend(
            [
                "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
                "-DCMAKE_SYSTEM_NAME=Linux",
                f"-DCMAKE_SYSTEM_PROCESSOR={processor}",
                # Disable examples/tests during cross-compilation to avoid
                # linking issues with host system libraries (e.g., OpenSSL)
                "-DNO_EXAMPLES=ON",
                "-DNO_TESTS=ON",
            ]
        )

        if target_cc or target_cxx:
            if not (target_cc and target_cxx):
                raise click.UsageError("--target-cc and --target-cxx options need to be used together")

            # Use explicit compilers from args.gn
            c_compiler = target_cc
            cxx_compiler = target_cxx
        elif cross_compile_cpu_type == "arm64":
            sysroot = SysRootPath("SYSROOT_AARCH64")
            cmake_cmd.append(f"-DCMAKE_SYSROOT={sysroot}")

            if clang:
                # These defines are not used by gcc
                cmake_cmd.extend(
                    [
                        "-DCMAKE_C_COMPILER_TARGET=aarch64-linux-gnu",
                        "-DCMAKE_CXX_COMPILER_TARGET=aarch64-linux-gnu",
                    ]
                )
        else:
            raise click.UsageError(f"No sysroot and compiler targets defined for target_cpu '{cross_compile_cpu_type}'")

    # Set compilers for CMake
    cmake_cmd.extend(
        [
            f"-DCMAKE_C_COMPILER={c_compiler}",
            f"-DCMAKE_CXX_COMPILER={cxx_compiler}",
        ]
    )

    print(f"Running: {shlex.join(cmake_cmd)}")
    subprocess.run(cmake_cmd, check=True)

    make_cmd = ["cmake", "--build", str(build_dir), "--target", "datachannel-static", f"-j{os.cpu_count()}"]
    print(f"Running: {shlex.join(make_cmd)}")
    subprocess.run(make_cmd, check=True)

    print("libdatachannel build complete.")
    print(f"Artifacts are located in: {build_dir}")


def SysRootPath(name):
    if name not in os.environ:
        raise Exception(f'Missing environment variable "{name}"')
    return os.environ[name]


if __name__ == "__main__":
    main()
