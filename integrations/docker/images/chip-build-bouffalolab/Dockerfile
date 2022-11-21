ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

RUN apt update -y \
 && apt clean \
 && pip3 install bflb-iot-tool

COPY setup.sh /tmp

RUN cd /tmp \
 && bash setup.sh
