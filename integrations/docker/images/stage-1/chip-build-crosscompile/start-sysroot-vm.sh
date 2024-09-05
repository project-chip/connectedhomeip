#!/usr/bin/env bash

# This script is intended to build a SYSROOT that can be used as a base to cross-compile
# CHIP for aarch64 (i.e. that runs on a RaspberryPi) from a arm64 linux machine
#
# This requires:
#
#   sudo apt install qemu-system-arm virtinst libvirt-daemon
#

set -e

if ! which qemu-system-aarch64 >/dev/null; then
    echo "Cannot find 'qemu-system-aarch64'. Did you 'sudo apt-get install qemu-system-arm' ?"
    exit 1
fi

if ! which virt-install >/dev/null; then
    echo "Cannot find 'virt-install'. Did you 'sudo apt-get install virtinst' ?"
    exit 1
fi

if ! [ -f /usr/sbin/libvirtd ]; then
    echo "Cannot find 'virt-install'. Did you 'sudo apt-get install libvirt-daemon' ?"
    exit 1
fi

if ! which mkpasswd >/dev/null; then
  echo "Cannot find 'mkpasswd'. Did you 'sudo apt-get install whois' ?"
  exit 1
fi


if ! which qemu-img >/dev/null; then
    echo "Cannot find 'qemu-img'."
    exit 1
fi

if ! which genisoimage >/dev/null; then
    echo "Cannot find 'genisoimage'."
    exit 1
fi

CLOUD_IMAGE_URL="https://cloud-images.ubuntu.com/releases/24.04/release-20240821/ubuntu-24.04-server-cloudimg-arm64.img"
CLOUD_IMAGE_FILE=$(basename "$CLOUD_IMAGE_URL")

if ! [ -f "$CLOUD_IMAGE_FILE" ]; then
    echo "Image $CLOUD_IMAGE_FILE does not exist. Downloading ..."
    wget "$CLOUD_IMAGE_URL"
else
    echo "Using existing $CLOUD_IMAGE_FILE ..."
fi

rm -f sysrootsrv.img
qemu-img create -b "$CLOUD_IMAGE_FILE" -f qcow2 -F qcow2 sysrootsrv.img 10G

# local user allowed to SSH in to the host directly without a password
SSH_KEY=$(cat ~/.ssh/id_rsa.pub)

# set a login password
PASSWORD_HASH=$(mkpasswd 1234)

# Create cloud init files
cat >user-data <<TEXTEND
#cloud-config

users:
  - default
  - name: ubuntu
    shell: /bin/bash
    ssh_authorized_keys:
      - $SSH_KEY
    sudo: ["ALL=(ALL) NOPASSWD:ALL"]
    groups: sudo,admin,users
    shell: /bin/bash
    lock_passwd: false
    passwd: $PASSWORD_HASH

system_info:
  default_user:
    name: ubuntu
    home: /home/ubuntu

hostname: sysrootsrv

package_update: true
package_upgrade: true
packages:
  - g++
  - gcc
  - libavahi-client-dev
  - libcairo2-dev
  - libdbus-1-dev
  - libgirepository1.0-dev
  - libglib2.0-dev
  - libreadline-dev
  - libsdl2-dev
  - libssl-dev
  - ninja-build
  - pkg-config
  - unzip

runcmd:
  # Ubuntu cloud-init merges with a locked ubuntu, so the "lock_passwd" option does
  # not actually work. Unlock it here to make console work
  - [ passwd, --unlock, ubuntu ]
  # A marker saying that initial install is done. Installing all the packages above is VERY slow
  - [ echo, Initial installation DONE ]

TEXTEND

truncate -s 0 meta-data
echo 'instance-id: sysrootsrv' >>meta-data
echo 'local-hostname: sysrootsrv' >>meta-data

genisoimage -output cidata.iso -V cidata -r -J user-data meta-data

# NOTE: direct qemu-system-aarch64 can also be used, however that requires
#       having QEMU_EFI.fd available (and location for that varies). Using
#       virt-install uses extra packages/complexity however also allows
#       the use of virsh for some operations.
#
#    truncate -s 64m varstore.img
#    truncate -s 64m efi.img
#    dd if=/usr/share/qemu-efi-aarch64/QEMU_EFI.fd of=efi.img conv=notrunc
#
#    # QEMU console exit is `CTRL + a, x`
#    qemu-system-aarch64                                   \
#     -m 8192                                              \
#     -M virt                                              \
#     -cpu max                                             \
#     -nographic                                           \
#     -drive if=pflash,format=raw,file=efi.img,readonly=on \
#     -drive if=pflash,format=raw,file=varstore.img        \
#     -drive if=none,file=sysrootsrv.img,id=hd0            \
#     -device virtio-blk-device,drive=hd0                  \
#     -cdrom cidata.iso                                    \
#     -netdev user,id=netdev0,hostfwd=tcp::5555-:22        \
#     -device virtio-net-pci,netdev=netdev0                \
#     ; # last line

# EXAMPLE COMMANDS:
#
# View generated image:
#   virsh dumpxml sysrootsrv
#
# Get a console and login (exit with `CTRL + ]`)
#   virsh console sysrootsrv
#
# Delete:
#   virsh destroy sysrootsrv && virsh undefine --nvram sysrootsrv
virt-install \
    --name sysrootsrv \
    --memory 8192 \
    --arch aarch64 \
    --import \
    --disk path=sysrootsrv.img,format=qcow2 \
    --disk path=cidata.iso,device=cdrom \
    --os-variant=ubuntu24.04 \
    --network=user \
    --xml "xpath.delete=./devices/input[@bus = 'usb']" \
    --noautoconsole \
    ;# last-line

# Allow ssh via:
#   ssh ubuntu@localhost -p 5555
virsh qemu-monitor-command --hmp sysrootsrv 'hostfwd_add ::5555-:22'
