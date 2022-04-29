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
    btrfs-progs \
    genisoimage \
    squashfs-tools \
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
    # Download
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

COPY files/rootfs-security-smack.xattrs .

RUN set -x \
    && ROOT=$PWD/rootfs \
    && SYSTEMD_SYSTEM=$ROOT/usr/lib/systemd/system \
    # Download
    && wget --progress=dot:giga -r -nd --no-parent -A 'tizen-*.tar.gz' \
    http://download.tizen.org/snapshots/tizen/$TIZEN_VERSION-unified/latest/images/standard/iot-headless-2parts-armv7l-btrfs-rootfs-rpi/ \
    # Unpack
    && tar -xzf tizen-*.tar.gz \
    && mv system-data.img $TIZEN_FS_IMAGE_DATA \
    # Build patched (read xattrs from file) SquashFS tools
    && wget https://github.com/Arkq/squashfs-tools/archive/refs/heads/master.zip \
    && unzip master.zip \
    && make -C squashfs-tools-master/squashfs-tools -j$(nproc) \
    # Extract files from RootFS image
    && mkdir $ROOT \
    && btrfs restore --metadata --symlinks rootfs.img $ROOT \
    # Add extra libraries
    && cp $TIZEN_SDK_TOOLCHAIN/arm-tizen-linux-gnueabi/lib/libasan.so.* $ROOT/usr/lib/ \
    && cp $TIZEN_SDK_TOOLCHAIN/arm-tizen-linux-gnueabi/lib/libubsan.so.* $ROOT/usr/lib/ \
    && cp $TIZEN_SDK_SYSROOT/usr/lib/libcapi-network-thread.so.* $ROOT/usr/lib/ \
    && cp $TIZEN_SDK_SYSROOT/usr/lib/libnsd-dns-sd.so.* $ROOT/usr/lib/ \
    # Disable failing systemd services
    && rm $SYSTEMD_SYSTEM/deviced.service \
    && rm $SYSTEMD_SYSTEM/display-manager* \
    && rm $SYSTEMD_SYSTEM/mnt-inform.mount \
    && rm $SYSTEMD_SYSTEM/stability-monitor.service \
    # Add HCI emulator (btvirt) from BlueZ project
    && wget -O bluez.tar.gz https://github.com/bluez/bluez/archive/refs/tags/5.64.tar.gz \
    && tar -xzf bluez.tar.gz && cd bluez-* \
    && export CFLAGS="--sysroot=${TIZEN_SDK_SYSROOT}" \
    && export PKG_CONFIG_PATH="${TIZEN_SDK_SYSROOT}/usr/lib/pkgconfig" \
    && export PKG_CONFIG_SYSROOT_DIR="${TIZEN_SDK_SYSROOT}" \
    && ./bootstrap \
    && ./configure --host=arm-linux-gnueabi --disable-client --disable-manpages --enable-testing --enable-tools \
    && make lib/bluetooth/bluetooth.h lib/bluetooth/hci.h \
    && make emulator/btvirt -j$(nproc) \
    && install emulator/btvirt $ROOT/usr/bin/ \
    # Grant access to vHCI for network_fw group
    && UDEV_BT_VHCI=$ROOT/usr/lib/udev/rules.d/99-bluetooth-vhci.rules \
    && echo 'KERNEL=="vhci", MODE="0660", GROUP="network_fw"' > $UDEV_BT_VHCI \
    # Create vHCI devices on startup (modify RPi4 hciattach script)
    && BT_DEV_START_SH=$ROOT/usr/etc/bluetooth/bt-dev-start-rpi3.sh \
    && echo > $BT_DEV_START_SH \
    "#!/bin/sh\n" \
    "/usr/bin/btvirt -l2 &\n" \
    "sleep 0.5\n" \
    # Mount Tizen system partition on /opt-ro instead of /opt
    && SYSTEMD_UNIT_OPT_RO_MOUNT=$SYSTEMD_SYSTEM/opt\\x2dro.mount \
    && ln -frs $SYSTEMD_UNIT_OPT_RO_MOUNT $SYSTEMD_SYSTEM/local-fs.target.requires \
    && mv $SYSTEMD_SYSTEM/opt.mount $SYSTEMD_UNIT_OPT_RO_MOUNT \
    && sed -i -e 's#/opt#/opt-ro#g' $SYSTEMD_UNIT_OPT_RO_MOUNT \
    && sed -i -e 's#Options=#Options=ro,#' $SYSTEMD_UNIT_OPT_RO_MOUNT \
    && mkdir -p $ROOT/opt-ro \
    # Mount Tizen system partition as a tmp-based overlay
    && SYSTEMD_UNIT_OPT_PRE_MOUNT=$SYSTEMD_SYSTEM/opt.pre-mount.service \
    && echo > $SYSTEMD_UNIT_OPT_PRE_MOUNT \
    "[Unit]\n" \
    "DefaultDependencies=no\n" \
    "RequiresMountsFor=/tmp\n" \
    "[Service]\n" \
    "Type=oneshot\n" \
    "ExecStart=mkdir -p /tmp/.opt-upper /tmp/.opt-work\n" \
    "RemainAfterExit=yes\n" \
    && SYSTEMD_UNIT_OPT_MOUNT=$SYSTEMD_SYSTEM/opt.mount \
    && ln -frs $SYSTEMD_UNIT_OPT_MOUNT $SYSTEMD_SYSTEM/local-fs.target.requires \
    && echo > $SYSTEMD_UNIT_OPT_MOUNT \
    "[Unit]\n" \
    "ConditionPathIsMountPoint=!/opt\n" \
    "RequiresMountsFor=/opt-ro /tmp\n" \
    "Wants=opt.pre-mount.service\n" \
    "After=opt.pre-mount.service\n" \
    "[Mount]\n" \
    "What=overlay\n" \
    "Where=/opt\n" \
    "Type=overlay\n" \
    "Options=lowerdir=/opt-ro,upperdir=/tmp/.opt-upper,workdir=/tmp/.opt-work\n" \
    && SYSTEMD_UNIT_OPT_POST_MOUNT=$SYSTEMD_SYSTEM/opt.post-mount.service \
    && ln -frs $SYSTEMD_UNIT_OPT_POST_MOUNT $SYSTEMD_SYSTEM/local-fs.target.requires \
    && echo > $SYSTEMD_UNIT_OPT_POST_MOUNT \
    "[Unit]\n" \
    "DefaultDependencies=no\n" \
    "RequiresMountsFor=/opt\n" \
    "[Service]\n" \
    "Type=oneshot\n" \
    "ExecStart=chsmack -a * /tmp/.opt-work/work\n" \
    "RemainAfterExit=yes\n" \
    # Mount Matter APP drive on startup
    && SYSTEMD_UNIT_CHIP_MOUNT=$SYSTEMD_SYSTEM/mnt-chip.mount \
    && ln -frs $SYSTEMD_UNIT_CHIP_MOUNT $SYSTEMD_SYSTEM/local-fs.target.requires \
    && echo > $SYSTEMD_UNIT_CHIP_MOUNT \
    "[Unit]\n" \
    "ConditionPathIsMountPoint=!/mnt/chip\n" \
    "[Mount]\n" \
    "What=/dev/disk/by-label/CHIP\n" \
    "Where=/mnt/chip\n" \
    && mkdir -p $ROOT/mnt/chip \
    # Install Matter APP on startup
    && SYSTEMD_UNIT_CHIP_INSTALL=$SYSTEMD_SYSTEM/chip-install-tpk.service \
    && ln -frs $SYSTEMD_UNIT_CHIP_INSTALL $SYSTEMD_SYSTEM/default.target.wants \
    && echo > $SYSTEMD_UNIT_CHIP_INSTALL \
    "[Service]\n" \
    "Type=oneshot\n" \
    "ExecStart=sh -c \"pkgcmd -i -t tpk -p /mnt/chip/org.tizen.matter.*\"\n" \
    "RemainAfterExit=yes\n" \
    && : # last line

RUN set -x \
    && ROOT=$PWD/rootfs \
    && SYSTEMD_SYSTEM=$ROOT/usr/lib/systemd/system \
    # XXXX
    && wget --progress=dot:mega -r -nd --no-parent \
    http://download.tizen.org/releases/milestone/tizen/unified/latest/repos/standard/packages/armv7l/ \
    -A 'strace-*.armv7l.rpm' \
    && unrpm *.rpm \
    && cp usr/bin/strace $ROOT/usr/bin/ \
    # Setup auto-login for root user
    && mkdir -p $ROOT/etc/systemd/system/serial-getty@ttyAMA0.service.d \
    && AUTOLOGIN_CONF=$ROOT/etc/systemd/system/serial-getty@ttyAMA0.service.d/autologin.conf \
    # && echo "[Service]\nExecStart=" > $AUTOLOGIN_CONF \
    # && echo "ExecStart=-/sbin/agetty -o '-p -- \\u' --noclear --keep-baud 115200,38400,9600 --autologin root - %I \$TERM" >> $AUTOLOGIN_CONF \
    # Create RootFS image as SquashFS
    && squashfs-tools-master/squashfs-tools/mksquashfs $ROOT $TIZEN_FS_IMAGE_ROOT -xattrs-file rootfs-security-smack.xattrs \
    # Cleanup
    && rm -rf /tmp/workdir \
    && : # last line

# ------------------------------------------------------------------------------
# Switch to the non-root user
USER $USER_NAME
WORKDIR $USER_HOME
