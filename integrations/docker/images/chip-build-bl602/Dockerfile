ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

RUN set -x \
    && pip install bflb-iot-tool \
    && apt-get update \
    && apt-get install -fy \
    vim \
    && : # last line

ENV CHIP_DIR=/opt/chip
ENV BL_FLASH_TOOL=${CHIP_DIR}/third_party/bouffalolab/bl602_sdk/repo/tools/flash_tool/chips/bl602

RUN set -x \
    && git clone https://github.com/project-chip/connectedhomeip.git ${CHIP_DIR} \
    && : # last line

WORKDIR ${CHIP_DIR}
RUN set -x \
    && ./scripts/checkout_submodules.py --shallow --platform bl602 \
    && /bin/bash scripts/activate.sh \
    && : # last line
