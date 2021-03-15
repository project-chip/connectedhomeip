ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# Compatible Nordic Connect SDK revision.
ARG NCS_REVISION=v1.5.0

# ==================================================
# nRF Connect SDK dependencies
# ==================================================

# Tools for building, flashing and accessing device logs
RUN set -x \
    && apt-get update \
    && apt-get install --no-install-recommends -fy device-tree-compiler \
    && (mkdir -p /opt/NordicSemiconductor/nRF5_tools/install && cd /opt/NordicSemiconductor/nRF5_tools/install \
    && curl https://www.nordicsemi.com/-/media/Software-and-other-downloads/Desktop-software/nRF-command-line-tools/sw/Versions-10-x-x/10-12-1/nRFCommandLineTools10121Linuxamd64.tar.gz \
    | tar zxvf - \
    && cd .. \
    && tar xvf install/JLink_Linux_V688a_x86_64.tgz \
    && tar xvf install/nRF-Command-Line-Tools_10_12_1.tar \
    && rm -rf ./install /var/lib/apt/lists/) \
    && : # last line

ENV NRF5_TOOLS_ROOT=/opt/NordicSemiconductor/nRF5_tools
ENV PATH=${NRF5_TOOLS_ROOT}/JLink_Linux_V688a_x86_64:${PATH}
ENV PATH=${NRF5_TOOLS_ROOT}/mergehex:${NRF5_TOOLS_ROOT}/nrfjprog:${PATH}
ENV LD_LIBRARY_PATH=${NRF5_TOOLS_ROOT}/JLink_Linux_V688a_x86_64:${LD_LIBRARY_PATH}

# GNU ARM Embedded toolchain, cross compiler for various platform builds
RUN set -x \
    && (mkdir -p /opt/ARM-software && cd /opt/ARM-software \
    && curl https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/9-2019q4/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2 \
    | tar jxvf -) \
    && : # last line

ENV ARM_GCC_INSTALL_ROOT=/opt/ARM-software/gcc-arm-none-eabi-9-2019-q4-major/bin/

# ==================================================
# nRF Connect SDK
# ==================================================
RUN set -x \
    && (mkdir -p /opt/NordicSemiconductor/nrfconnect && cd /opt/NordicSemiconductor/nrfconnect \
    && python3 -m pip install -U --no-cache-dir pip setuptools wheel cmake west \
    && west init -m https://github.com/nrfconnect/sdk-nrf --mr $NCS_REVISION \
    && west update \
    && python3 -m pip install --no-cache-dir -r zephyr/scripts/requirements.txt \
    && python3 -m pip install --no-cache-dir -r nrf/scripts/requirements.txt \
    && python3 -m pip install --no-cache-dir -r bootloader/mcuboot/scripts/requirements.txt) \
    && echo "source /opt/NordicSemiconductor/nrfconnect/zephyr/zephyr-env.sh" >> ~/.bashrc \
    && : # last line

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
ENV ZEPHYR_BASE=/opt/NordicSemiconductor/nrfconnect/zephyr
ENV ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
ENV GNUARMEMB_TOOLCHAIN_PATH=/opt/ARM-software/gcc-arm-none-eabi-9-2019-q4-major
