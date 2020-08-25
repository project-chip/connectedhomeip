ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# Download and install android NDK
RUN set -x \
    && cd /tmp \
    && wget https://dl.google.com/android/repository/android-ndk-r21b-linux-x86_64.zip \
    && mkdir -p /usr/local/share \
    && cd /usr/local/share \
    && unzip /tmp/android-ndk-r21b-linux-x86_64.zip \
    && : # last line

# Install specific release of openssl and compile for x86/armv7-a
# Leaves compiled .so's in /usr/local/src/{arch}/openssl-1.1.1g dirs
# CHIP builds may configure with --with-openssl using one of the arch dirs
# NOTE: CHIP builds will also need to either link or copy libc++_shared.so from
# the NDK and libcrypto.so (and any other deps) into the jniLibs for any APK that
# is to be built with this image.
RUN set -x \
    && export OPENSSL_ARMV7=/usr/local/src/armeabi-v7a \
    && export OPENSSL_X86=/usr/local/src/x86 \
    && export ANDROID_NDK_HOME=/usr/local/share/android-ndk-r21b \
    && export PATH=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH \
    && cd /tmp && wget https://www.openssl.org/source/openssl-1.1.1g.tar.gz \
    && mkdir -p $OPENSSL_ARMV7 && cd $OPENSSL_ARMV7 && tar xfz /tmp/openssl-1.1.1g.tar.gz \
    && cd $OPENSSL_ARMV7/openssl-1.1.1g && CC=clang ANDROID_API=21 ./Configure android-arm -D__ANDROID_API__=21 && make SHLIB_VERSION_NUMBER= SHLIB_EXT=.so \
    && mkdir -p $OPENSSL_X86 && cd $OPENSSL_X86 && tar xfz /tmp/openssl-1.1.1g.tar.gz \
    && cd $OPENSSL_X86/openssl-1.1.1g && CC=clang ANDROID_API=21 ./Configure android-x86 -D__ANDROID_API__=21 && make SHLIB_VERSION_NUMBER= SHLIB_EXT=.so \
    && rm -rf /tmp/OpenSSL_1_1_1g.zip \
    && : # last line
