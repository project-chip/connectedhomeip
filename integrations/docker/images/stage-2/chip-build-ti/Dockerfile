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

# Install Sysconfig versions for CC32XX (1.18.1) and other device families (1.22.0)
RUN set -x \
    && wget https://dr-download.ti.com/software-development/ide-configuration-compiler-or-debugger/MD-nsUM6f7Vvb/1.18.1.3343/sysconfig-1.18.1_3343-setup.run \
    && chmod +x sysconfig-1.18.1_3343-setup.run \
    && ./sysconfig-1.18.1_3343-setup.run --mode unattended \
    && wget https://dr-download.ti.com/software-development/ide-configuration-compiler-or-debugger/MD-nsUM6f7Vvb/1.22.0.3893/sysconfig-1.22.0_3893-setup.run \
    && chmod +x sysconfig-1.22.0_3893-setup.run \
    && ./sysconfig-1.22.0_3893-setup.run --mode unattended \
    && rm sysconfig-1.18.1_3343-setup.run sysconfig-1.22.0_3893-setup.run \
    && : # last line

ENV TI_SYSCONFIG_ROOT=/opt/ti/sysconfig_1.22.0
ENV TI_SYSCONFIG_ROOT_CC32XX=/opt/ti/sysconfig_1.18.1
