ARG VERSION=1
FROM ghcr.io/project-chip/chip-build-tizen:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

ENV TIZEN_IOT_QEMU_KERNEL=$TIZEN_SDK_ROOT/iot-qemu-virt-zImage
ENV TIZEN_IOT_IMAGE_ROOT=$TIZEN_SDK_ROOT/iot-rootfs.img
ENV TIZEN_IOT_IMAGE_DATA=$TIZEN_SDK_ROOT/iot-sysdata.img

# ------------------------------------------------------------------------------
# Switch to the root user so we could install things
USER root

# ------------------------------------------------------------------------------
# Install QEMU and build dependencies
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    bc \
    libgmp-dev \
    libmpc-dev \
    patch \
    qemu-system-arm \
    xz-utils \
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
    && wget --progress=dot:giga -r -nd --no-parent -e robots=off -A 'rpi4-linux-kernel-*.src.rpm' \
    http://download.tizen.org/snapshots/TIZEN/Tizen-$TIZEN_VERSION/Tizen-$TIZEN_VERSION-Unified/latest/repos/standard/source/ \
    # Prepare kernel source (Linux kernel + Tizen patchset)
    && 7z x -so rpi4-linux-kernel-*.src.rpm | cpio -idmv \
    && rm rpi4-linux-kernel-*.src.rpm \
    && tar -xJf linux-kernel-*.tar.xz \
    && rm linux-kernel-*.tar.xz \
    && cd *linux-kernel-* \
    && zcat ../*-to-*.diff.gz | patch -p1 \
    && patch -p1 < $TIZEN_SDK_ROOT/files/0001-smack-add-permissive-mode.patch \
    # Configure
    && export MAKEFLAGS=-j$(nproc) \
    && export ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- \
    && make tizen_bcm2711_defconfig \
    # Add support for QEMU VirtIO devices
    && ./scripts/config -e ARCH_VIRT -e VIRTIO_MMIO \
    && ./scripts/config -e VIRTIO_PCI -e VIRTIO_BLK \
    && ./scripts/config -e VIRTIO_NET -e VETH \
    && ./scripts/config -e HW_RANDOM_VIRTIO \
    # Use ARM PL031 RTC and synchronize time during boot
    && ./scripts/config -e RTC_DRV_PL031 -e RTC_HCTOSYS \
    # Enable vHCI driver to add support for Bluetooth virtualization
    && ./scripts/config -e BT_HCIVHCI -e CRYPTO_USER_API_HASH -e CRYPTO_USER_API_SKCIPHER \
    # Enable overlay FS and disable SMACK to workaround permission issues
    && ./scripts/config -e OVERLAY_FS -e SECURITY_SMACK_PERMISSIVE_MODE \
    # Enable 9P filesystem support to share files between host and guest
    && ./scripts/config -e INET -e PCI_HOST_GENERIC -e NET_9P -e NET_9P_VIRTIO -e 9P_FS \
    && make olddefconfig \
    # Compile
    && make -j$(nproc) zImage \
    && mv arch/arm/boot/zImage $TIZEN_IOT_QEMU_KERNEL \
    # Cleanup
    && rm -rf /tmp/workdir \
    && : # last line

# ------------------------------------------------------------------------------
# Create Tizen IoT File System
RUN set -x \
    && mkdir -p /tmp/workdir && cd /tmp/workdir \
    && SYSTEMD_SYSTEM=/usr/lib/systemd/system \
    && SYSTEMD_REQUIRES_LOCAL_FS=$SYSTEMD_SYSTEM/local-fs.target.requires \
    # Download Tizen images
    && wget --progress=dot:giga -r -nd --no-parent -e robots=off -A 'tizen-*.tar.gz' \
    http://download.tizen.org/snapshots/TIZEN/Tizen-$TIZEN_VERSION/Tizen-$TIZEN_VERSION-Unified/latest/images/standard/tizen-headless-armv7l/  \
    # Unpack
    && tar -xzf tizen-*.tar.gz \
    && mv system-data.img $TIZEN_IOT_IMAGE_DATA \
    && mv rootfs.img $TIZEN_IOT_IMAGE_ROOT \
    # Install libguestfs and linux-image-generic required for in-place
    # modifications of the rootfs image
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends libguestfs-tools linux-image-generic \
    # Add extra libraries to the root image
    && guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda copy-in \
    $TIZEN_SDK_TOOLCHAIN/arm-tizen-linux-gnueabi/lib/libasan.so.* \
    $TIZEN_SDK_TOOLCHAIN/arm-tizen-linux-gnueabi/lib/libatomic.so.* \
    $TIZEN_SDK_TOOLCHAIN/arm-tizen-linux-gnueabi/lib/libubsan.so.* \
    $TIZEN_SDK_SYSROOT/usr/lib/libbluetooth-api.so.* \
    $TIZEN_SDK_SYSROOT/usr/lib/libcapi-network-bluetooth.so.* \
    $TIZEN_SDK_SYSROOT/usr/lib/libcapi-network-thread.so.* \
    $TIZEN_SDK_SYSROOT/usr/lib/libnsd-dns-sd.so.* \
    /usr/lib/ \
    # Disable failing systemd services
    && guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda \
    glob rm $SYSTEMD_SYSTEM/clone_partitions.service : \
    glob rm $SYSTEMD_SYSTEM/deviced.service : \
    glob rm $SYSTEMD_SYSTEM/mm-resource-managerd.service : \
    glob rm $SYSTEMD_SYSTEM/mnt-inform.mount : \
    glob rm $SYSTEMD_SYSTEM/muse-server.* : \
    glob rm $SYSTEMD_SYSTEM/murphyd.service : \
    glob rm $SYSTEMD_SYSTEM/pulseaudio.service \
    # Mount Tizen system partition on /opt-ro instead of /opt
    && SYSTEMD_UNIT_OPT_RO_MOUNT=$SYSTEMD_SYSTEM/opt\\x2dro.mount \
    && guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda \
    mv $SYSTEMD_SYSTEM/opt.mount $SYSTEMD_UNIT_OPT_RO_MOUNT : \
    ln-sf $SYSTEMD_UNIT_OPT_RO_MOUNT $SYSTEMD_REQUIRES_LOCAL_FS : \
    mkdir /opt-ro \
    && virt-edit $TIZEN_IOT_IMAGE_ROOT -e 's#/opt#/opt-ro#g' $SYSTEMD_UNIT_OPT_RO_MOUNT \
    && virt-edit $TIZEN_IOT_IMAGE_ROOT -e 's#Options=#Options=ro,#' $SYSTEMD_UNIT_OPT_RO_MOUNT \
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
    | guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda upload - $SYSTEMD_UNIT_OPT_MOUNT : \
    ln-sf $SYSTEMD_UNIT_OPT_MOUNT $SYSTEMD_REQUIRES_LOCAL_FS \
    # Fix SMACK label for /tmp/.opt-work/work
    && SYSTEMD_UNIT_OPT_POST_MOUNT=$SYSTEMD_SYSTEM/opt.post-mount.service \
    && echo -n \
    "[Unit]\n" \
    "DefaultDependencies=no\n" \
    "RequiresMountsFor=/opt\n" \
    "[Service]\n" \
    "Type=oneshot\n" \
    "ExecStart=chsmack -a * /tmp/.opt-work/work\n" \
    "RemainAfterExit=yes\n" \
    | guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda upload - $SYSTEMD_UNIT_OPT_POST_MOUNT : \
    ln-sf $SYSTEMD_UNIT_OPT_POST_MOUNT $SYSTEMD_REQUIRES_LOCAL_FS \
    # Mount using 9P at /mnt/chip
    && SYSTEMD_UNIT_9P_MOUNT=$SYSTEMD_SYSTEM/mnt-chip.mount \
    && echo -n \
    "[Unit]\n" \
    "After=network.target\n" \
    "[Mount]\n" \
    "What=host0\n" \
    "Where=/mnt/chip\n" \
    "Type=9p\n" \
    "Options=nofail,trans=virtio\n" \
    | guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda upload - $SYSTEMD_UNIT_9P_MOUNT : \
    ln-sf $SYSTEMD_UNIT_9P_MOUNT $SYSTEMD_REQUIRES_LOCAL_FS : \
    mkdir /mnt/chip \
    # Setup auto-login for root user
    && SYSTEMD_UNIT_SERIAL_GETTY=$SYSTEMD_SYSTEM/serial-getty@.service \
    && virt-edit $TIZEN_IOT_IMAGE_ROOT -e \
    's#^ExecStart=.*#ExecStart=-/sbin/agetty -o "-p -f root" --noclear -a root --keep-baud 115200 %I \$TERM#' $SYSTEMD_UNIT_SERIAL_GETTY \
    # Run serial-getty.service after multi-user.target
    && SYSTEMD_WANTS_GRAPHICAL=$SYSTEMD_SYSTEM/graphical.target.wants \
    && SYSTEMD_WANTS_MULTI_USER=$SYSTEMD_SYSTEM/multi-user.target.wants \
    && guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda mv $SYSTEMD_WANTS_MULTI_USER/getty.target $SYSTEMD_WANTS_GRAPHICAL \
    && virt-edit $TIZEN_IOT_IMAGE_ROOT -e 's#^After=#Before=serial-getty\@ttyAMA0.service\nAfter=#' $SYSTEMD_SYSTEM/multi-user.target \
    # Execute launcher script after root login
    && echo -n \
    "[ -x /launcher.sh ] && /launcher.sh\n" \
    | guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda upload - /root/.profile \
    # Launcher script
    && echo -n \
    "#!/bin/bash\n" \
    "grep -q 'rootshell' /proc/cmdline && exit\n" \
    "runner=\$(grep -o 'runner=[^ ]*' /proc/cmdline | sed 's/runner=//')\n" \
    "if [[ -z \"\$runner\" ]]; then\n" \
    "  runner='/mnt/chip/runner.sh' \n" \
    "fi\n" \
    "if [[ -x \"\$runner\" ]]; then\n" \
    "  echo '### RUNNER START ###'\n" \
    "  \"\$runner\"\n" \
    "  echo '### RUNNER STOP:' \$?\n" \
    "else\n" \
    "  read -r -t 5 -p 'Press ENTER to access root shell...' && exit || echo ' timeout.'\n" \
    "fi\n" \
    "echo 'Shutting down emulated system...'\n" \
    "echo o > /proc/sysrq-trigger\n" \
    | guestfish --rw -a $TIZEN_IOT_IMAGE_ROOT -m /dev/sda upload - /launcher.sh : chmod 0755 /launcher.sh \
    # Remove ALL previously installed packages
    && apt-get autopurge -fy $(tail -2 /var/log/apt/history.log | grep ^Install | cut -c 10- | sed 's/([^)]*),\?//g') \
    && rm -rf /var/lib/apt/lists \
    && rm -rf /var/tmp/.guestfs-0 \
    # Cleanup
    && rm -rf /tmp/workdir \
    && : # last line

# ------------------------------------------------------------------------------
# Switch back to the non-root user
USER ubuntu
WORKDIR /home/ubuntu
