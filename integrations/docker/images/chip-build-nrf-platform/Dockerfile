ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION} as build

# Compatible Nordic Connect SDK revision.
ARG NCS_REVISION=v2.0.2

RUN set -x \
    && apt-get update \
    && apt-get install --no-install-recommends -fy \
    curl \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

# ==================================================
# nRF Connect SDK dependencies
# ==================================================

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
RUN set -x \
    && mkdir -p /opt/NordicSemiconductor/nRF5_tools/ \
    && curl --location https://www.nordicsemi.com/-/media/Software-and-other-downloads/Desktop-software/nRF-command-line-tools/sw/Versions-10-x-x/10-12-1/nRFCommandLineTools10121Linuxamd64.tar.gz \
    | tar zxvf - \
    && tar xvf JLink_Linux_V688a_x86_64.tgz -C /opt/NordicSemiconductor/nRF5_tools/ \
    && tar xvf nRF-Command-Line-Tools_10_12_1.tar -C /opt/NordicSemiconductor/nRF5_tools/ \
    && : # last line

WORKDIR /opt/NordicSemiconductor/nrfconnect
RUN set -x \
    && python3 -m pip install -U --no-cache-dir \
    west==0.13.1 \
    && west init -m https://github.com/nrfconnect/sdk-nrf \
    && git -C nrf fetch origin "$NCS_REVISION" \
    && git -C nrf checkout FETCH_HEAD \
    && west config update.narrow true \
    && west config update.fetch smart \
    && west update -o=--depth=1 -n -f smart \
    && : # last line

FROM connectedhomeip/chip-build:${VERSION}

# Tools for building, flashing and accessing device logs
RUN set -x \
    && apt-get update \
    && apt-get install --no-install-recommends -fy \
    device-tree-compiler=1.5.1-1 \
    ccache=3.7.7-1 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

COPY --from=build /opt/NordicSemiconductor/nRF5_tools/ /opt/NordicSemiconductor/nRF5_tools/
COPY --from=build /opt/NordicSemiconductor/nrfconnect/ /opt/NordicSemiconductor/nrfconnect/

# ==================================================
# nRF Connect SDK
# ==================================================

RUN set -x \
    # python3-yaml package conflicts with nRF Python requirements
    && (apt-get remove -fy python3-yaml && apt-get autoremove || exit 0) \
    && python3 -m pip install -U --no-cache-dir cmake==3.22.5 \
    && python3 -m pip install --no-cache-dir -r /opt/NordicSemiconductor/nrfconnect/zephyr/scripts/requirements.txt \
    && python3 -m pip install --no-cache-dir -r /opt/NordicSemiconductor/nrfconnect/nrf/scripts/requirements.txt \
    && python3 -m pip install --no-cache-dir -r /opt/NordicSemiconductor/nrfconnect/bootloader/mcuboot/scripts/requirements.txt \
    && : # last line

ENV NRF5_TOOLS_ROOT=/opt/NordicSemiconductor/nRF5_tools
ENV PATH=${NRF5_TOOLS_ROOT}/JLink_Linux_V688a_x86_64:${PATH}
ENV PATH=${NRF5_TOOLS_ROOT}/mergehex:${NRF5_TOOLS_ROOT}/nrfjprog:${PATH}
ENV LD_LIBRARY_PATH=${NRF5_TOOLS_ROOT}/JLink_Linux_V688a_x86_64:${LD_LIBRARY_PATH}
ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
ENV ZEPHYR_BASE=/opt/NordicSemiconductor/nrfconnect/zephyr
ENV ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
