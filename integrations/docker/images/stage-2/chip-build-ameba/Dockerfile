ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

# Setup Ameba
ARG AMEBA_DIR=/opt/ameba
ARG TAG_NAME=ameba_update_2024_03_22
RUN set -x \
    && apt-get update \
    && mkdir ${AMEBA_DIR} \
    && cd ${AMEBA_DIR} \
    && git clone --progress -b cmake_build https://github.com/pankore/ambd_sdk_with_chip_non_NDA.git \
    && cd ambd_sdk_with_chip_non_NDA \
    && git checkout -f ${TAG_NAME} \
    && git submodule update --depth 1 --init --progress \
    && cd project/realtek_amebaD_va0_example/GCC-RELEASE/project_hp/toolchain \
    && cat asdk/asdk-10.3.0-linux-newlib-build-3638-x86_64.tar.bz2.part* > asdk/asdk-10.3.0-linux-newlib-build-3638-x86_64.tar.bz2 \
    && mkdir -p linux && tar -jxvf asdk/asdk-10.3.0-linux-newlib-build-3638-x86_64.tar.bz2 -C linux/ \
    && : # last line

ENV AMEBA_PATH=${AMEBA_DIR}/ambd_sdk_with_chip_non_NDA
