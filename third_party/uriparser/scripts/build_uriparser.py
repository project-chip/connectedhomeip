#!/usr/bin/env python3
import os
import subprocess

import click


@click.command()
@click.option("--clang", is_flag=True, default=False, help="Use clang instead of gcc")
@click.option("--output-dir", default="build", help="Output directory for uriparser build")
def main(clang, output_dir):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    repo_dir = os.path.join(script_dir, "..", "repo")
    build_dir = os.path.join(repo_dir, output_dir)

    # CMake command
    cmake_cmd = [
        "cmake",
        "-B", build_dir,
        "-DCMAKE_BUILD_TYPE=Release",
        "-DBUILD_SHARED_LIBS=OFF",
        "-DURIPARSER_BUILD_TESTS=OFF",
        "-DURIPARSER_BUILD_DOCS=OFF"
    ]

    if clang:
        cmake_cmd += ["-DCMAKE_C_COMPILER=clang", "-DCMAKE_CXX_COMPILER=clang++"]
    else:
        cmake_cmd += ["-DCMAKE_C_COMPILER=gcc", "-DCMAKE_CXX_COMPILER=g++"]

    print("Running:", " ".join(cmake_cmd))
    subprocess.run(cmake_cmd, cwd=repo_dir, check=True)

    # Build
    make_cmd = ["make", "-j", str(os.cpu_count())]
    print("Running:", " ".join(make_cmd))
    subprocess.run(make_cmd, cwd=build_dir, check=True)

    print("uriparser build complete at", build_dir)


if __name__ == "__main__":
    main()
