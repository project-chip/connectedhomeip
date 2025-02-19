ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION} as build
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

# Requirements to clone SDKs in temporary container
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    git \
    git-lfs \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

# Download Simplicity SDK v2024.12.1-0 (da66128)
RUN wget https://github.com/SiliconLabs/simplicity_sdk/releases/download/v2024.12.1-0/simplicity-sdk.zip -O /tmp/simplicity_sdk.zip \
    && unzip /tmp/simplicity_sdk.zip -d /tmp/simplicity_sdk \
    && rm -rf /tmp/simplicity_sdk.zip \
    # Deleting files that are not needed to save space
    && rm -rf /tmp/simplicity_sdk/protocol/flex /tmp/simplicity_sdk/protocol/z-wave /tmp/simplicity_sdk/protocol/zigbee /tmp/simplicity_sdk/protocol/wisun /tmp/simplicity_sdk/util/third_party/tensorflow_extra \
    /tmp/simplicity_sdk/util/third_party/sqlite /tmp/simplicity_sdk/util/third_party/ot-br-posix /tmp/simplicity_sdk/util/third_party/tflite-micro /tmp/simplicity_sdk/util/third_party/tflite-fatfs /tmp/simplicity_sdk/util/third_party/unity_test_framework \
    /tmp/simplicity_sdk/platform/radio/efr32_multiphy_configurator \
    && find /tmp/simplicity_sdk/protocol/bluetooth /tmp/simplicity_sdk/platform -name "*.a" -type f -delete \
    && find /tmp/simplicity_sdk/protocol/openthread -name "*efr32mg21*" -delete \
    && find /tmp/simplicity_sdk \( -name "libsl_platform_ftd_efr32mg2*" -o -name "libsl_ot_stack_mtd_efr32mg2*" \) -type f -delete \
    && find /tmp/simplicity_sdk/hardware/board/config -mindepth 1 -maxdepth 1 -type d ! \( -name '*brd4186c*' -o -name '*brd4187c*' -o -name '*brd4186a*' -o -name '*brd4187a*' -o -name '*brd2601b*' -o -name '*brd2703a*' -o -name '*brd2704a*' -o -name '*component*' \
    -o -name '*brd4316a*' -o -name '*brd4317a*' -o -name '*brd4318a*' -o -name '*brd4319a*' -o -name '*brd4116a*' -o -name '*brd4117a*' -o -name '*brd4118a*' -o -name '*brd2608a*' -o -name '*brd4350a*' -o -name '*brd4351a*' -o -name '*brd2709a*' \) -exec rm -rf {} + \
    && find /tmp/simplicity_sdk/platform/Device/SiliconLabs  -mindepth 1 -maxdepth 1 -type d ! \( -name 'EFR32MG24' -o -name 'EFR32MG26' -o -name 'MGM24' -o -name 'MGM26' \) -exec rm -rf {} + \
    && : # last line

# Clone WiSeConnect Wi-Fi and Bluetooth Software 2.11.2 (3dbc243)
RUN git clone --depth=1 --single-branch --branch=2.11.2 https://github.com/SiliconLabs/wiseconnect-wifi-bt-sdk.git /tmp/wiseconnect-wifi-bt-sdk && \
    cd /tmp/wiseconnect-wifi-bt-sdk && \
    rm -rf .git examples \
    && : # last line

# Clone WiSeConnect SDK v3.4.1 (f675628)
RUN git clone --depth=1 --single-branch --branch=v3.4.1 https://github.com/SiliconLabs/wiseconnect.git /tmp/wifi_sdk && \
    cd /tmp/wifi_sdk && \
    rm -rf .git examples components/device/stm32 \
    && : # last line

# SLC-cli install
# TODO: figure out a way to make this a fixed version. Currently a moving target.
RUN wget https://www.silabs.com/documents/login/software/slc_cli_linux.zip && \
    unzip ./slc_cli_linux.zip -d /tmp && \
    rm ./slc_cli_linux.zip \
    && : # last line

# Final SDK container for compiling using Silabs SDK
FROM ghcr.io/project-chip/chip-build:${VERSION}

ADD requirements.txt /tmp/requirements.txt

# GNU ARM Embedded toolchain, cross compiler for various platform builds
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    gcc-arm-none-eabi \
    binutils-arm-none-eabi \
    openjdk-17-jdk-headless \
    ccache \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    # Install Python Packages
    && pip3 install --break-system-packages -r /tmp/requirements.txt \
    && rm /tmp/requirements.txt \
    && : # last line

# Keep GSDK_ROOT name until rename transition to SISDK is completed
ENV GSDK_ROOT=/opt/silabs/simplicity_sdk/
ENV SISDK_ROOT=/opt/silabs/simplicity_sdk/
ENV WISECONNECT_SDK_ROOT=/opt/silabs/wiseconnect-wifi-bt-sdk/
ENV WIFI_SDK_ROOT=/opt/silabs/wifi_sdk/
ENV PATH="${PATH}:/opt/silabs/slc_cli/"

COPY --from=build /tmp/simplicity_sdk /opt/silabs/simplicity_sdk
COPY --from=build /tmp/wiseconnect-wifi-bt-sdk/ /opt/silabs/wiseconnect-wifi-bt-sdk/
COPY --from=build /tmp/wifi_sdk /opt/silabs/wifi_sdk
COPY --from=build /tmp/slc_cli /opt/silabs/slc_cli
