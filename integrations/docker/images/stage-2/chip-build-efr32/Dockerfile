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


# Clone Gecko SDK 4.4.1 (911f6cd)
RUN wget https://github.com/SiliconLabs/gecko_sdk/releases/download/v4.4.1/gecko-sdk.zip -O /tmp/gecko_sdk.zip \
    && unzip /tmp/gecko_sdk.zip -d /tmp/gecko_sdk \
    && rm -rf /tmp/gecko_sdk.zip \
    # Deleting files that are not needed to save space
    && rm -rf /tmp/gecko_sdk/protocol/flex /tmp/gecko_sdk/protocol/z-wave /tmp/gecko_sdk/protocol/zigbee /tmp/gecko_sdk/protocol/wisun \
    && find /tmp/gecko_sdk/protocol/bluetooth /tmp/gecko_sdk/platform -name "*.a" -type f -delete \
    && find /tmp/gecko_sdk/protocol/openthread -name "*efr32mg21*" -delete \
    && find /tmp/gecko_sdk/protocol/openthread -name "*efr32mg13*" -delete \
    && : # last line

# Clone WiSeConnect Wi-Fi and Bluetooth Software 2.8.2 (4fa5c5f)
RUN git clone --depth=1 --single-branch --branch=2.8.2 https://github.com/SiliconLabs/wiseconnect-wifi-bt-sdk.git /tmp/wiseconnect-wifi-bt-sdk && \
    cd /tmp/wiseconnect-wifi-bt-sdk && \
    rm -rf .git \
    && : # last line

# Clone WiSeConnect SDK 3.1.3 (00dd57a)
RUN git clone --depth=1 --single-branch --branch=v3.1.3 https://github.com/SiliconLabs/wiseconnect.git /tmp/wifi_sdk && \
    cd /tmp/wifi_sdk && \
    rm -rf .git \
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
    && pip3 install -r /tmp/requirements.txt \
    && rm /tmp/requirements.txt \
    && : # last line

ENV GSDK_ROOT=/opt/silabs/gecko_sdk/
ENV WISECONNECT_SDK_ROOT=/opt/silabs/wiseconnect-wifi-bt-sdk/
ENV WIFI_SDK_ROOT=/opt/silabs/wifi_sdk/
ENV PATH="${PATH}:/opt/silabs/slc_cli/"

COPY --from=build /tmp/gecko_sdk /opt/silabs/gecko_sdk
COPY --from=build /tmp/wiseconnect-wifi-bt-sdk/ /opt/silabs/wiseconnect-wifi-bt-sdk/
COPY --from=build /tmp/wifi_sdk /opt/silabs/wifi_sdk
COPY --from=build /tmp/slc_cli /opt/silabs/slc_cli
