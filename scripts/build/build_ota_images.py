#!/usr/bin/env -S python3 -B

# Copyright (c) 2021 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import logging
import os
import sys

import click

base_path = os.getcwd()
built_images_stack = []
target_file = f"{base_path}/examples/ota-requestor-app/linux/include/CHIPProjectAppConfig.h"
backup_config_cmd = f"cp {target_file} {base_path}/examples/ota-requestor-app/linux/include/CHIPProjectAppConfig.h.backup"
restore_config_cmd = f"cp {base_path}/examples/ota-requestor-app/linux/include/CHIPProjectAppConfig.h.backup {target_file}"


@click.command()
@click.option(
    '--vendor-id',
    default="0xDEAD",
    help='Vendor id')
@click.option(
    '--product-id',
    default="0xBEEF",
    help='Product id.')
@click.option(
    '--max-range',
    default=5,
    help='Build target(s)')
@click.option(
    '--out-prefix',
    default='out/su_ota_images_min',
    help='Path for Ota.min files')
def main(vendor_id, product_id, max_range, out_prefix):
    if max_range < 2 or max_range > 10:
        print("Max range is lower than 2 or greater than 10")
        return
    print(f"Creating ota_images into folder {out_prefix}")
    build_ota_images(vendor_id=vendor_id, product_id=product_id, max_range=max_range, out_prefix=out_prefix)


def restore_configuration():
    print("Restoring the config file and deleting config file")
    os.system(restore_config_cmd)
    os.system(f"rm {base_path}/examples/ota-requestor-app/linux/include/CHIPProjectAppConfig.h.backup")


def build_ota_images(vendor_id, product_id, max_range, out_prefix):
    """Build the ota images requirered for the SU test cases.
    """

    config_version_string = '''
#ifdef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
#undef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION $VERSION_N
#endif
#ifdef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING 
#undef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING \"$VERSION_STR.0\"
#endif
'''

    logging.info("Creating the backup file.")
    os.system(backup_config_cmd)

    status = 0
    for version in range(2, max_range):
        config_str = config_version_string
        config_str = config_str.replace("$VERSION_N", str(version))
        config_str = config_str.replace("$VERSION_STR", str(version))
        logging.info(f"Injecting version code into {target_file}:")

        logging.info(f"Building requestor-app with version {version}")
        code_inject_cmd = f"echo '{config_str}' >> {target_file}"
        status = os.system(code_inject_cmd)
        if status != 0:
            logging.error("Unable to append to configfile.")
            break

        # Build the image without flags and debug
        logging.info("Building the requestor app")
        cmd_build = f"{base_path}/scripts/examples/gn_build_example.sh {base_path}/examples/ota-requestor-app/linux {base_path}/{out_prefix} chip_config_network_layer_ble=false is_debug=false > /dev/null"
        status = os.system(cmd_build)
        if status != 0:
            logging.error("Failed to build the image")
            break

        # Strip the flags
        linux_opt = ''
        if 'linux' in sys.platform.lower():
            linux_opt = '--strip-all'

        logging.info("Striping application to minify it.")
        cmd_strip = f"strip {linux_opt}   {base_path}/{out_prefix}/chip-ota-requestor-app -o {base_path}/{out_prefix}/chip-ota-requestor-app.min > /dev/null"
        status = os.system(cmd_strip)
        if status != 0:
            logging.error("Failed to strip the app")
            break

        # build the ota image
        logging.info("Adding the ota headers into the binary file")
        ota_image_path = f"{base_path}/{out_prefix}/chip-ota-requestor-app_v{version}.min.ota"
        cmd_build_ota = f'python3 {base_path}/src/app/ota_image_tool.py create -v {vendor_id} -p {product_id} -vn {version} -vs "{version}.0" -da sha256 {base_path}/{out_prefix}/chip-ota-requestor-app.min {ota_image_path}'
        status = os.system(cmd_build_ota)
        if status != 0:
            print("Failed to build the ota image")
            break
        built_images_stack.append(ota_image_path)
        logging.info("Restoring the config file")
        os.system(restore_config_cmd)

    if status != 0:
        logging.error("Failed to build images")
        os.system(restore_config_cmd)
        os.system(f"rm {base_path}/examples/ota-requestor-app/linux/include/CHIPProjectAppConfig.h.backup")
        exit(1)

    if status == 0:
        logging.info("Image built completed")
        os.system(f"rm {base_path}/examples/ota-requestor-app/linux/include/CHIPProjectAppConfig.h.backup")

    logging.info("Completed images built in this process")
    print("\n=====".join(built_images_stack))
    exit(0)


if __name__ == "__main__":
    sys.exit(main())
