ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# Setup the ESP-IDF
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y python \
    && mkdir -p /opt/expressif \
    && cd /opt/expressif \
    && git clone --progress -b v4.1 https://github.com/espressif/esp-idf.git \
    && cd esp-idf \
    && git submodule update --init --progress \
    && IDF_TOOLS_PATH=/opt/expressif/tools ./install.sh \
    && : # last line

ENV IDF_PATH=/opt/expressif/esp-idf/
ENV IDF_TOOLS_PATH=/opt/expressif/tools
