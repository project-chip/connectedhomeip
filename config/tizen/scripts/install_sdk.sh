#!/bin/bash

ROOTSTRAP_PATH="$1"
CHIPROOT=$(git rev-parse --show-toplevel)

NSD_DNS_LIBS="libnsd-dns-sd.so*"

echo "$ROOTSTRAP_PATH"

if [ -z "$ROOTSTRAP_PATH" ]; then
    echo "ROOTSTRAP_PATH should be input"
    exit 1
fi

if [[ ! -d $ROOTSTRAP_PATH ]]; then
    echo "Can't find the rootstrap dir, please make dir and run this script again"
    exit 1
fi

cd "$ROOTSTRAP_PATH"

# Get Tizen rootstrap
if [[ ! -f mobile-6.0-rs-device.core_0.0.123_ubuntu-64.zip ]]; then
    echo "Getting tizen rootstrap..."
    wget http://download.tizen.org/sdk/tizenstudio/official/binary/mobile-6.0-rs-device.core_0.0.123_ubuntu-64.zip
fi
unzip mobile-6.0-rs-device.core_0.0.123_ubuntu-64.zip
mv data/platforms/tizen-6.0/mobile/rootstraps/mobile-6.0-device.core/usr .
mv data/platforms/tizen-6.0/mobile/rootstraps/mobile-6.0-device.core/lib .

# Clean files
rm -rf data
rm -rf mobile-6.0-rs-device.*
rm pkginfo.manifest

# Copy internal dns-sd header into rootstrap
# After finishing ACR for internal APIs, we will remove this.

rm "$ROOTSTRAP_PATH/usr/lib/$NSD_DNS_LIBS"

# Get tizen packages
function download_pkg() {
    if [ "$2" = "snapshots" ]; then
        wget -r -nd --no-parent -A "$1" "http://download.tizen.org/snapshots/tizen/unified/latest/repos/standard/packages/armv7l/"
    else
        wget -r -nd --no-parent -A "$1" "http://download.tizen.org/releases/milestone/tizen/${2:-base}/latest/repos/standard/packages/armv7l/"
    fi
}

# Base package
for pkg in 'pcre-devel-*.armv7l.rpm' 'libffi-devel-*.armv7l.rpm' 'libmount-devel-*.armv7l.rpm' \
    'libblkid-devel-*.armv7l.rpm' 'libcap-*.armv7l.rpm' 'liblzma-*.armv7l.rpm'; do
    download_pkg "$pkg"
done

# Unified package
for pkg in 'vconf-compat-*.armv7l.rpm' 'libcynara-commons-*.armv7l.rpm' 'cynara-devel-*.armv7l.rpm' \
    'libcynara-client-*.armv7l.rpm' 'dbus-1*.armv7l.rpm' 'dbus-devel-*.armv7l.rpm' \
    'dbus-libs-1*.armv7l.rpm' 'glib2-devel-2*.armv7l.rpm' 'libdns_sd-*.armv7l.rpm' \
    'buxton2-*.armv7l.rpm' 'libsystemd-*.armv7l.rpm'; do
    download_pkg "$pkg" unified
done

# Latest snapshots package (For nsd)
for pkg in 'capi-network-nsd-0*.armv7l.rpm' 'capi-network-nsd-devel*.armv7l.rpm' 'libnsd-dns-sd*.armv7l.rpm' 'libdns_sd-*.armv7l.rpm'; do
    download_pkg "$pkg" snapshots
done

unrpm *.rpm

cp usr/lib/pkgconfig/openssl1.1.pc usr/lib/pkgconfig/openssl.pc

rm usr/lib/libdns_sd.so
cp usr/lib/libdns_sd.so.878.* usr/lib/libdns_sd.so

rm *.rpm

# Get toolchain
wget http://download.tizen.org/sdk/tizenstudio/official/binary/cross-arm-gcc-9.2_0.1.9_ubuntu-64.zip
unzip cross-arm-gcc-9.2_0.1.9_ubuntu-64.zip

cp -rf data/tools/arm-linux-gnueabi-gcc-9.2/* .

rm pkginfo.manifest
rm changelog
rm -rf data

rm cross-arm-gcc-9.2_0.1.9_ubuntu-64.zip
