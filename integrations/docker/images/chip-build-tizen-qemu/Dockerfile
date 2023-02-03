ARG VERSION=latest
FROM connectedhomeip/chip-build-tizen:${VERSION}

ENV TIZEN_IOT_QEMU_KERNEL $TIZEN_SDK_ROOT/iot-qemu-virt-zImage
ENV TIZEN_IOT_IMAGE_ROOT $TIZEN_SDK_ROOT/iot-rootfs.img
ENV TIZEN_IOT_IMAGE_DATA $TIZEN_SDK_ROOT/iot-sysdata.img

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
    # Cleanup
    && apt-get clean \
    && rm -rf /var/lib/apt/lists \
    && : # last line

# It seems that Linux kernel does not support overlay FS with SMACK enabled. In
# order not to disable SMACK LSM support completely (it could break some tools
# in Tizen ecosystem) we will add "permissive" mode to SMACK.
COPY files/0001-smack-add-permissive-mode.patch $TIZEN_SDK_ROOT/files/

# ------------------------------------------------------------------------------
# Build Tizen kernel
RUN set -x \
    && mkdir -p /tmp/workdir && cd /tmp/workdir \
    # Download Linux rpi4 kernel
    && wget --progress=dot:giga -r -nd --no-parent -A 'rpi4-linux-kernel-*.src.rpm' \
    http://download.tizen.org/snapshots/tizen/$TIZEN_VERSION-unified/latest/repos/standard/source/ \
    # Prepare kernel source (Linux kernel + Tizen patchset)
    && unrpm rpi4-linux-kernel-*.src.rpm \
    && tar -xJf linux-kernel-*.tar.xz \
    && rm linux-kernel-*.tar.xz \
    && cd linux-kernel-* \
    && zcat ../*-to-*.diff.gz | patch -p1 \
    && patch -p1 < $TIZEN_SDK_ROOT/files/0001-smack-add-permissive-mode.patch \
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
    && mv arch/arm/boot/zImage $TIZEN_IOT_QEMU_KERNEL \
    # Cleanup
    && rm -rf /tmp/workdir \
    && : # last line

# ------------------------------------------------------------------------------
# Create Tizen IoT File System
RUN set -x \
    && mkdir -p /tmp/workdir && cd /tmp/workdir \
    && SYSTEMD_SYSTEM=/usr/lib/systemd/system \
    # Download Tizen images
    && wget --progress=dot:giga -r -nd --no-parent -A 'tizen-*.tar.gz' \
    http://download.tizen.org/snapshots/tizen/unified/latest/images/standard/tizen-headless-armv7l/  \
    # Unpack
    && tar -xzf tizen-*.tar.gz \
    && mv system-data.img $TIZEN_IOT_IMAGE_DATA \
    && mv rootfs.img $TIZEN_IOT_IMAGE_ROOT \
    # Install libguestfs and linux-image-generic required for in-place
    # modifications of the rootfs image
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends libguestfs-tools linux-image-generic \
    # Add extra libraries to the root image
    && guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda glob copy-in \
    $TIZEN_SDK_TOOLCHAIN/arm-tizen-linux-gnueabi/lib/libasan.so.* \
    $TIZEN_SDK_TOOLCHAIN/arm-tizen-linux-gnueabi/lib/libubsan.so.* \
    $TIZEN_SDK_SYSROOT/usr/lib/libcapi-network-thread.so.* \
    $TIZEN_SDK_SYSROOT/usr/lib/libnsd-dns-sd.so.* \
    /usr/lib/ \
    # Disable failing systemd services
    && guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda glob rm $SYSTEMD_SYSTEM/deviced.service \
    && guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda glob rm $SYSTEMD_SYSTEM/mnt-inform.mount \
    && guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda glob rm $SYSTEMD_SYSTEM/murphyd.service \
    && guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda glob rm $SYSTEMD_SYSTEM/pulseaudio.service \
    # Mount Tizen system partition on /opt-ro instead of /opt
    && SYSTEMD_UNIT_OPT_RO_MOUNT=$SYSTEMD_SYSTEM/opt\\x2dro.mount \
    && guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda ln-sf $SYSTEMD_UNIT_OPT_RO_MOUNT $SYSTEMD_SYSTEM/local-fs.target.requires \
    && guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda mv $SYSTEMD_SYSTEM/opt.mount $SYSTEMD_UNIT_OPT_RO_MOUNT \
    && virt-edit $TIZEN_IOT_IMAGE_ROOT -e 's#/opt#/opt-ro#g' $SYSTEMD_UNIT_OPT_RO_MOUNT \
    && virt-edit $TIZEN_IOT_IMAGE_ROOT -e 's#Options=#Options=ro,#' $SYSTEMD_UNIT_OPT_RO_MOUNT \
    && guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda mkdir /opt-ro \
    # Create /tmp/.opt-upper and /tmp/.opt-work after /tmp is mounted
    && SYSTEMD_UNIT_OPT_PRE_MOUNT=$SYSTEMD_SYSTEM/opt.pre-mount.service \
    && echo -n \
    "[Unit]\n" \
    "DefaultDependencies=no\n" \
    "RequiresMountsFor=/tmp\n" \
    "[Service]\n" \
    "Type=oneshot\n" \
    "ExecStart=mkdir -p /tmp/.opt-upper /tmp/.opt-work\n" \
    "RemainAfterExit=yes\n" \
    | guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda upload - $SYSTEMD_UNIT_OPT_PRE_MOUNT \
    # Mount Tizen system partition as a tmp-based overlay
    && SYSTEMD_UNIT_OPT_MOUNT=$SYSTEMD_SYSTEM/opt.mount \
    && guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda ln-sf $SYSTEMD_UNIT_OPT_MOUNT $SYSTEMD_SYSTEM/local-fs.target.requires \
    && echo -n \
    "[Unit]\nConditionPathIsMountPoint=!/opt\n" \
    "RequiresMountsFor=/opt-ro /tmp\n" \
    "Wants=opt.pre-mount.service\n" \
    "After=opt.pre-mount.service\n" \
    "[Mount]\n" \
    "What=overlay\n" \
    "Where=/opt\n" \
    "Type=overlay\n" \
    "Options=lowerdir=/opt-ro,upperdir=/tmp/.opt-upper,workdir=/tmp/.opt-work\n" \
    | guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda upload - $SYSTEMD_UNIT_OPT_MOUNT \
    # Fix SMACK label for /tmp/.opt-work/work
    && SYSTEMD_UNIT_OPT_POST_MOUNT=$SYSTEMD_SYSTEM/opt.post-mount.service \
    && guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda ln-sf $SYSTEMD_UNIT_OPT_POST_MOUNT $SYSTEMD_SYSTEM/local-fs.target.requires \
    && echo -n \
    "[Unit]\n" \
    "DefaultDependencies=no\n" \
    "RequiresMountsFor=/opt\n" \
    "[Service]\n" \
    "Type=oneshot\n" \
    "ExecStart=chsmack -a * /tmp/.opt-work/work\n" \
    "RemainAfterExit=yes\n" \
    | guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda upload - $SYSTEMD_UNIT_OPT_POST_MOUNT \
    # Mount Matter ISO image on startup
    && SYSTEMD_UNIT_CHIP_MOUNT=$SYSTEMD_SYSTEM/mnt-chip.mount \
    && guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda ln-sf $SYSTEMD_UNIT_CHIP_MOUNT $SYSTEMD_SYSTEM/local-fs.target.requires \
    && echo -n \
    "[Unit]\n" \
    "ConditionPathIsMountPoint=!/mnt/chip\n" \
    "[Mount]\n" \
    "What=/dev/disk/by-label/CHIP\n" \
    "Where=/mnt/chip\n" \
    "Options=nofail\n" \
    | guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda upload - $SYSTEMD_UNIT_CHIP_MOUNT \
    && guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda mkdir /mnt/chip \
    # Setup auto-login for root user
    && SYSTEMD_UNIT_SERIAL_GETTY=$SYSTEMD_SYSTEM/serial-getty@.service \
    && virt-edit $TIZEN_IOT_IMAGE_ROOT -e \
    's#^ExecStart=.*#ExecStart=-/sbin/agetty -o "-p -f root" --noclear -a root --keep-baud 115200 %I \$TERM#' $SYSTEMD_UNIT_SERIAL_GETTY \
    # Execute launcher script after root login
    && echo -n \
    "[ -x /launcher.sh ] && /launcher.sh\n" \
    | guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda upload - /root/.profile \
    # Launcher script
    && echo -n \
    "#!/bin/bash\n" \
    "if [[ -x /mnt/chip/runner.sh ]]; then\n" \
    "  echo '### RUNNER START ###'\n" \
    "  /mnt/chip/runner.sh\n" \
    "  echo '### RUNNER STOP:' \$?\n" \
    "else\n" \
    "  read -r -t 5 -p 'Press ENTER to access root shell...' && exit || echo ' timeout.'\n" \
    "fi\n" \
    "echo 'Shutting down emulated system...'\n" \
    "echo o > /proc/sysrq-trigger\n" \
    | guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda upload - /launcher.sh \
    && guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda chmod 0755 /launcher.sh \
    # Remove ALL previously installed packages
    && apt-get autopurge -fy $(tail -2 /var/log/apt/history.log | grep ^Install | cut -c 10- | sed 's/([^)]*),\?//g') \
    && rm -rf /var/lib/apt/lists \
    && rm -rf /var/tmp/.guestfs-0 \
    # Cleanup
    && rm -rf /tmp/workdir \
    && : # last line

# ------------------------------------------------------------------------------
# Switch back to the non-root user
USER $USER_NAME
WORKDIR $USER_HOME
