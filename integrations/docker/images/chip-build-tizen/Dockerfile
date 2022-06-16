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
# Install dependencies
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    cpio=2.13+dfsg-2 \
    obs-build=20180831-3ubuntu1 \
    openjdk-8-jre-headless \
    zip \
    # Cleanup
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* \
    && : # last line

# ------------------------------------------------------------------------------
# Install tizen SDK
ENV TIZEN_SDK_ROOT /opt/tizen-sdk
ENV TIZEN_VERSION 6.0

WORKDIR /tmp/workdir

# ------------------------------------------------------------------------------
# Get tizen studio CLI
RUN set -x \
    && mkdir -p $TIZEN_SDK_ROOT \
    # Download
    && wget --progress=dot:giga \
    http://download.tizen.org/sdk/tizenstudio/official/binary/certificate-encryptor_1.0.7_ubuntu-64.zip \
    http://download.tizen.org/sdk/tizenstudio/official/binary/certificate-generator_0.1.3_ubuntu-64.zip \
    http://download.tizen.org/sdk/tizenstudio/official/binary/new-common-cli_2.5.7_ubuntu-64.zip \
    http://download.tizen.org/sdk/tizenstudio/official/binary/new-native-cli_2.5.7_ubuntu-64.zip \
    http://download.tizen.org/sdk/tizenstudio/official/binary/sdb_4.2.23_ubuntu-64.zip \
    # Install tizen studio CLI
    && unzip -o '*.zip' \
    && cp -rf data/* $TIZEN_SDK_ROOT \
    && echo "TIZEN_SDK_INSTALLED_PATH=$TIZEN_SDK_ROOT" > $TIZEN_SDK_ROOT/sdk.info \
    && echo "TIZEN_SDK_DATA_PATH=$USER_HOME/tizen-sdk-data" >> $TIZEN_SDK_ROOT/sdk.info \
    && ln -sf $USER_HOME/.tizen-cli-config $TIZEN_SDK_ROOT/tools/.tizen-cli-config \
    # Cleanup
    && rm -rf /tmp/workdir \
    && : # last line

COPY secret-tool.py $TIZEN_SDK_ROOT/tools/certificate-encryptor/secret-tool
RUN chmod 0755 $TIZEN_SDK_ROOT/tools/certificate-encryptor/secret-tool
ENV PATH="$TIZEN_SDK_ROOT/tools/ide/bin:$TIZEN_SDK_ROOT/tools:$PATH"

# ------------------------------------------------------------------------------
# Get toolchain
RUN set -x \
    && mkdir -p $TIZEN_SDK_ROOT \
    # Download
    && wget --progress=dot:giga \
    http://download.tizen.org/sdk/tizenstudio/official/binary/cross-arm-gcc-9.2_0.1.9_ubuntu-64.zip \
    http://download.tizen.org/sdk/tizenstudio/official/binary/sbi-toolchain-gcc-9.2.cpp.app_2.2.16_ubuntu-64.zip \
    # Install toolchain
    && unzip -o '*.zip' \
    && cp -rf data/* $TIZEN_SDK_ROOT \
    # Cleanup
    && rm -rf /tmp/workdir \
    && : # last line

ENV TIZEN_SDK_TOOLCHAIN $TIZEN_SDK_ROOT/tools/arm-linux-gnueabi-gcc-9.2
ENV PATH="$TIZEN_SDK_TOOLCHAIN/bin:$PATH"

# ------------------------------------------------------------------------------
# Get tizen sysroot
ENV TIZEN_SDK_SYSROOT \
    $TIZEN_SDK_ROOT/platforms/tizen-$TIZEN_VERSION/mobile/rootstraps/mobile-$TIZEN_VERSION-device.core
RUN set -x \
    # Base sysroot
    && wget --progress=dot:giga \
    http://download.tizen.org/sdk/tizenstudio/official/binary/mobile-$TIZEN_VERSION-core-add-ons_0.0.262_ubuntu-64.zip \
    http://download.tizen.org/sdk/tizenstudio/official/binary/mobile-$TIZEN_VERSION-rs-device.core_0.0.123_ubuntu-64.zip \
    # Base packages
    && wget --progress=dot:mega -r -nd --no-parent \
    http://download.tizen.org/releases/milestone/tizen/base/latest/repos/standard/packages/armv7l/ \
    -A 'iniparser-*.armv7l.rpm' \
    -A 'libblkid-devel-*.armv7l.rpm' \
    -A 'libcap-*.armv7l.rpm' \
    -A 'libffi-devel-*.armv7l.rpm' \
    -A 'liblzma-*.armv7l.rpm' \
    -A 'libmount-devel-*.armv7l.rpm' \
    -A 'libncurses6-*.armv7l.rpm' \
    -A 'libreadline-*.armv7l.rpm' \
    -A 'libuuid-*.armv7l.rpm' \
    -A 'pcre-devel-*.armv7l.rpm' \
    -A 'readline-devel-*.armv7l.rpm' \
    -A 'xdgmime-*.armv7l.rpm' \
    # Unified packages
    && wget --progress=dot:mega -r -nd --no-parent \
    http://download.tizen.org/releases/milestone/tizen/unified/latest/repos/standard/packages/armv7l/ \
    -A 'aul-0*.armv7l.rpm' \
    -A 'aul-devel-*.armv7l.rpm' \
    -A 'bundle-0*.armv7l.rpm' \
    -A 'bundle-devel-*.armv7l.rpm' \
    -A 'buxton2-*.armv7l.rpm' \
    -A 'cynara-devel-*.armv7l.rpm' \
    -A 'dbus-1*.armv7l.rpm' \
    -A 'dbus-devel-*.armv7l.rpm' \
    -A 'dbus-libs-1*.armv7l.rpm' \
    -A 'glib2-devel-2*.armv7l.rpm' \
    -A 'json-glib-devel-*.armv7l.rpm' \
    -A 'libcynara-client-*.armv7l.rpm' \
    -A 'libcynara-commons-*.armv7l.rpm' \
    -A 'libdns_sd-*.armv7l.rpm' \
    -A 'libjson-glib-*.armv7l.rpm' \
    -A 'libsessiond-0*.armv7l.rpm' \
    -A 'libsystemd-*.armv7l.rpm' \
    -A 'libtzplatform-config-*.armv7l.rpm' \
    -A 'parcel-0*.armv7l.rpm' \
    -A 'parcel-devel-*.armv7l.rpm' \
    -A 'pkgmgr-info-*.armv7l.rpm' \
    -A 'vconf-compat-*.armv7l.rpm' \
    -A 'vconf-internal-keys-devel-*.armv7l.rpm' \
    # Unified packages (snapshots)
    && wget --progress=dot:mega -r -nd --no-parent \
    http://download.tizen.org/snapshots/tizen/unified/latest/repos/standard/packages/armv7l/ \
    -A 'capi-network-nsd-*.armv7l.rpm' \
    -A 'capi-network-thread-*.armv7l.rpm' \
    -A 'libnsd-dns-sd-*.armv7l.rpm' \
    # Install base sysroot
    && unzip -o '*.zip' \
    && cp -rf data/* $TIZEN_SDK_ROOT \
    # Install packages
    && unrpm *.rpm \
    && cp -rf lib usr $TIZEN_SDK_SYSROOT \
    # Make symbolic links relative
    && for LNK in $(find $TIZEN_SDK_SYSROOT/usr/lib -maxdepth 1 -type l); \
    do ln -sf $(basename $(readlink "$LNK")) "$LNK" ; done \
    && ln -sf ../../lib/libcap.so.2 $TIZEN_SDK_SYSROOT/usr/lib/libcap.so \
    && ln -sf openssl1.1.pc $TIZEN_SDK_SYSROOT/usr/lib/pkgconfig/openssl.pc \
    # Cleanup
    && rm -rf /tmp/workdir \
    && : # last line

# ------------------------------------------------------------------------------
# Switch to the non-root user
USER $USER_NAME
WORKDIR $USER_HOME
