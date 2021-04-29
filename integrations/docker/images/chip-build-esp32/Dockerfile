ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

WORKDIR /opt/espressif/esp-idf

# Setup the ESP-IDF
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    python-is-python2=2.7.17-4 libgcrypt20-dev=1.8.5-5ubuntu1 \
    && git clone --depth=1 -b release/v4.2 https://github.com/espressif/esp-idf.git /opt/espressif/esp-idf \
    && git submodule update --init \
    && IDF_TOOLS_PATH=/opt/espressif/tools ./install.sh \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

ENV IDF_PATH=/opt/espressif/esp-idf/
ENV IDF_TOOLS_PATH=/opt/espressif/tools
