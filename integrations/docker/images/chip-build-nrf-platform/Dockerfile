ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# Compatible Nordic Connect SDK revision.
ARG NCS_REVISION=910c9ce5ba8ea30155b50b9d12f03ece7123a0c2

# ==================================================
# nRF Connect SDK dependencies
# ==================================================

# Tools for building, flashing and accessing device logs
WORKDIR /opt/NordicSemiconductor/nRF5_tools/install
RUN set -x \
    && apt-get update \
    && apt-get install --no-install-recommends -fy device-tree-compiler \
    && curl https://www.nordicsemi.com/-/media/Software-and-other-downloads/Desktop-software/nRF-command-line-tools/sw/Versions-10-x-x/10-12-1/nRFCommandLineTools10121Linuxamd64.tar.gz \
    | tar zxvf - \
    && cd .. \
    && tar xvf install/JLink_Linux_V688a_x86_64.tgz \
    && tar xvf install/nRF-Command-Line-Tools_10_12_1.tar \
    && rm -rf ./install /var/lib/apt/lists/ \
    && : # last line

# ==================================================
# nRF Connect SDK
# ==================================================

WORKDIR /opt/NordicSemiconductor/nrfconnect
RUN set -x \
    && python3 -m pip install -U --no-cache-dir pip setuptools wheel cmake west \
    && west init -m https://github.com/nrfconnect/sdk-nrf --mr "$NCS_REVISION" \
    && west config update.narrow true \
    && west config update.fetch smart \
    && west update -o=--depth=1 \
    && python3 -m pip install --no-cache-dir -r zephyr/scripts/requirements.txt \
    && python3 -m pip install --no-cache-dir -r nrf/scripts/requirements.txt \
    && python3 -m pip install --no-cache-dir -r bootloader/mcuboot/scripts/requirements.txt \
    && : # last line

ENV NRF5_TOOLS_ROOT=/opt/NordicSemiconductor/nRF5_tools
ENV PATH=${NRF5_TOOLS_ROOT}/JLink_Linux_V688a_x86_64:${PATH}
ENV PATH=${NRF5_TOOLS_ROOT}/mergehex:${NRF5_TOOLS_ROOT}/nrfjprog:${PATH}
ENV LD_LIBRARY_PATH=${NRF5_TOOLS_ROOT}/JLink_Linux_V688a_x86_64:${LD_LIBRARY_PATH}
ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
ENV ZEPHYR_BASE=/opt/NordicSemiconductor/nrfconnect/zephyr
ENV ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
