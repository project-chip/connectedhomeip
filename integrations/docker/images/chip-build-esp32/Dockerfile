ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# Setup the ESP-IDF
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y python \
    && cd /var \
    && git clone --progress -b release/v4.0 https://github.com/espressif/esp-idf.git \
    && cd esp-idf \
    && git submodule update --init --progress \
    && IDF_TOOLS_PATH=/var/.espressif ./install.sh \
    && : # last line

ENV IDF_PATH=/var/esp-idf/
ENV IDF_TOOLS_PATH=/var/.espressif
