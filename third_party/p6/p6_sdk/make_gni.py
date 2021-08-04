#!/usr/bin/env python3

import os, sys, subprocess, json, argparse, shutil
from pathlib import Path

# De-duplicate and strip whitespace from a list of strings
def dedup_and_sanitize(lst):
    # There is no build-in python method for de-duping lists but converting
    # list entries to dict keys and converting back is the easiest way.
    # Note: dicts (as of python3.7) preserve insertion order (use OrderedDict
    # otherwise)
    return [i.strip() for i in list(dict.fromkeys(lst))]

def main(args):
    # get path to SDK
    sdk_dir = Path(__file__).parent.resolve()
    build_json_path = sdk_dir / Path(f"build/{args.board}/{args.config}/{args.toolchain}.json")

    # We now have a JSON file that provides correct...
    #   CFLAGS, CXXFLAGS, ASFLAGS, LDFLAGS, defines, includes, all source files, all pre-compiled libs
    # ... for the given TARGET/CONFIG/TOOLCHAIN.
    #
    # It is located at sdk_dir/build/$(TARGET)/$(CONFIG)/$(TOOLCHAIN).json

    # NOTE: The JSON generation here is a prototype of an implementation that will
    #       be fleshed out and folded into a future release of core-make. Because
    #       of this, all paths in the generated JSON are relative to sdk_dir. In
    #       order to be used by CHIP's build process (which has its own separate
    #       working directory), the paths should be converted to absolute paths.
    # 
    # NOTE: All targets supported by CHIP need to be added as a direct dependency.
    #       E.g., a suitable .mtb files placed in the "deps" folder.
    #
    # NOTE: Due to a feature regression in ModusToolbox 2.2, the .mtb system has no way for CHIP to request all
    #       assets required for supported targets BUT exclude the AnyCloud version of LwIP. CHIP must use its own
    #       custom LwIP fork. In order to work around this, all required dependencies have been added as direct 
    #       dependencies of the SDK (i.e., .mtb files added to the deps/ directory) and automatic dependency
    #       management has been disabled in the SDK's Makefile.

    build_json = json.loads(build_json_path.read_text())

    # Clean up the resulting json
    build_json["includes"] = dedup_and_sanitize(build_json["includes"])
    build_json["c_source"] = dedup_and_sanitize(build_json["c_source"])
    build_json["cxx_source"] = dedup_and_sanitize(build_json["cxx_source"])
    build_json["asm_source"] = dedup_and_sanitize(build_json["asm_source"])
    build_json["libs"] = dedup_and_sanitize(build_json["libs"])
    build_json["objs"] = dedup_and_sanitize(build_json["objs"])

    # Remove unneeded paths and files that are already provided by CHIP
    build_json["includes"].remove("-I./libs/wifi-mw-core/configs")
    build_json["includes"].remove("-I./libs/wifi-mw-core/lwip-whd-port/COMPONENT_FREERTOS/arch")
    build_json["c_source"].remove("./libs/wifi-mw-core/lwip-whd-port/COMPONENT_FREERTOS/arch/sys_arch.c")

    print(json.dumps(build_json))

if __name__=="__main__":
    parser = argparse.ArgumentParser(description="Create output json file containing mtb build data.")

    parser.add_argument("board", help="Board name")
    parser.add_argument("-t", "--toolchain", dest="toolchain", default="GCC_ARM", choices=["GCC_ARM"], help="Toolchain")
    parser.add_argument("-c", "--config", dest="config", default="Debug", choices=["Debug", "Release"], help="Build config")

    args = parser.parse_args()

    main(args)
