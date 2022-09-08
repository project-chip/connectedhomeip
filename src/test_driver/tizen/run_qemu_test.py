#!/usr/bin/env python3
# Copyright (c) 2021 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import coloredlogs
import click
import logging
import os
import re
import sys
import subprocess
import glob
import shutil

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    'debug': logging.DEBUG,
    'info': logging.INFO,
    'warn': logging.WARN,
    'fatal': logging.FATAL,
}


@click.command()
@click.option(
    '--log-level',
    default='INFO',
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    help='Determines the verbosity of script output.')
@click.option(
    '--target',
    default='tizen-arm-light-no-ble',
    help='Select target application')
@click.option(
    '--out-dir',
    default='/opt/tizen-qemu',
    help='Select directory where app.iso will be created for mount inside qemu')
@click.option(
    '--images-dir',
    default='/opt/tizen-qemu',
    help='Select directory where located prepared system images for qemu')
@click.option(
    '--verbose',
    default=False,
    is_flag=True,
    help='More verbose output')
def main(images_dir, log_level, out_dir, target, verbose):
    # Ensures somewhat pretty logging of what is going on
    log_fmt = '%(asctime)s %(levelname)-7s %(message)s'
    coloredlogs.install(level=__LOG_LEVELS__[log_level], fmt=log_fmt)
    script_path = os.path.dirname(os.path.realpath(sys.argv[0]))
    terget_dir = os.path.join(os.getenv('PW_PROJECT_ROOT'), "out", target)

    # Create an ISO witch target application and chip-tool
    try:
        if not os.path.exists('%s/apps' % out_dir):
            os.makedirs('%s/apps' % out_dir)

        for src_files in glob.glob(r'%s/package/*-app' % terget_dir):
            print(src_files)
            for file in glob.glob(r'%s/package/out/*.tpk' % src_files):
                print(file)
                shutil.copy(file, '%s/apps' % out_dir)

        output = subprocess.run('cp -r %s/chip-tool %s/apps' % (terget_dir, out_dir), shell=True)
        if output.returncode != 0:
            raise Exception("Can't copy %s/chip-tool" % terget_dir)

        output = subprocess.run('cp -r %s/testing_process.sh %s/apps' % (script_path, out_dir), shell=True)
        if output.returncode != 0:
            raise Exception("Can't copy testing process script")

        output = subprocess.run(['mkisofs', "-o",
                                 "%s/app.iso" % out_dir,
                                "-J", "-U", "-R",
                                 "-input-charset", "default",
                                 "-V", "CHIP",
                                 "%s/apps" % out_dir], capture_output=True)

        if output.returncode != 0:
            raise Exception("Can't create ISO file image")

        # Clean after create iso
        shutil.rmtree('%s/apps' % out_dir)

        logging.info("Correct created %s/app.iso" % out_dir)
    except subprocess.CalledProcessError as e:
        print(e.output)
        raise

    logging.info("Testing target: %s", target)
    print("============== TEST BEGIN ================")

    with subprocess.Popen(['qemu-system-arm',
                          '-monitor', 'null',
                           '-serial', 'stdio',
                           '-display', 'none',
                           '-M', 'virt',
                           '-smp', '2',
                           '-m', '512M',
                           '-kernel', '%s/tizen-iot-virt-zImage' % images_dir,
                           '-append', "console=ttyAMA0 earlyprintk earlycon root=/dev/vda",
                           '-device', 'virtio-net-device,netdev=virtio-net',
                           '-device', 'virtio-blk-device,drive=virtio-blk3',
                           '-device', 'virtio-blk-device,drive=virtio-blk2',
                           '-device', 'virtio-blk-device,drive=virtio-blk1',
                           '-netdev', 'user,id=virtio-net',
                           '-drive', 'file=%s/tizen-iot-rootfs.img,id=virtio-blk1,if=none,format=raw' % images_dir,
                           '-drive', 'file=%s/tizen-iot-sysdata.img,id=virtio-blk2,if=none,format=raw' % images_dir,
                           '-drive', 'file=%s/app.iso,id=virtio-blk3,if=none,format=raw' % out_dir],
                          encoding="850",
                          stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT,
                          universal_newlines=True) as proc:
        status = proc
        for line in proc.stdout.readlines():
            line = line.rstrip()
            try:
                if verbose:
                    print(line)

                if status.returncode is not None:
                    raise Exception("Target %s testing failed with code: " % target + str(status.returncode))

                if re.match(r"^CHIP test(\s+([A-Za-z0-9]+\s+)+)status: [1-9]+$", line):
                    raise Exception("CHIP test FAILED: %s" % line)

            except Exception as e:
                logging.fatal(e)
                print("============== TEST END ================")
                raise

    print("============== TEST END ================")
    logging.info("Target %s PASSED", target)


if __name__ == '__main__':
    main()
