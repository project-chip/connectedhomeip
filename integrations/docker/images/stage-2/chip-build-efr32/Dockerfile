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


# Clone Gecko SDK 4.3.1 (2ba59bf)
RUN git clone --depth=1 --branch=v4.3.1 https://github.com/SiliconLabs/gecko_sdk.git /tmp/gecko_sdk && \
    cd /tmp/gecko_sdk && \
    rm -rf .git \
    && : # last line

# Clone Wiseconnect SDK 3.1.0-MatterCert (f84bf66)
RUN git clone --depth=1 --branch=MatterCert https://github.com/SiliconLabs/wiseconnect.git /tmp/wifi_sdk && \
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

# GNU ARM Embedded toolchain, cross compiler for various platform builds
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    gcc-arm-none-eabi \
    binutils-arm-none-eabi \
    openjdk-17-jdk-headless \
    python3-sphinx \
    ccache \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    # Install Python Packages
    && pip3 install \
    # codegen.py build requirements
    # TODO: why are these added here instead of build-env?
    lark \
    jinja2 \
    stringcase \
    # Sphinx dependencies (for slc-cli)
    myst_parser \
    sphinx_rtd_theme \
    sphinx_tabs \
    linkify-it-py \
    && : # last line

ENV GSDK_ROOT=/opt/silabs/gecko_sdk/
ENV WISECONNECT_PATH=/opt/silabs/wifi_sdk/
ENV PATH="${PATH}:/opt/silabs/slc_cli/"

COPY --from=build /tmp/gecko_sdk /opt/silabs/gecko_sdk
COPY --from=build /tmp/wifi_sdk /opt/silabs/wifi_sdk
COPY --from=build /tmp/slc_cli /opt/silabs/slc_cli
