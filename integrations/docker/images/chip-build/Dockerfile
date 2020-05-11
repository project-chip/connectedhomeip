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
    libmbedtls-dev \
    python-pip \
    libusb-1.0 \
    libncurses5-dev \
    libncursesw5-dev \
    flex \
    bison \
    gperf \
    zlib1g-dev \
    libglib2.0-dev \
    libpixman-1-dev \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

# Install specific release of openssl
RUN set -x \
    && cd /tmp && wget https://github.com/openssl/openssl/archive/OpenSSL_1_1_1f.zip \
    && mkdir -p /tmp/openssl && cd /tmp/openssl && unzip ../OpenSSL_1_1_1f.zip \
    && cd /tmp/openssl/openssl-OpenSSL_1_1_1f && ./config && make && make install \
    && rm -rf /tmp/OpenSSL_1_1_1f.zip \
    && : # last line

# nRF5 SDK, needed for building Nordic Platform code
RUN set -x \
    && curl -o /tmp/nRF5SDKforThreadandZigbee.zip \
    https://www.nordicsemi.com/-/media/Software-and-other-downloads/SDKs/nRF5-SDK-for-Thread/nRF5-SDK-for-Thread-and-Zigbee/nRF5SDKforThreadandZigbeev400dc7186b.zip \
    && (mkdir /var/nRF5_SDK_for_Thread_and_Zigbee \
    && cd /var/nRF5_SDK_for_Thread_and_Zigbee \
    && unzip /tmp/nRF5SDKforThreadandZigbee.zip) \
    && rm -rf /tmp/nRF5SDKforThreadandZigbee.zip \
    && : # last line

# Tools for flashing software on Nordic devices, and accessing device logs
RUN set -x \
    && (mkdir /var/nRF5_tools && cd /var/nRF5_tools \
    && curl https://www.nordicsemi.com/-/media/Software-and-other-downloads/Desktop-software/nRF-command-line-tools/sw/Versions-10-x-x/10-7-0/nRFCommandLineTools1070Linuxamd64tar.gz \
    | tar zxvf - \
    && dpkg -i JLink_Linux_*.deb \
    && dpkg -i nRF-Command-Line-Tools_*.deb \
    && tar zxvf nRF-Command-Line-Tools_*.tar.gz) \
    && rm -rf /var/nRF5_tools/*.tar.gz \
    && rm -rf /var/nRF5_tools/*.deb \
    && : # last line

# GNU ARM Embedded toolchain, cross compiler for various platform builds
RUN set -x \
    && (cd /var \
    && curl https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/9-2019q4/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2 \
    | tar jxvf -) \
    && : # last line

ENV NRF5_SDK_ROOT=/var/nRF5_SDK_for_Thread_and_Zigbee
ENV NRF5_TOOLS_ROOT=/var/nRF5_tools
ENV ARM_GCC_INSTALL_ROOT=/var/gcc-arm-none-eabi-9-2019-q4-major/bin/

# Setup the ESP-IDF
RUN set -x \
    && cd /var \
    && git clone --progress -b release/v4.0 https://github.com/espressif/esp-idf.git \
    && cd esp-idf \
    && git submodule update --init --progress \
    && IDF_TOOLS_PATH=/var/.espressif ./install.sh \
    && : # last line

ENV IDF_PATH=/var/esp-idf/
ENV IDF_TOOLS_PATH=/var/.espressif

# Setup QEMU emulator for ESP32 platform
RUN set -x \
    && cd /var \
    && git clone --progress https://github.com/espressif/qemu.git qemu_esp32 \
    && (cd /var/qemu_esp32 \
    && ./configure --target-list=xtensa-softmmu --enable-debug --enable-sanitizers --disable-strip --disable-user --disable-capstone --disable-vnc --disable-sdl --disable-gtk \
    && make -j8) \
    && : # last line

ENV QEMU_ESP32_DIR=/var/qemu_esp32
ENV QEMU_ESP32=/var/qemu_esp32/xtensa-softmmu/qemu-system-xtensa
