ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

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
