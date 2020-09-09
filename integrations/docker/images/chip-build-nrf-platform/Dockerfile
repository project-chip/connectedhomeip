ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# ==================================================
# Common layers for nRF5 and nRF Connect SDK
# ==================================================

# Tools for flashing software on Nordic devices, and accessing device logs
RUN set -x \
    && (mkdir /var/nRF5_tools && cd /var/nRF5_tools \
    && curl https://www.nordicsemi.com/-/media/Software-and-other-downloads/Desktop-software/nRF-command-line-tools/sw/Versions-10-x-x/10-7-0/nRFCommandLineTools1070Linuxamd64tar.gz \
    | tar zxvf - \
    && dpkg -i --force-depends JLink_Linux_*.deb \
    && dpkg -i --force-depends nRF-Command-Line-Tools_*.deb \
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

# ==================================================
# nRF5 SDK
# ==================================================
RUN set -x \
    && curl -o /tmp/nRF5SDKforThreadandZigbee.zip \
    https://www.nordicsemi.com/-/media/Software-and-other-downloads/SDKs/nRF5-SDK-for-Thread/nRF5-SDK-for-Thread-and-Zigbee/nRF5SDKforThreadandZigbeev400dc7186b.zip \
    && (mkdir /var/nRF5_SDK_for_Thread_and_Zigbee \
    && cd /var/nRF5_SDK_for_Thread_and_Zigbee \
    && unzip /tmp/nRF5SDKforThreadandZigbee.zip) \
    && rm -rf /tmp/nRF5SDKforThreadandZigbee.zip \
    && : # last line

ENV NRF5_SDK_ROOT=/var/nRF5_SDK_for_Thread_and_Zigbee
ENV NRF5_TOOLS_ROOT=/var/nRF5_tools
ENV ARM_GCC_INSTALL_ROOT=/var/gcc-arm-none-eabi-9-2019-q4-major/bin/

# ==================================================
# nRF Connect SDK
# ==================================================
RUN set -x \
    # Device Tree Compiler 1.4.7
    && curl -o /tmp/dtc.deb http://mirrors.edge.kernel.org/ubuntu/pool/main/d/device-tree-compiler/device-tree-compiler_1.4.7-3ubuntu2_amd64.deb \
    && dpkg -i /tmp/dtc.deb \
    # nRF Connect SDK [master - Aug 27th] sources & requirements
    && (mkdir /var/ncs && cd /var/ncs \
    && pip3 install --no-cache-dir setuptools wheel cmake west \
    && west init -m https://github.com/nrfconnect/sdk-nrf --mr 83764f01f04f58c80bb1ae1d3e0a640496f79b49 \
    && west update \
    && pip3 install --no-cache-dir -r zephyr/scripts/requirements.txt \
    && pip3 install --no-cache-dir -r nrf/scripts/requirements.txt \
    && pip3 install --no-cache-dir -r bootloader/mcuboot/scripts/requirements.txt) \
    # Configuration
    && echo "source /var/ncs/zephyr/zephyr-env.sh" >> ~/.bashrc \
    # Cleanup
    && rm -rf /tmp/* \
    && : # last line

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
ENV ZEPHYR_BASE=/var/ncs/zephyr
ENV ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
ENV GNUARMEMB_TOOLCHAIN_PATH=/var/gcc-arm-none-eabi-9-2019-q4-major
