ARG VERSION=latest
FROM connectedhomeip/chip-build-tizen:${VERSION}

# ------------------------------------------------------------------------------
# Switch to the root user so we could install things
USER root

# ------------------------------------------------------------------------------
# Install QEMU and build dependencies
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    bc \
    genisoimage \
    qemu-system-arm \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

# ------------------------------------------------------------------------------
# Build Tizen kernel
ENV TIZEN_QEMU_KERNEL /opt/tizen-iot-virt-zImage

WORKDIR /tmp/workdir

# It seems that Linux kernel does not support overlay FS with SMACK enabled. In
# order not to disable SMACK LSM support completely (it could break some tools
# in Tizen ecosystem) we will add "permissive" mode to SMACK.
COPY files/0001-smack-add-permissive-mode.patch .

RUN set -x \
    # ------------------------------------------------------------------------------
    # Download Linux rpi4 kernel
    && wget --progress=dot:giga -r -nd --no-parent -A 'rpi4-linux-kernel-*.src.rpm' \
    http://download.tizen.org/snapshots/tizen/$TIZEN_VERSION-unified/latest/repos/standard/source/ \
    # Prepare kernel source (Linux kernel + Tizen patchset)
    && unrpm rpi4-linux-kernel-*.src.rpm \
    && tar -xJf linux-kernel-*.tar.xz \
    && rm linux-kernel-*.tar.xz \
    && cd linux-kernel-* \
    && zcat ../*-to-*.diff.gz | patch -p1 \
    && patch -p1 < ../0001-smack-add-permissive-mode.patch \
    # Compile
    && export MAKEFLAGS=-j$(nproc) \
    && export ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- \
    && make tizen_bcm2711_defconfig \
    && ./scripts/config -e ARCH_VIRT \
    && ./scripts/config -e VIRTIO_MMIO -e HW_RANDOM_VIRTIO \
    && ./scripts/config -e VIRTIO_PCI -e VIRTIO_BLK \
    && ./scripts/config -e VIRTIO_NET -e VETH \
    && ./scripts/config -e IKCONFIG -e IKCONFIG_PROC \
    && ./scripts/config -e BT_HCIVHCI -e CRYPTO_USER_API_HASH -e CRYPTO_USER_API_SKCIPHER \
    && ./scripts/config -e OVERLAY_FS -e ISO9660_FS \
    && ./scripts/config -e SECURITY_SMACK_PERMISSIVE_MODE \
    && make olddefconfig \
    && make zImage \
    && mv arch/arm/boot/zImage $TIZEN_QEMU_KERNEL \
    # Cleanup
    && rm -rf /tmp/workdir \
    && : # last line

# ------------------------------------------------------------------------------
# Create Tizen IoT File System
ENV TIZEN_FS_IMAGE_ROOT /opt/tizen-iot-rootfs.img
ENV TIZEN_FS_IMAGE_DATA /opt/tizen-iot-sysdata.img

WORKDIR /tmp/workdir

RUN set -x \
    # ------------------------------------------------------------------------------
    # Download Tizen images
    && wget --progress=dot:giga -r -nd --no-parent -A 'tizen-*.tar.gz' \
    http://download.tizen.org/snapshots/tizen/unified/latest/images/standard/tizen-headless-armv7l/  \
    # Unpack
    && tar -xzf tizen-*.tar.gz \
    && mv system-data.img $TIZEN_FS_IMAGE_DATA \
    && mv rootfs.img $TIZEN_FS_IMAGE_ROOT \
    # Cleanup
    && rm -rf /tmp/workdir \
    && : # last line

WORKDIR /tmp/workdir

RUN set -x \
    # ------------------------------------------------------------------------------
    # Install libguestfs and linux-image-generic for guestfish
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    libguestfs-tools \
    linux-image-generic-hwe-20.04 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && SYSTEMD_SYSTEM=/usr/lib/systemd/system \
    # Add extra libraries
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda glob copy-in \
    $TIZEN_SDK_TOOLCHAIN/arm-tizen-linux-gnueabi/lib/libasan.so.* /usr/lib/ \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda glob copy-in \
    $TIZEN_SDK_TOOLCHAIN/arm-tizen-linux-gnueabi/lib/libubsan.so.* /usr/lib/ \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda glob copy-in \
    $TIZEN_SDK_SYSROOT/usr/lib/libcapi-network-thread.so.* /usr/lib/ \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda glob copy-in \
    $TIZEN_SDK_SYSROOT/usr/lib/libnsd-dns-sd.so.* /usr/lib/ \
    # Disable failing systemd services
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda glob rm \
    $SYSTEMD_SYSTEM/deviced.service \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda glob rm \
    $SYSTEMD_SYSTEM/mnt-inform.mount \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda glob rm \
    $SYSTEMD_SYSTEM/murphyd.service \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda glob rm \
    $SYSTEMD_SYSTEM/pulseaudio.service \
    # Add HCI emulator (btvirt) from BlueZ project
    && wget -O bluez.tar.gz https://github.com/bluez/bluez/archive/refs/tags/5.64.tar.gz \
    && tar -xzf bluez.tar.gz \
    && cd bluez-* \
    && export CFLAGS="--sysroot=${TIZEN_SDK_SYSROOT}" \
    && export PKG_CONFIG_PATH="${TIZEN_SDK_SYSROOT}/usr/lib/pkgconfig" \
    && export PKG_CONFIG_SYSROOT_DIR="${TIZEN_SDK_SYSROOT}" \
    && ./bootstrap \
    && ./configure --host=arm-linux-gnueabi --disable-client --disable-manpages --enable-testing --enable-tools \
    && make lib/bluetooth/bluetooth.h lib/bluetooth/hci.h \
    && make emulator/btvirt -j$(nproc) \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda glob copy-in \
    emulator/btvirt /usr/bin/ \
    # Grant access to vHCI for network_fw group
    && UDEV_BT_VHCI=/usr/lib/udev/rules.d/99-bluetooth-vhci.rules \
    && echo 'KERNEL=="vhci", MODE="0660", GROUP="network_fw"' \
    | guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda upload - $UDEV_BT_VHCI \
    # Create vHCI devices on startup (modify RPi4 hciattach script)
    && BT_DEV_START_SH=/usr/etc/bluetooth/bt-dev-start-rpi3.sh \
    && echo > temp.txt \
    "#!/bin/sh\n" \
    "/usr/bin/btvirt -l2 &\n" \
    "sleep 0.5\n" \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda upload temp.txt $BT_DEV_START_SH \
    # Mount Tizen system partition on /opt-ro instead of /opt
    && SYSTEMD_UNIT_OPT_RO_MOUNT=$SYSTEMD_SYSTEM/opt\\x2dro.mount \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda ln-sf $SYSTEMD_UNIT_OPT_RO_MOUNT $SYSTEMD_SYSTEM/local-fs.target.requires \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda mv $SYSTEMD_SYSTEM/opt.mount $SYSTEMD_UNIT_OPT_RO_MOUNT \
    && virt-edit $TIZEN_FS_IMAGE_ROOT -e 's#/opt#/opt-ro#g' $SYSTEMD_UNIT_OPT_RO_MOUNT \
    && virt-edit $TIZEN_FS_IMAGE_ROOT -e 's#Options=#Options=ro,#' $SYSTEMD_UNIT_OPT_RO_MOUNT \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda mkdir /opt-ro \
    # Mount Tizen system partition as a tmp-based overlay
    && SYSTEMD_UNIT_OPT_PRE_MOUNT=$SYSTEMD_SYSTEM/opt.pre-mount.service \
    && echo > temp.txt \
    "[Unit]\n" \
    "DefaultDependencies=no\n" \
    "RequiresMountsFor=/tmp\n" \
    "[Service]\n" \
    "Type=oneshot\n" \
    "ExecStart=mkdir -p /tmp/.opt-upper /tmp/.opt-work\n" \
    "RemainAfterExit=yes\n" \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda upload temp.txt $SYSTEMD_UNIT_OPT_PRE_MOUNT \
    #
    && SYSTEMD_UNIT_OPT_MOUNT=$SYSTEMD_SYSTEM/opt.mount \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda ln-sf $SYSTEMD_UNIT_OPT_MOUNT $SYSTEMD_SYSTEM/local-fs.target.requires \
    && echo > temp.txt \
    "[Unit]\nConditionPathIsMountPoint=!/opt\n" \
    "RequiresMountsFor=/opt-ro /tmp\n" \
    "Wants=opt.pre-mount.service\n" \
    "After=opt.pre-mount.service\n" \
    "[Mount]\n" \
    "What=overlay\n" \
    "Where=/opt\n" \
    "Type=overlay\n" \
    "Options=lowerdir=/opt-ro,upperdir=/tmp/.opt-upper,workdir=/tmp/.opt-work\n" \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda upload temp.txt $SYSTEMD_UNIT_OPT_MOUNT \
    #
    && SYSTEMD_UNIT_OPT_POST_MOUNT=$SYSTEMD_SYSTEM/opt.post-mount.service \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda ln-sf $SYSTEMD_UNIT_OPT_POST_MOUNT $SYSTEMD_SYSTEM/local-fs.target.requires \
    && echo > temp.txt \
    "[Unit]\n" \
    "DefaultDependencies=no\n" \
    "RequiresMountsFor=/opt\n" \
    "[Service]\n" \
    "Type=oneshot\n" \
    "ExecStart=chsmack -a * /tmp/.opt-work/work\n" \
    "RemainAfterExit=yes\n" \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda upload temp.txt $SYSTEMD_UNIT_OPT_POST_MOUNT \
    # Mount Matter APP drive on startup
    && SYSTEMD_UNIT_CHIP_MOUNT=$SYSTEMD_SYSTEM/mnt-chip.mount \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda ln-sf \
    $SYSTEMD_UNIT_CHIP_MOUNT $SYSTEMD_SYSTEM/local-fs.target.requires \
    && echo > temp.txt \
    "[Unit]\n" \
    "ConditionPathIsMountPoint=!/mnt/chip\n" \
    "[Mount]\n" \
    "What=/dev/disk/by-label/CHIP\nWhere=/mnt/chip\n" \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda upload temp.txt $SYSTEMD_UNIT_CHIP_MOUNT \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda mkdir /mnt/chip \
    # Run Chip testing script on login
    && echo "/mnt/chip/testing_process.sh" > temp.txt \
    && guestfish --rw -a $TIZEN_FS_IMAGE_ROOT -m /dev/sda upload temp.txt /root/.profile \
    # Setup auto-login for root user
    && SGETTY=/usr/lib/systemd/system/serial-getty@.service \
    && virt-edit $TIZEN_FS_IMAGE_ROOT -e \
    's#^ExecStart=.*#ExecStart=-/sbin/agetty -o "-p -f root" --noclear -a root --keep-baud 115200 %I \$TERM#' $SGETTY \
    # Cleanup
    && rm -rf /tmp/workdir \
    && rm -rf /var/tmp/.guestfs-0 \
    && apt-get purge -fy \
    libguestfs-tools \
    linux-image-generic-hwe-20.04 \
    && apt-get autoremove -fy \
    && : # last line

# ------------------------------------------------------------------------------
# Switch to the non-root user
USER $USER_NAME
WORKDIR $USER_HOME
