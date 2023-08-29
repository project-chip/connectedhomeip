ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION} as build
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    wget \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

# Setup toolchain
RUN set -x \
    && mkdir /opt/telink \
    && cd /opt/telink \
    && wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.1/zephyr-sdk-0.16.1_linux-x86_64_minimal.tar.xz \
    && wget -O - https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.1/sha256.sum | shasum --check --ignore-missing \
    && tar xvf zephyr-sdk-0.16.1_linux-x86_64_minimal.tar.xz \
    && rm -rf zephyr-sdk-0.16.1_linux-x86_64_minimal.tar.xz \
    && cd zephyr-sdk-0.16.1 \
    && ./setup.sh -t riscv64-zephyr-elf -h -c \
    && cd / \
    && : # last line

# Setup Zephyr
ARG ZEPHYR_REVISION=333abf31311e73def9db739026d217fca547a01f
WORKDIR /opt/telink/zephyrproject
RUN set -x \
    && python3 -m pip install -U --no-cache-dir \
    west==1.0.0 \
    imgtool==1.7.0 \
    && git clone https://github.com/telink-semi/zephyr \
    && cd zephyr \
    && git reset ${ZEPHYR_REVISION} --hard \
    && west init -l \
    && cd .. \
    && west update -o=--depth=1 -n -f smart \
    && west blobs fetch hal_telink \
    && west zephyr-export \
    && : # last line

FROM ghcr.io/project-chip/chip-build:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

COPY --from=build /opt/telink/zephyr-sdk-0.16.1/ /opt/telink/zephyr-sdk-0.16.1/
COPY --from=build /opt/telink/zephyrproject/ /opt/telink/zephyrproject/

ENV ZEPHYR_TOOLCHAIN_VARIANT=zephyr
ENV ZEPHYR_SDK_INSTALL_DIR=/opt/telink/zephyr-sdk-0.16.1

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    ccache \
    dfu-util=0.9-1 \
    device-tree-compiler \
    gcc-multilib \
    g++-multilib \
    libsdl2-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && python3 -m pip install -U --no-cache-dir \
    pyelftools==0.27 \
    && pip3 install --no-cache-dir --user -r /opt/telink/zephyrproject/zephyr/scripts/requirements.txt \
    && : # last line

# Setup Telink tools required for "west flash"
ARG TELINK_TOOLS_BASE=/opt/telink/tools
RUN wget http://wiki.telink-semi.cn/tools_and_sdk/Tools/IDE/telink_riscv_ice_flash_tool.zip -O /opt/telink/tools.zip \
    && unzip /opt/telink/tools.zip -d ${TELINK_TOOLS_BASE} \
    && rm /opt/telink/tools.zip \
    && mv ${TELINK_TOOLS_BASE}/telink_riscv_linux_toolchain/* ${TELINK_TOOLS_BASE} \
    && rm -rf ${TELINK_TOOLS_BASE}/telink_riscv_linux_toolchain \
    && chmod +x ${TELINK_TOOLS_BASE}/flash/bin/SPI_burn \
    && chmod +x ${TELINK_TOOLS_BASE}/ice/ICEman \
    && : # last line

# Add path to Telink tools
ENV PATH="${TELINK_TOOLS_BASE}/flash/bin:${PATH}"
ENV PATH="${TELINK_TOOLS_BASE}/ice:${PATH}"

ARG ZEPHYR_PROJECT_DIR=/opt/telink/zephyrproject
ENV TELINK_ZEPHYR_BASE=${ZEPHYR_PROJECT_DIR}/zephyr
ENV ZEPHYR_BASE=${ZEPHYR_PROJECT_DIR}/zephyr
