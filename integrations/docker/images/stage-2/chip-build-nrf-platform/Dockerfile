ARG VERSION=1

FROM ghcr.io/project-chip/chip-build:${VERSION} as build

# Compatible Nordic Connect SDK revision and required tools versions
ARG NCS_REVISION=v3.0.0
ARG JLINK_VERSION="JLink_Linux_V818_x86_64"
ARG WEST_VERSION=1.2.0
ARG CMAKE_VERSION=3.25.0

# Requirements to clone SDKs
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    xz-utils \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
WORKDIR /opt/NordicSemiconductor/nRF5_tools
RUN set -x \
    # Get JLink
    && curl -d "accept_license_agreement=accepted&submit=Download+software" -X POST -O "https://www.segger.com/downloads/jlink/${JLINK_VERSION}.tgz" \
    && tar xvf ${JLINK_VERSION}.tgz \
    && rm ${JLINK_VERSION}.* \
    # Get nRF Util
    && curl --location https://files.nordicsemi.com/artifactory/swtools/external/nrfutil/executables/x86_64-unknown-linux-gnu/nrfutil -o nrfutil \
    && chmod +x ./nrfutil \
    && ./nrfutil self-upgrade \
    && ./nrfutil install device \
    && ./nrfutil install sdk-manager \
    # Download nRF Connect toolchain
    && ./nrfutil sdk-manager toolchain install --ncs-version "$NCS_REVISION" --install-dir ./zephyr-sdk \
    && : # last line

WORKDIR /opt/NordicSemiconductor/nrfconnect
RUN set -x \
    && python3 -m pip install --break-system-packages -U --no-cache-dir west==${WEST_VERSION} \
    && west init -m https://github.com/nrfconnect/sdk-nrf --mr "$NCS_REVISION" \
    && west config update.narrow true \
    && west config update.fetch smart \
    && west update -o=--depth=1 -n -f smart \
    && : # last line

# Tools for building, flashing and accessing device logs
RUN set -x \
    && apt-get update \
    && apt-get install --no-install-recommends -fy device-tree-compiler gcc-multilib g++-multilib libusb-1.0-0 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

ENV NRF5_TOOLS_ROOT=/opt/NordicSemiconductor/nRF5_tools
ENV PATH=${NRF5_TOOLS_ROOT}:$PATH
ENV PATH=${NRF5_TOOLS_ROOT}/${JLINK_VERSION}:${PATH}
ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
ENV ZEPHYR_BASE=/opt/NordicSemiconductor/nrfconnect/zephyr

# Set up Zephyr SDK environment variables
RUN set -x \
    # Setup Zephyr SDK environment variables
    # Do not use the nrfconnect python environment because the python environment should be created by bootstrap.sh
    && nrfutil sdk-manager toolchain env --as-script --ncs-version "$NCS_REVISION" --install-dir ${NRF5_TOOLS_ROOT}/zephyr-sdk | grep -v "PYTHONHOME\|PYTHONPATH" | tee -a /root/.bashrc \
    # Remove redundant directory created by nrfutil
    && rm -rf ${NRF5_TOOLS_ROOT}/zephyr-sdk/downloads \
    && : # last line
