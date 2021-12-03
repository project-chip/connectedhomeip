ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# Setup Ameba
ARG AMEBA_DIR=/opt/ameba
RUN set -x \
    && apt-get update \
    && mkdir ${AMEBA_DIR} \
    && cd ${AMEBA_DIR} \
    && git clone --progress -b cmake_build https://github.com/pankore/ambd_sdk_with_chip_non_NDA.git \
    && cd ambd_sdk_with_chip_non_NDA \
    && git reset --hard 2e85ca6 \
    && git submodule update --depth 1 --init --progress \
    && cd project/realtek_amebaD_va0_example/GCC-RELEASE/project_hp/toolchain \
    && cat asdk/asdk-9.3.0-linux-newlib-build-3483-x86_64.tar.bz2.part* > asdk/asdk-9.3.0-linux-newlib-build-3483-x86_64.tar.bz2 \
    && mkdir -p linux && tar -jxvf asdk/asdk-9.3.0-linux-newlib-build-3483-x86_64.tar.bz2 -C linux/ \
    && : # last line

ENV AMEBA_PATH=${AMEBA_DIR}/ambd_sdk_with_chip_non_NDA
