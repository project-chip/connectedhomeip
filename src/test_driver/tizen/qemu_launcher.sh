#!/usr/bin/env bash

#
#    Copyright (c) 2021 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

cd /opt || exit

KERNEL=${1-tizen-iot-virt-zImage}

stty intr ^]

echo "$KERNEL"
qemu-system-arm \
    -monitor null \
    -serial stdio \
    -display none \
    -M virt -smp 2 -m 512M \
    -kernel "$KERNEL" \
    -append "console=ttyAMA0 earlyprintk earlycon root=/dev/vda" \
    -device virtio-net-device,netdev=virtio-net \
    -device virtio-blk-device,drive=virtio-blk3 \
    -device virtio-blk-device,drive=virtio-blk2 \
    -device virtio-blk-device,drive=virtio-blk1 \
    -netdev user,id=virtio-net \
    -drive file=tizen-iot-rootfs.img,id=virtio-blk1,if=none,format=raw \
    -drive file=tizen-iot-sysdata.img,id=virtio-blk2,if=none,format=raw \
    -drive file=app.iso,id=virtio-blk3,if=none,format=raw \
    "$@"

stty intr ^C
