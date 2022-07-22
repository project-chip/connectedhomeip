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
    && wget https://software-dl.ti.com/ccs/esd/sysconfig/sysconfig-1.11.0_2225-setup.run \
    && chmod +x sysconfig-1.11.0_2225-setup.run \
    && ./sysconfig-1.11.0_2225-setup.run --mode unattended \
    && : # last line

ENV TI_SYSCONFIG_ROOT=/opt/ti/sysconfig_1.11.0
