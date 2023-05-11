ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# Setup ASR
ARG ASR_DIR=/opt/asr
RUN set -x \
    && apt-get update \
    && apt-get install -y git git-lfs \
    && mkdir ${ASR_DIR} \
    && cd ${ASR_DIR} \
    && git clone --depth=1 https://github.com/asriot/Tools.git \
    && cd Tools \
    && git lfs pull \
    && cd toolchain \
    && tar -jxvf gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2 \
    && mkdir -p asr_riscv_gnu_toolchain_10.2_ubuntu-16.04 \
    && tar -jxvf asr_riscv_gnu_toolchain_10.2_ubuntu-16.04.tar.bz2 -C asr_riscv_gnu_toolchain_10.2_ubuntu-16.04/ \
    && : # last line

ENV ASR_TOOLCHAIN_PATH=${ASR_DIR}/Tools/toolchain/gcc-arm-none-eabi-9-2019-q4-major/bin/
