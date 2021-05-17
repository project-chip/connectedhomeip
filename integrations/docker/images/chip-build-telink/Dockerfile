ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

RUN apt update

# Telink Toolchain
ARG TELINK_DIR=/opt/telink
RUN dpkg --add-architecture i386
RUN apt-get update
RUN apt-get install -y libc6:i386 libncurses5:i386 libstdc++6:i386
RUN mkdir ${TELINK_DIR}
RUN cd ${TELINK_DIR} && wget http://wiki.telink-semi.cn/tools_and_sdk/Tools/IDE/telink_riscv_linux_toolchain.zip
RUN unzip ${TELINK_DIR}/telink_riscv_linux_toolchain.zip -d ${TELINK_DIR}
RUN rm ${TELINK_DIR}/telink_riscv_linux_toolchain.zip

# Setup Zephyr dependencies
ENV DEBIAN_FRONTEND=noninteractive
RUN apt install -y git
RUN apt install -y gperf
RUN apt install -y ccache
RUN apt install -y dfu-util
RUN apt install -y device-tree-compiler
RUN apt install -y wget
RUN apt install -y xz-utils
RUN apt install -y gcc-multilib
RUN apt install -y g++-multilib
RUN apt install -y libsdl2-dev

RUN pip3 install --user -U west
RUN pip3 install pyelftools

# Setup Zephyr
ARG ZEPHYR_PROJECT_DIR=/opt/zephyrproject
ENV PATH="/root/.local/bin:${PATH}"
RUN west init ${ZEPHYR_PROJECT_DIR}
RUN cd ${ZEPHYR_PROJECT_DIR} && west update && west zephyr-export
RUN python3 -m pip install -U pip
RUN pip3 install --user -r ${ZEPHYR_PROJECT_DIR}/zephyr/scripts/requirements.txt

# Workaround below is require due to Telink platform
# is not included into official Zephyr repo yet.
# Below steps will be removed in future.

# Remove original Zephyr repository.
RUN rm -rf ${ZEPHYR_PROJECT_DIR}/zephyr

# Clone forked Zephyr repository
RUN git clone --branch telink_b91_tlsr9518adk80d_internal https://github.com/rikorsev/zephyr ${ZEPHYR_PROJECT_DIR}/zephyr

# Clone Telink HAL repository
RUN git clone https://github.com/yurvyn/hal_telink ${ZEPHYR_PROJECT_DIR}/modules/hal/telink

# Update once again, ignore an error
RUN cd ${ZEPHYR_PROJECT_DIR} && west update || true

ENV ZEPHYR_BASE=${ZEPHYR_PROJECT_DIR}/zephyr
ENV TELINK_TOOLCHAIN_PATH=${TELINK_DIR}/telink_riscv_linux_toolchain/nds32le-elf-mculib-v5f/bin
