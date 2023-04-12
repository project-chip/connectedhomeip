ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

RUN apt update -y \
 && apt install vim -fy \
 && apt clean \
 && pip3 install bflb-iot-tool \
 && : # last line

COPY setup.sh /tmp

RUN cd /tmp \
 && bash setup.sh \
 && rm setup.sh \
 && : # last line

ENV BOUFFALOLAB_SDK_ROOT=/opt/bouffalolab_sdk
