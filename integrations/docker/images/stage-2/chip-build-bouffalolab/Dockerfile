ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

RUN apt update -y \
    && apt install vim -fy \
    && apt clean \
    && pip3 install bflb-iot-tool==1.8.6 \
    && : # last line

COPY setup.sh /tmp

RUN cd /tmp \
    && bash setup.sh \
    && rm setup.sh \
    && : # last line

ENV BOUFFALOLAB_SDK_ROOT=/opt/bouffalolab_sdk
