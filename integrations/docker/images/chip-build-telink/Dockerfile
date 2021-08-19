ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# Setup toolchain
ENV ZEPHYR_TOOLCHAIN_VARIANT=zephyr
ENV ZEPHYR_SDK_INSTALL_DIR=/opt/telink/zephyr-sdk-0.13.0
RUN wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.13.0/zephyr-toolchain-riscv64-0.13.0-linux-x86_64-setup.run\
    && chmod +x zephyr-toolchain-riscv64-0.13.0-linux-x86_64-setup.run \
    && ./zephyr-toolchain-riscv64-0.13.0-linux-x86_64-setup.run -- -d ${ZEPHYR_SDK_INSTALL_DIR} \
    && rm zephyr-toolchain-riscv64-0.13.0-linux-x86_64-setup.run

# Setup Zephyr
ARG ZEPHYR_PROJECT_DIR=/opt/telink/zephyrproject
ENV PATH="/root/.local/bin:${PATH}"
RUN apt update \
    && DEBIAN_FRONTEND=noninteractive apt install -y \
    ccache \
    dfu-util \
    device-tree-compiler \
    gcc-multilib \
    g++-multilib \
    libsdl2-dev \
    && pip3 install --user -U west \
    && pip3 install pyelftools \
    # @todo: Add --mr=zephyr-v2.7.0 as soon as this version would be released     
    && west init ${ZEPHYR_PROJECT_DIR} \
    && cd ${ZEPHYR_PROJECT_DIR} && west update && west zephyr-export \
    && python3 -m pip install -U pip \
    && pip3 install --user -r ${ZEPHYR_PROJECT_DIR}/zephyr/scripts/requirements.txt

ENV ZEPHYR_BASE=${ZEPHYR_PROJECT_DIR}/zephyr