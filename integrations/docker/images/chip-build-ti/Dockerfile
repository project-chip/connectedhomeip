ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    wget \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

# Install Sysconfig
RUN set -x \
    && wget https://dr-download.ti.com/software-development/ide-configuration-compiler-or-debugger/MD-nsUM6f7Vvb/1.14.0.2667/sysconfig-1.14.0_2667-setup.run \
    && chmod +x sysconfig-1.14.0_2667-setup.run \
    && ./sysconfig-1.14.0_2667-setup.run --mode unattended \
    && : # last line

ENV TI_SYSCONFIG_ROOT=/opt/ti/sysconfig_1.14.0
