ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# Telink Toolchain
ARG TELINK_DIR=/opt/telink
RUN dpkg --add-architecture i386 \
    && apt-get update \
    && apt-get install -y \
    libc6:i386 \
    libncurses5:i386 \
    libstdc++6:i386 \
    && mkdir ${TELINK_DIR} \
    && cd ${TELINK_DIR} \
    && wget http://wiki.telink-semi.cn/tools_and_sdk/Tools/IDE/telink_riscv_linux_toolchain.zip \
    && unzip ${TELINK_DIR}/telink_riscv_linux_toolchain.zip -d ${TELINK_DIR} \
    && rm ${TELINK_DIR}/telink_riscv_linux_toolchain.zip

# Setup Zephyr
ARG ZEPHYR_PROJECT_DIR=/opt/zephyrproject
ENV PATH="/root/.local/bin:${PATH}"
RUN DEBIAN_FRONTEND=noninteractive apt install -y \
    git \
    gperf \
    ccache \
    dfu-util \
    device-tree-compiler \
    wget \
    xz-utils \
    gcc-multilib \
    g++-multilib \
    libsdl2-dev \
    && pip3 install --user -U west \
    && pip3 install pyelftools \
    && west init --mr=zephyr-v2.5.0 ${ZEPHYR_PROJECT_DIR} \
    && cd ${ZEPHYR_PROJECT_DIR} && west update && west zephyr-export \
    && python3 -m pip install -U pip \
    && pip3 install --user -r ${ZEPHYR_PROJECT_DIR}/zephyr/scripts/requirements.txt

# Workaround below is require due to Telink platform
# is not included into official Zephyr repo yet.
# Below step will be removed in future.
RUN rm -rf ${ZEPHYR_PROJECT_DIR}/zephyr \
    && git clone --branch telink_b91_tlsr9518adk80d_internal https://github.com/rikorsev/zephyr ${ZEPHYR_PROJECT_DIR}/zephyr \
    && git clone https://github.com/rikorsev/telink_hal_zephyr ${ZEPHYR_PROJECT_DIR}/modules/hal/telink \
    && cd ${ZEPHYR_PROJECT_DIR} && west update || true

ENV ZEPHYR_BASE=${ZEPHYR_PROJECT_DIR}/zephyr
ENV TELINK_TOOLCHAIN_PATH=${TELINK_DIR}/telink_riscv_linux_toolchain/nds32le-elf-mculib-v5f/bin

# Required for west flash command
ENV TELINK_TOOLCHAIN_BASE=${TELINK_DIR}/telink_riscv_linux_toolchain
