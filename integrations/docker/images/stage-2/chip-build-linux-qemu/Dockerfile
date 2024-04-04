ARG VERSION=latest
FROM ghcr.io/project-chip/chip-build:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

ARG KERNEL_VERSION=6.7.3
ARG BLUEZ_VERSION=5.72
ARG ELL_VERSION=0.62

ENV UBUNTU_QEMU_IMG="/opt/ubuntu-qemu/ubuntu-20.04.img"
ENV PW_ENVIRONMENT_ROOT="/root/pw_root"

RUN mkdir -p /tmp/workdir/linux
COPY files/linux/0001-Bluetooth-MGMT-Synchronize-scan-start-and-LE-Meta-ev.patch /tmp/workdir/linux/0001-Bluetooth-MGMT-Synchronize-scan-start-and-LE-Meta-ev.patch
COPY run-img.sh /opt/ubuntu-qemu/run-img.sh
COPY files/bluetooth/main.conf /tmp/workdir/main.conf
RUN set -x \
	&& chmod +x /opt/ubuntu-qemu/run-img.sh \
	&& apt-get update \
	&& DEBIAN_FRONTEND=noninteractive apt-get install -fy apt-utils \
	&& DEBIAN_FRONTEND=noninteractive apt-get install -fy \
	bc \
	build-essential \
	cpio \
	dwarves \
	elfutils \
	fakeroot \
	flex \
	libdw-dev \
	libelf-dev \
	libell-dev \
	libell0 \
	libguestfs-tools \
	linux-image-generic \
	ncurses-dev \
	python3 \
	python3-docutils \
	qemu \
	xz-utils \
	zstd \
	&& apt-get clean \
	&& rm -rf /var/lib/apt/lists/ \
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
	&& ./scripts/config -e MAC80211 \
	&& ./scripts/config -e MAC80211_HWSIM \
	&& ./scripts/config -e BT \
	&& ./scripts/config -e BT_BREDR \
	&& ./scripts/config -e BT_HCIVHCI \
	&& ./scripts/config -e CONFIG_CRYPTO_USER \
	&& ./scripts/config -e CONFIG_CRYPTO_USER_API_SKCIPHER \
	&& ./scripts/config -e CONFIG_CRYPTO_AES \
	&& ./scripts/config -e CONFIG_CRYPTO_ECB \
	&& ./scripts/config -e CONFIG_CRYPTO_CMAC \
	&& ./scripts/config -e CONFIG_CRYPTO_USER_API_HASH \
	&& ./scripts/config -e CONFIG_VETH \
	&& ./scripts/config -e CONFIG_BRIDGE \
	# Compile
	&& make olddefconfig  \
	&& make \
	&& mkdir -p /opt/ubuntu-qemu/rootfs \
	&& make modules_install INSTALL_MOD_PATH=/opt/ubuntu-qemu/rootfs \
	&& cp /tmp/workdir/linux/arch/x86/boot/bzImage /opt/ubuntu-qemu/bzImage \
	# Download Ubuntu image for QEMU
	&& curl https://cloud-images.ubuntu.com/minimal/releases/focal/release/ubuntu-20.04-minimal-cloudimg-amd64.img \
	-o /tmp/workdir/ubuntu-20.04-minimal-cloudimg-amd64.img \
	# Build bluez
	&& git clone git://git.kernel.org/pub/scm/libs/ell/ell.git /tmp/workdir/ell --depth 1 --branch ${ELL_VERSION} \
	&& git clone https://github.com/bluez/bluez.git /tmp/workdir/bluez --depth 1 --branch ${BLUEZ_VERSION} \
	&& cd /tmp/workdir/bluez \
	&& ./bootstrap \
	&& ./configure \
	--disable-silent-rules \
	--enable-pie \
	--enable-tools \
	--enable-test \
	--enable-library \
	--enable-testing \
	--enable-udev \
	--enable-experimental \
	--enable-deprecated \
	--enable-backtrace \
	--enable-debug \
	--enable-tools \
	--enable-monitor \
	--disable-cups \
	--disable-sap \
	--disable-hid2hci \
	--disable-a2dp \
	--disable-avrcp \
	--disable-network \
	--disable-hid \
	--disable-hog \
	--disable-health \
	--disable-bap \
	--disable-bass \
	--disable-mcp  \
	--disable-vcp \
	--disable-micp \
	--disable-csip \
	--disable-cups \
	--disable-mesh \
	--disable-midi \
	--disable-obex \
	--disable-optimization \
	--prefix=/usr \
	--mandir=/usr/share/man \
	--sysconfdir=/etc \
	--localstatedir=/var \
	--with-systemdsystemunitdir=/lib/systemd/system \
	--with-systemduserunitdir=/usr/lib/systemd \
	&& make \
	&& make install DESTDIR=/opt/ubuntu-qemu/rootfs && mkdir -p /opt/ubuntu-qemu/rootfs/usr/bin && cp /tmp/workdir/bluez/emulator/btvirt /opt/ubuntu-qemu/rootfs/usr/bin \
	# Prepare ubuntu image
	&& qemu-img create -f qcow2 -o preallocation=off $UBUNTU_QEMU_IMG 10G \
	&& virt-resize --expand /dev/sda1 /tmp/workdir/ubuntu-20.04-minimal-cloudimg-amd64.img $UBUNTU_QEMU_IMG \
	&& guestfish -a $UBUNTU_QEMU_IMG \
	--mount /dev/sda3:/ \
	--network \
	copy-in /opt/ubuntu-qemu/rootfs/lib /usr : \
	copy-in /opt/ubuntu-qemu/rootfs/usr / : \
	sh 'apt-get remove -y snapd' : \
	sh 'apt-get update' : \
	sh 'DEBIAN_FRONTEND=noninteractive apt-get install -y dnsmasq hostapd wpasupplicant iw libdw1 rfkill' : \
	sh '/usr/bin/systemctl enable bluetooth.service' : \
	sh '/usr/bin/systemctl disable systemd-networkd-wait-online.service' : \
	sh '/usr/bin/systemctl disable cloud-init.service' : \
	sh '/usr/bin/systemctl disable lxd-agent.service' : \
	sh '/usr/bin/systemctl disable hostapd.service' : \
	sh '/usr/bin/systemctl disable dnsmasq.service' : \
	sh '/usr/bin/systemctl disable dbus-fi.w1.wpa_supplicant1.service' : \
	sh '/usr/bin/systemctl disable wpa_supplicant.service' : \
	sh '/usr/bin/systemctl disable systemd-timesyncd.service' : \
	sh '/usr/bin/systemctl mask systemd-networkd-wait-online.service' : \
	sh '/usr/bin/systemctl mask cloud-init.service' : \
	sh '/usr/bin/systemctl mask lxd-agent.service' : \
	sh '/usr/bin/systemctl mask hostapd.service' : \
	sh '/usr/bin/systemctl mask dnsmasq.service' : \
	sh '/usr/bin/systemctl mask dbus-fi.w1.wpa_supplicant1.service' : \
	sh '/usr/bin/systemctl mask wpa_supplicant.service' : \
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
	sh '/bin/echo -e "export PW_ENVIRONMENT_ROOT=/root/pw_root\n[ -x /launcher.sh ] && /launcher.sh\n" >> /root/.profile' : \
	sh 'DEBIAN_FRONTEND=noninteractive apt-get -y install git gcc g++ pkg-config libssl-dev libdbus-1-dev libglib2.0-dev libavahi-client-dev ninja-build python3 python3-venv python3-dev python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev' : \
	sh 'git config --file /root/.gitconfig  --add safe.directory "*"' : \
	sh 'apt-get clean' : \
	sh 'rm -rf /var/lib/apt/lists' \
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
	| guestfish --rw -a $UBUNTU_QEMU_IMG -m /dev/sda3:/ upload - /launcher.sh : chmod 0755 /launcher.sh \
	&& rm -rf /var/tmp/.guestfs-0 \
	&& rm -rf /tmp/workdir

WORKDIR /chip
