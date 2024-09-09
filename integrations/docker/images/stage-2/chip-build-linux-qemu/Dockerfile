ARG VERSION=latest
ARG UBUNTU_QEMU_DIR_DEFAULT="/opt/ubuntu-qemu"
ARG UBUNTU_QEMU_IMG_DEFAULT="${UBUNTU_QEMU_DIR_DEFAULT}/ubuntu-24.04.img"

FROM ghcr.io/project-chip/chip-build:${VERSION} as build-env
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

ARG BLUEZ_VERSION=5.72
ARG ELL_VERSION=0.62
ARG KERNEL_VERSION=6.7.3
ARG UBUNTU_QEMU_DIR_DEFAULT
ARG UBUNTU_QEMU_IMG_DEFAULT

ENV UBUNTU_QEMU_DIR=${UBUNTU_QEMU_DIR_DEFAULT}
ENV UBUNTU_QEMU_IMG=${UBUNTU_QEMU_IMG_DEFAULT}
ENV KERNEL_PATH="${UBUNTU_QEMU_DIR_DEFAULT}/bzImage"


RUN mkdir -p /tmp/workdir/linux
COPY files/linux/0001-Bluetooth-MGMT-Synchronize-scan-start-and-LE-Meta-ev.patch /tmp/workdir/linux/0001-Bluetooth-MGMT-Synchronize-scan-start-and-LE-Meta-ev.patch
COPY files/bluetooth/main.conf /tmp/workdir/main.conf
RUN set -x \
	&& apt-get update \
	&& DEBIAN_FRONTEND=noninteractive apt-get install -fy \
	bc \
	cpio \
	dwarves \
	elfutils \
	fakeroot \
	libdw-dev \
	libelf-dev \
	libell-dev \
	libell0 \
	libguestfs-tools \
	linux-image-generic \
	ncurses-dev \
	patch \
	qemu-system \
	xz-utils \
	zstd \
	&& apt-get clean \
	&& rm -rf /var/lib/apt/lists/* \
	&& rm -rf /var/cache/apt/* \
	&& : # last line

# Download Linux kernel source
RUN mkdir -p /tmp/workdir/linux \
	&& export MAKEFLAGS=-j$(nproc) \
	&& cd /tmp/workdir \
	&& curl https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-${KERNEL_VERSION}.tar.xz -o /tmp/workdir/linux-${KERNEL_VERSION}.tar.xz \
	&& tar -xJf linux-${KERNEL_VERSION}.tar.xz -C /tmp/workdir/linux --strip-components=1 \
	&& rm /tmp/workdir/linux-${KERNEL_VERSION}.tar.xz \
	# Set configuration for btvirt
	&& cd /tmp/workdir/linux \
	&& patch -p1 < /tmp/workdir/linux/0001-Bluetooth-MGMT-Synchronize-scan-start-and-LE-Meta-ev.patch \
	&& make x86_64_defconfig \
	&& ./scripts/config -e BT \
	&& ./scripts/config -e BT_BREDR \
	&& ./scripts/config -e BT_HCIVHCI \
	&& ./scripts/config -e CONFIG_BRIDGE \
	&& ./scripts/config -e CONFIG_CRYPTO_AES \
	&& ./scripts/config -e CONFIG_CRYPTO_CMAC \
	&& ./scripts/config -e CONFIG_CRYPTO_ECB \
	&& ./scripts/config -e CONFIG_CRYPTO_USER \
	&& ./scripts/config -e CONFIG_CRYPTO_USER_API_HASH \
	&& ./scripts/config -e CONFIG_CRYPTO_USER_API_SKCIPHER \
	&& ./scripts/config -e CONFIG_VETH \
	&& ./scripts/config -e MAC80211 \
	&& ./scripts/config -e MAC80211_HWSIM \
	# Compile
	&& make olddefconfig  \
	&& make \
	&& mkdir -p /opt/ubuntu-qemu/rootfs \
	&& make modules_install INSTALL_MOD_PATH=/opt/ubuntu-qemu/rootfs \
	&& cp /tmp/workdir/linux/arch/x86/boot/bzImage /opt/ubuntu-qemu/bzImage \
	# Build bluez
	&& git clone git://git.kernel.org/pub/scm/libs/ell/ell.git /tmp/workdir/ell --depth 1 --branch ${ELL_VERSION} \
	&& git clone https://github.com/bluez/bluez.git /tmp/workdir/bluez --depth 1 --branch ${BLUEZ_VERSION} \
	&& cd /tmp/workdir/bluez \
	&& ./bootstrap \
	&& ./configure \
	--enable-backtrace \
	--enable-debug \
	--enable-deprecated \
	--enable-experimental \
	--enable-library \
	--enable-monitor \
	--enable-pie \
	--enable-test \
	--enable-testing \
	--enable-tools \
	--enable-tools \
	--enable-udev \
	--disable-a2dp \
	--disable-avrcp \
	--disable-bap \
	--disable-bass \
	--disable-csip \
	--disable-cups \
	--disable-cups \
	--disable-health \
	--disable-hid \
	--disable-hid2hci \
	--disable-hog \
	--disable-manpages \
	--disable-mcp  \
	--disable-mesh \
	--disable-micp \
	--disable-midi \
	--disable-network \
	--disable-obex \
	--disable-optimization \
	--disable-sap \
	--disable-silent-rules \
	--disable-vcp \
	--prefix=/usr \
	--mandir=/usr/share/man \
	--sysconfdir=/etc \
	--localstatedir=/var \
	--with-systemdsystemunitdir=/lib/systemd/system \
	--with-systemduserunitdir=/usr/lib/systemd \
	&& make \
	&& make install DESTDIR=/opt/ubuntu-qemu/rootfs && mkdir -p /opt/ubuntu-qemu/rootfs/usr/bin && cp /tmp/workdir/bluez/emulator/btvirt /opt/ubuntu-qemu/rootfs/usr/bin \
	# Download Ubuntu image for QEMU
	&& curl https://cloud-images.ubuntu.com/minimal/releases/noble/release/ubuntu-24.04-minimal-cloudimg-amd64.img \
	-o /tmp/workdir/ubuntu-24.04-minimal-cloudimg-amd64.img
	# Prepare ubuntu image
RUN qemu-img create -f qcow2 -o preallocation=off $UBUNTU_QEMU_IMG 10G \
	&& virt-resize --expand /dev/sda1 /tmp/workdir/ubuntu-24.04-minimal-cloudimg-amd64.img $UBUNTU_QEMU_IMG \
	&& guestfish -a $UBUNTU_QEMU_IMG \
	--mount /dev/sda4:/ \
	--network \
	copy-in /opt/ubuntu-qemu/rootfs/lib /usr : \
	copy-in /opt/ubuntu-qemu/rootfs/usr / : \
	sh 'apt-get remove -y snapd' : \
	sh '/usr/bin/systemctl enable bluetooth.service' : \
	sh '/usr/bin/systemctl disable cloud-init.service' : \
	sh '/usr/bin/systemctl disable lxd-agent.service' : \
	sh '/usr/bin/systemctl disable systemd-networkd-wait-online.service' : \
	sh '/usr/bin/systemctl disable systemd-timesyncd.service' : \
	sh '/usr/bin/systemctl mask cloud-init.service' : \
	sh '/usr/bin/systemctl mask lxd-agent.service' : \
	sh '/usr/bin/systemctl mask systemd-networkd-wait-online.service' : \
	sh '/usr/bin/systemctl mask systemd-timesyncd.service' : \
	sh 'passwd -d root' : \
	sh 'ssh-keygen -A' : \
	sh '/bin/echo -e "PermitRootLogin yes\nPasswordAuthentication yes\nPermitEmptyPasswords yes" > /etc/ssh/sshd_config' : \
	mkdir-p "/etc/netplan" : \
	sh '/bin/echo -e "network:\n  version: 2\n  renderer: networkd\n  ethernets:\n    enp0s4:\n      dhcp4: true\n" > /etc/netplan/01-netcfg.yaml' : \
	sh 'chmod -R 700 /etc/netplan' : \
	sh 'sed -i "s#^ExecStart=.*#ExecStart=-/sbin/agetty -o \"-p -- \\\\\\\\u\" -a root --keep-baud 115200,38400,9600 %I \$TERM#" "/usr/lib/systemd/system/serial-getty@.service"' : \
	mkdir-p "/etc/bluetooth" : \
	copy-in /tmp/workdir/main.conf /etc/bluetooth : \
	sh 'sed -i "s#^ExecStart=.*#ExecStart=-/usr/libexec/bluetooth/bluetoothd -E#" /lib/systemd/system/bluetooth.service' : \
	sh 'rm -f /etc/resolv.conf && /bin/echo -e "nameserver 8.8.8.8" > /etc/resolv.conf' : \
	sh '/bin/echo -e "host0 /chip 9p trans=virtio,version=9p2000.L 0 0" >> /etc/fstab' : \
	sh '/bin/echo -e "[ -x /init.sh ] && /init.sh && rm -f /init.sh && sync && systemctl poweroff\n" >> /root/.profile' : \
	sh '/bin/echo -e "export PW_ENVIRONMENT_ROOT=/root/pw_root\n[ -x /launcher.sh ] && /launcher.sh\n" >> /root/.profile' \
	&& echo -n \
	"#!/bin/bash\n" \
	"set -e \n" \
	"export DEBIAN_FRONTEND=noninteractive \n" \
	"apt-get update\n" \
	"apt-get install -y apt-utils\n" \
	"apt-get install -y dnsmasq hostapd wpasupplicant iw libdw1 rfkill\n" \
	"DEBIAN_FRONTEND=noninteractive apt-get -y install " \
	"	g++ " \
	"	gcc " \
	"	git " \
	"	libavahi-client-dev " \
	"	libcairo2-dev " \
	"	libdbus-1-dev " \
	"	libgirepository1.0-dev " \
	"	libglib2.0-dev " \
	"	libreadline-dev " \
	"	libssl-dev " \
	"	ninja-build " \
	"	pkg-config " \
	"	python3 " \
	"	python3-dev " \
	"	python3-pip " \
	"	python3-venv " \
	"	unzip \n" \
	"/usr/bin/systemctl disable dbus-fi.w1.wpa_supplicant1.service\n" \
	"/usr/bin/systemctl disable dnsmasq.service\n" \
	"/usr/bin/systemctl disable hostapd.service\n" \
	"/usr/bin/systemctl disable wpa_supplicant.service\n" \
	"/usr/bin/systemctl mask dnsmasq.service\n" \
	"/usr/bin/systemctl mask hostapd.service\n" \
	"/usr/bin/systemctl mask dbus-fi.w1.wpa_supplicant1.service\n" \
	"/usr/bin/systemctl mask wpa_supplicant.service\n" \
	"git config --file /root/.gitconfig  --add safe.directory \"*\"\n" \
	"apt-get clean\n" \
	"rm -rf /var/lib/apt/lists/*\n" \
	"rm -rf /var/cache/apt/*\n" \
	"echo Configuration completed\n " \
	| guestfish --rw -a $UBUNTU_QEMU_IMG -m /dev/sda4:/ upload - /init.sh : chmod 0755 /init.sh \
	&& qemu-system-x86_64 \
	-machine ubuntu \
	-smp 4 \
	-m 2048 \
	-nographic \
	-monitor null \
	-serial stdio \
	-display none \
	-device virtio-blk-pci,drive=virtio-blk1 \
	-drive file="${UBUNTU_QEMU_IMG}",id=virtio-blk1,if=none,format=qcow2,readonly=off \
	-kernel "${KERNEL_PATH}" \
	-append 'console=ttyS0 root=/dev/vda4' \
	-netdev user,id=network0 \
	-device e1000,netdev=network0,mac=52:54:00:12:34:56 \
    -virtfs "local,path=/tmp,mount_tag=host0,security_model=passthrough,id=host0" \
# tmp folder is mounted only to preserve error during boot
	&& mkdir -p /chip \
	&& rm -rf /opt/ubuntu-qemu/rootfs \
	&& echo -n \
	"#!/bin/bash\n" \
	"grep -q 'rootshell' /proc/cmdline && exit\n" \
	"if [[ -x /chip/runner.sh ]]; then\n" \
	"  echo '### RUNNER START ###'\n" \
	"  cd /chip\n" \
	"  bash /chip/runner.sh\n" \
	"  status=\$?\n" \
	"  echo \"### RUNNER STOP, RETURN: \$status\"\n" \
	"  echo \$status > /chip/runner_status\n" \
	"else\n" \
	"  read -r -t 5 -p 'Press ENTER to access root shell...' && exit || echo ' timeout.'\n" \
	"fi\n" \
	"echo 'Shutting down emulated system...'\n" \
	"echo o > /proc/sysrq-trigger\n" \
	| guestfish --rw -a $UBUNTU_QEMU_IMG -m /dev/sda4:/ upload - /launcher.sh : chmod 0755 /launcher.sh \
	&& virt-sparsify --compress ${UBUNTU_QEMU_IMG} ${UBUNTU_QEMU_IMG}.compressed \
	&& mv ${UBUNTU_QEMU_IMG}.compressed ${UBUNTU_QEMU_IMG} \
	&& rm -rf /var/tmp/.guestfs-0/* \
	&& rm -rf /tmp/* \
	&& : # last line

FROM ghcr.io/project-chip/chip-build:${VERSION}

ARG UBUNTU_QEMU_DIR_DEFAULT
ARG UBUNTU_QEMU_IMG_DEFAULT

ENV UBUNTU_QEMU_DIR=${UBUNTU_QEMU_DIR_DEFAULT}
ENV UBUNTU_QEMU_IMG=${UBUNTU_QEMU_IMG_DEFAULT}
ENV PW_ENVIRONMENT_ROOT="/root/pw_root"

RUN set -x \
	&& apt-get update \
	&& DEBIAN_FRONTEND=noninteractive apt-get install -fy \
	cpu-checker \
	libgirepository-1.0-1 \
	qemu-system-x86 \
	&& apt-get clean \
	&& rm -rf /var/lib/apt/lists/* \
	&& rm -rf /var/cache/apt/* \
	&& : # last line
COPY --from=build-env ${UBUNTU_QEMU_DIR} ${UBUNTU_QEMU_DIR}

WORKDIR /chip
