ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

# GNU ARM Embedded toolchain, cross compiler for various platform builds
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    gcc-arm-none-eabi \
    binutils-arm-none-eabi \
    git-lfs \
    openjdk-17-jdk \
    python3-sphinx \
    ccache \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line


#Clone Gecko SDK 4.2.0 (ef05eb6)
RUN git clone --depth=1 --branch=v4.2.3 https://github.com/SiliconLabs/gecko_sdk.git && \
    cd gecko_sdk && \
    rm -rf .git \
    && : # last line

ENV GSDK_ROOT=/gecko_sdk/

# SLC-cli install
# TODO: figure out a way to make this a fixed version. Currently a moving target.
RUN wget https://www.silabs.com/documents/login/software/slc_cli_linux.zip && \
    unzip ./slc_cli_linux.zip -d ./ && \
    rm ./slc_cli_linux.zip

ENV PATH="${PATH}:/slc_cli/"

# Install Python Packages

# codegen.py build requirements
# TODO: why are these added here instead of build-env?
RUN pip3 install lark jinja2 stringcase

# Sphinx dependencies (for slc-cli)
RUN pip3 install myst_parser sphinx_rtd_theme sphinx_tabs linkify-it-py
