ARG VERSION=1
FROM ghcr.io/project-chip/chip-build-java:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
    openjdk-11-jdk \
    rsync \
    swig \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

# Download and install android SDK
RUN set -x \
    && wget -O /tmp/android-30.zip https://dl.google.com/android/repository/platform-30_r03.zip \
    && mkdir -p /opt/android/sdk/platforms \
    && cd /opt/android/sdk/platforms \
    && unzip /tmp/android-30.zip \
    && mv android-11 android-30 \
    && rm -f /tmp/android-30.zip \
    && chmod -R a+rX /opt/android/sdk \
    && test -d /opt/android/sdk/platforms/android-30 \
    && : # last line

# Download and install android command line tool (for installing `sdkmanager`)
# We need create 10.0 folder inide cmdline-tools, since latest android commandline tool looks for this latest folder
# when running sdkmanager --licenses
RUN set -x \
    && wget -O /tmp/cmdline-tools.zip https://dl.google.com/android/repository/commandlinetools-linux-9862592_latest.zip \
    && cd /opt/android/sdk \
    && mkdir -p temp \
    && unzip /tmp/cmdline-tools.zip -d temp \
    && mkdir -p cmdline-tools/10.0 \
    && cp -rf temp/cmdline-tools/* cmdline-tools/10.0 \
    && rm -rf temp \
    && test -d /opt/android/sdk/cmdline-tools \
    && : # last line

# Download and install android NDK
RUN set -x \
    && cd /tmp \
    && wget https://dl.google.com/android/repository/android-ndk-r23c-linux.zip \
    && mkdir -p /opt/android \
    && cd /opt/android \
    && unzip /tmp/android-ndk-r23c-linux.zip \
    && rm -f /tmp/android-ndk-r23c-linux.zip \
    && : # last line

# Install specific release of openssl and compile for x86/armv7-a
# Leaves compiled .so's in /usr/local/src/{arch}/openssl-{version} dirs
# CHIP builds may configure with --with-openssl using one of the arch dirs
# NOTE: CHIP builds will also need to either link or copy libc++_shared.so from
# the NDK and libcrypto.so (and any other deps) into the jniLibs for any APK that
# is to be built with this image.
# See also: https://github.com/openssl/openssl/issues/18561#issuecomment-1155298077
RUN set -x \
    && export OPENSSL_ARMV7=/usr/local/src/armeabi-v7a \
    && export OPENSSL_X86=/usr/local/src/x86 \
    && export ANDROID_NDK_HOME=/opt/android/android-ndk-r23c \
    && export PATH=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH \
    && cd /tmp && wget https://www.openssl.org/source/openssl-1.1.1t.tar.gz \
    && mkdir -p $OPENSSL_ARMV7 && cd $OPENSSL_ARMV7 && tar xfz /tmp/openssl-1.1.1t.tar.gz \
    && cd $OPENSSL_ARMV7/openssl-1.1.1t && CC=clang ANDROID_API=26 ./Configure android-arm -U__ANDROID_API__ -D__ANDROID_API__=26 && make -j$(nproc) SHLIB_VERSION_NUMBER= SHLIB_EXT=.so \
    && mkdir -p $OPENSSL_X86 && cd $OPENSSL_X86 && tar xfz /tmp/openssl-1.1.1t.tar.gz \
    && cd $OPENSSL_X86/openssl-1.1.1t && CC=clang ANDROID_API=26 ./Configure android-x86 -U__ANDROID_API__ -D__ANDROID_API__=26 && make  -j$(nproc) SHLIB_VERSION_NUMBER= SHLIB_EXT=.so \
    && rm -rf /tmp/openssl-1.1.1t.tar.gz \
    && : # last line

ENV ANDROID_HOME=/opt/android/sdk
ENV ANDROID_NDK_HOME=/opt/android/android-ndk-r23c
ENV JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64
