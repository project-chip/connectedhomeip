ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# Setup the ESP-IDF
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y python libgcrypt20-dev \
    && mkdir -p /opt/espressif \
    && cd /opt/espressif \
    && git clone --progress -b release/v4.2 https://github.com/espressif/esp-idf.git \
    && cd esp-idf \
    && git reset --hard d9ec7df3931bea46292f015a99d19a28670bd561 \
    && git submodule update --init --progress \
    && IDF_TOOLS_PATH=/opt/espressif/tools ./install.sh \
    && : # last line

ENV IDF_PATH=/opt/espressif/esp-idf/
ENV IDF_TOOLS_PATH=/opt/espressif/tools
