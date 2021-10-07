ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# Setup Ameba
ARG AMEBA_DIR=/opt/ameba
RUN set -x \
    && apt-get update \
    && apt-get -y install python2 \
    && mkdir ${AMEBA_DIR} \
    && cd ${AMEBA_DIR} \
    && git clone --progress -b cmake_build https://github.com/pankore/ambd_sdk_with_chip_non_NDA.git \
    && cd ambd_sdk_with_chip_non_NDA \
    && git pull \
    && git submodule update --init --progress \
    && : # last line

ENV AMEBA_BUILD=${AMEBA_DIR}/ambd_sdk_with_chip_non_NDA/project/realtek_amebaD_va0_example/GCC-RELEASE
ENV AMEBA_MATTER=/root/chip
