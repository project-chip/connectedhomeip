ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

RUN set -x \
    && pip install bflb-iot-tool \
    && apt-get update \
    && apt-get install -fy \
    vim \
    && : # last line
