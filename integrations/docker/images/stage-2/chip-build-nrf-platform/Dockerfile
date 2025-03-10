ARG VERSION=1

# ==================================================
# Temporary image for SDK and dependencies download
# ==================================================

FROM ghcr.io/project-chip/chip-build:${VERSION} as build
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip
# Compatible Nordic Connect SDK revision.
ARG NCS_REVISION=v2.9.0

# Requirements to clone SDKs in temporary container
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    xz-utils \
    && : # last line

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
WORKDIR /opt/NordicSemiconductor/nRF5_tools
RUN set -x \
    && curl --location https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/desktop-software/nrf-command-line-tools/sw/versions-10-x-x/10-24-2/nrf-command-line-tools-10.24.2_linux-amd64.tar.gz \
    | tar zxvf - \
    && tar xvf JLink_Linux_V794e_x86_64.tgz \
    && rm JLink_Linux_V794e_x86_64.* \
    && curl --location https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.17.0/zephyr-sdk-0.17.0_linux-x86_64_minimal.tar.xz --output zephyr-sdk-0.17.0_linux-x86_64_minimal.tar.xz \
    && tar xvf zephyr-sdk-0.17.0_linux-x86_64_minimal.tar.xz \
    && rm -rf zephyr-sdk-0.17.0_linux-x86_64_minimal.tar.xz \
    && zephyr-sdk-0.17.0/setup.sh -t arm-zephyr-eabi \
    && : # last line

WORKDIR /opt/NordicSemiconductor/nrfconnect
RUN set -x \
    && python3 -m pip install --break-system-packages -U --no-cache-dir west==1.2.0 \
    && west init -m https://github.com/nrfconnect/sdk-nrf --mr "$NCS_REVISION" \
    && west config update.narrow true \
    && west config update.fetch smart \
    && west update -o=--depth=1 -n -f smart \
    && : # last line

# ==================================================
# nRF Connect SDK final image
# ==================================================

FROM ghcr.io/project-chip/chip-build:${VERSION}

# Tools for building, flashing and accessing device logs
RUN set -x \
    && apt-get update \
    && apt-get install --no-install-recommends -fy device-tree-compiler gcc-multilib g++-multilib \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

COPY --from=build /opt/NordicSemiconductor/nRF5_tools/ /opt/NordicSemiconductor/nRF5_tools/
COPY --from=build /opt/NordicSemiconductor/nrfconnect/ /opt/NordicSemiconductor/nrfconnect/

RUN set -x \
    # python3-yaml and python3-numpy packages conflict with nRF Python requirements
    && (apt-get remove -fy python3-yaml python3-numpy && apt-get autoremove || exit 0) \
    && python3 -m pip install --break-system-packages -U --no-cache-dir cmake==3.25.0 \
    && python3 -m pip install --break-system-packages --no-cache-dir -r /opt/NordicSemiconductor/nrfconnect/zephyr/scripts/requirements-base.txt \
    && python3 -m pip install --break-system-packages --no-cache-dir -r /opt/NordicSemiconductor/nrfconnect/nrf/scripts/requirements-build.txt \
    && python3 -m pip install --break-system-packages --no-cache-dir -r /opt/NordicSemiconductor/nrfconnect/bootloader/mcuboot/scripts/requirements.txt \
    && python3 -m pip install --break-system-packages --no-cache-dir -r /opt/NordicSemiconductor/nrfconnect/modules/lib/matter/scripts/setup/requirements.nrfconnect.txt \
    && : # last line

ENV NRF5_TOOLS_ROOT=/opt/NordicSemiconductor/nRF5_tools
ENV PATH=${NRF5_TOOLS_ROOT}/JLink_Linux_V794e_x86_64:${PATH}
ENV PATH=${NRF5_TOOLS_ROOT}/nrf-command-line-tools/bin:${PATH}
ENV LD_LIBRARY_PATH=${NRF5_TOOLS_ROOT}/JLink_Linux_V794e_x86_64:${LD_LIBRARY_PATH}
ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
ENV ZEPHYR_BASE=/opt/NordicSemiconductor/nrfconnect/zephyr
ENV ZEPHYR_SDK_INSTALL_DIR=${NRF5_TOOLS_ROOT}/zephyr-sdk-0.17.0
ENV ZEPHYR_TOOLCHAIN_VARIANT=zephyr
ENV ZEPHYR_TOOLCHAIN_PATH=${ZEPHYR_SDK_INSTALL_DIR}/arm-zephyr-eabi
