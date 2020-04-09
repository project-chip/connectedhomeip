# start with Ubuntu 18.04LTS
FROM ubuntu:bionic

VOLUME "/var/source"

# base build and check tools and libraries layer
RUN set -x \
    && apt-get update \
    && apt-get install -fy \
         git \
         curl \
         jq \
         make \
         autoconf \
         automake \
         libtool \
         pkg-config \
         g++ \
         clang-9 \
         clang-format-9 \
         clang-tidy-9 \
         lcov \
         shellcheck \
         libssl-dev \
         unzip \
         wget \
    && : # last line

# BabbleSim layer
RUN set -x \
    && apt-get install -fy \
         gcc-multilib \
         libfftw3-dev \
    && (mkdir -p /var/bsim \
    && cd /var/bsim \
    && curl https://storage.googleapis.com/git-repo-downloads/repo > ./repo  && chmod a+x ./repo \
    && ./repo init -u https://github.com/BabbleSim/manifest.git -m everything.xml -b v1.0.3 --depth=1 \
    && ./repo sync \
    && make everything -j 8) \
    && : # last line

# Install specific release of openssl
RUN set -x \
    && cd /tmp && wget https://github.com/openssl/openssl/archive/OpenSSL_1_1_1f.zip \
    && mkdir -p /tmp/openssl && cd /tmp/openssl && unzip ../OpenSSL_1_1_1f.zip \
    && cd /tmp/openssl/openssl-OpenSSL_1_1_1f && ./config && make && make install \
    && : # last line

# nRF5 SDK, needed for building Nordic Platform code
RUN set -x \
    && curl -o /tmp/nRF5SDKforThreadandZigbee.zip \
    https://www.nordicsemi.com/-/media/Software-and-other-downloads/SDKs/nRF5-SDK-for-Thread/nRF5-SDK-for-Thread-and-Zigbee/nRF5SDKforThreadandZigbeev400dc7186b.zip \
    && (mkdir /var/nRF5_SDK_for_Thread_and_Zigbee \
    && cd /var/nRF5_SDK_for_Thread_and_Zigbee \
    && unzip /tmp/nRF5SDKforThreadandZigbee.zip) \
    && : # last line

# Tools for flashing software on Nordic devices, and accessing device logs
RUN set -x \
    && (mkdir /var/nRF5_tools && cd /var/nRF5_tools \
    && curl https://www.nordicsemi.com/-/media/Software-and-other-downloads/Desktop-software/nRF-command-line-tools/sw/Versions-10-x-x/10-7-0/nRFCommandLineTools1070Linuxamd64tar.gz \
    | tar zxvf - \
    && dpkg -i JLink_Linux_*.deb \
    && dpkg -i nRF-Command-Line-Tools_*.deb \
    && tar zxvf nRF-Command-Line-Tools_*.tar.gz) \
    && : # last line

# GNU ARM Embedded toolchain, cross compiler for various platform builds
RUN set -x \
    && (cd /var \
    && curl https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/9-2019q4/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2 \
    | tar jxvf -) \
    && : # last line
