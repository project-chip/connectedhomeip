ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# ------------------------------------------------------------------------------
# Add group/user for tizen
ARG USER_NAME=tizen
ARG USER_UID=1000
ARG USER_GID=$USER_UID
ENV USER_HOME /home/$USER_NAME

RUN set -x \
    && groupadd -g $USER_GID $USER_NAME \
    && useradd -m $USER_NAME -s /bin/bash -u $USER_UID -g $USER_GID -G sudo -l \
    && : # last line

# ------------------------------------------------------------------------------
# Install toolchain
RUN set -x \
    && apt-get update \
    && apt-get install -fy --no-install-recommends obs-build=20180831-3ubuntu1 cpio=2.13+dfsg-2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* \
    && : # last line

# ------------------------------------------------------------------------------
# Install tizen SDK
ENV TIZEN_HOME /opt/tizen_sdk
ENV TIZEN_ROOTSTRAP_VERSION 6.0
ENV TIZEN_ROOTSTRAPS mobile-$TIZEN_ROOTSTRAP_VERSION-rs-device.core_0.0.123_ubuntu-64
ENV DNS_LIBS libnsd-dns-sd.so*

WORKDIR $TIZEN_HOME

RUN set -x \
    && wget --progress=dot:giga http://download.tizen.org/sdk/tizenstudio/official/binary/$TIZEN_ROOTSTRAPS.zip \
    && unzip $TIZEN_ROOTSTRAPS.zip \
    && mv data/platforms/tizen-$TIZEN_ROOTSTRAP_VERSION/mobile/rootstraps/mobile-$TIZEN_ROOTSTRAP_VERSION-device.core/usr . \
    && mv data/platforms/tizen-$TIZEN_ROOTSTRAP_VERSION/mobile/rootstraps/mobile-$TIZEN_ROOTSTRAP_VERSION-device.core/lib . \
    && rm -rf usr/lib/$DNS_LIBS \
    && rm -rf data \
    && rm -rf mobile-$TIZEN_ROOTSTRAP_VERSION-rs-device.* \
    && rm pkginfo.manifest \
    && : # last line

# ------------------------------------------------------------------------------
# Get tizen packages
RUN set -x \
    # Base packages
    && wget -r -nd --no-parent -q -A 'pcre-devel-*.armv7l.rpm' http://download.tizen.org/releases/milestone/tizen/base/latest/repos/standard/packages/armv7l/ \
    && wget -r -nd --no-parent -q -A 'libffi-devel-*.armv7l.rpm' http://download.tizen.org/releases/milestone/tizen/base/latest/repos/standard/packages/armv7l/ \
    && wget -r -nd --no-parent -q -A 'libmount-devel-*.armv7l.rpm' http://download.tizen.org/releases/milestone/tizen/base/latest/repos/standard/packages/armv7l/ \
    && wget -r -nd --no-parent -q -A 'libblkid-devel-*.armv7l.rpm' http://download.tizen.org/releases/milestone/tizen/base/latest/repos/standard/packages/armv7l/ \
    && wget -r -nd --no-parent -q -A 'libcap-*.armv7l.rpm' http://download.tizen.org/releases/milestone/tizen/base/latest/repos/standard/packages/armv7l/ \
    && wget -r -nd --no-parent -q -A 'liblzma-*.armv7l.rpm' http://download.tizen.org/releases/milestone/tizen/base/latest/repos/standard/packages/armv7l/ \
    # Unified packages
    && wget -r -nd --no-parent -q -A 'vconf-compat-*.armv7l.rpm' http://download.tizen.org/releases/milestone/tizen/unified/latest/repos/standard/packages/armv7l/ \
    && wget -r -nd --no-parent -q -A 'libcynara-commons-*.armv7l.rpm' http://download.tizen.org/releases/milestone/tizen/unified/latest/repos/standard/packages/armv7l/ \
    && wget -r -nd --no-parent -q -A 'cynara-devel-*.armv7l.rpm' http://download.tizen.org/releases/milestone/tizen/unified/latest/repos/standard/packages/armv7l/ \
    && wget -r -nd --no-parent -q -A 'libcynara-client-*.armv7l.rpm' http://download.tizen.org/releases/milestone/tizen/unified/latest/repos/standard/packages/armv7l/ \
    && wget -r -nd --no-parent -q -A 'dbus-1*.armv7l.rpm' http://download.tizen.org/releases/milestone/tizen/unified/latest/repos/standard/packages/armv7l/ \
    && wget -r -nd --no-parent -q -A 'dbus-devel-*.armv7l.rpm' http://download.tizen.org/releases/milestone/tizen/unified/latest/repos/standard/packages/armv7l/ \
    && wget -r -nd --no-parent -q -A 'dbus-libs-1*.armv7l.rpm' http://download.tizen.org/releases/milestone/tizen/unified/latest/repos/standard/packages/armv7l/ \
    && wget -r -nd --no-parent -q -A 'glib2-devel-2*.armv7l.rpm' http://download.tizen.org/releases/milestone/tizen/unified/latest/repos/standard/packages/armv7l/ \
    && wget -r -nd --no-parent -q -A 'libdns_sd-*.armv7l.rpm' http://download.tizen.org/releases/milestone/tizen/unified/latest/repos/standard/packages/armv7l/ \
    && wget -r -nd --no-parent -q -A 'buxton2-*.armv7l.rpm' http://download.tizen.org/releases/milestone/tizen/unified/latest/repos/standard/packages/armv7l/ \
    && wget -r -nd --no-parent -q -A 'libsystemd-*.armv7l.rpm' http://download.tizen.org/releases/milestone/tizen/unified/latest/repos/standard/packages/armv7l/ \
    && wget -r -nd --no-parent -q -A 'capi-network-nsd-*.armv7l.rpm' http://download.tizen.org/snapshots/tizen/unified/latest/repos/standard/packages/armv7l/ \
    && wget -r -nd --no-parent -q -A 'libnsd-dns-sd-*.armv7l.rpm' http://download.tizen.org/snapshots/tizen/unified/latest/repos/standard/packages/armv7l/ \
    && unrpm ./*.rpm \
    && cp usr/lib/pkgconfig/openssl1.1.pc usr/lib/pkgconfig/openssl.pc \
    && rm usr/lib/libdns_sd.so \
    && cp usr/lib/libdns_sd.so.878.* usr/lib/libdns_sd.so \
    && rm ./*.rpm \
    && : # last line

# ------------------------------------------------------------------------------
# Get toolchain
RUN set -x \
    && wget --progress=dot:giga http://download.tizen.org/sdk/tizenstudio/official/binary/cross-arm-gcc-9.2_0.1.9_ubuntu-64.zip \
    && unzip cross-arm-gcc-9.2_0.1.9_ubuntu-64.zip \
    && cp -rf data/tools/arm-linux-gnueabi-gcc-9.2/* . \
    && rm pkginfo.manifest \
    && rm changelog \
    && rm -rf data \
    && rm cross-arm-gcc-9.2_0.1.9_ubuntu-64.zip \
    && : # last line

# ------------------------------------------------------------------------------
# Switch to the non-root user
USER $USER_NAME
WORKDIR $USER_HOME
