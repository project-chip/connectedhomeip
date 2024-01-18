ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    wget \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

# Install Sysconfig
RUN set -x \
    && wget https://dr-download.ti.com/software-development/ide-configuration-compiler-or-debugger/MD-nsUM6f7Vvb/1.16.2.3028/sysconfig-1.16.2_3028-setup.run \
    && chmod +x sysconfig-1.16.2_3028-setup.run \
    && ./sysconfig-1.16.2_3028-setup.run --mode unattended \
    && : # last line

ENV TI_SYSCONFIG_ROOT=/opt/ti/sysconfig_1.16.2
