ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION} as build

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    git=1:2.25.1-1ubuntu3.2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

RUN set -x \
    && git clone --depth 1 --recursive -b v4.3 https://github.com/espressif/esp-idf.git /tmp/esp-idf \
    && : # last line

# TODO: Remove this patch once https://github.com/espressif/esp-idf/pull/7632 is available
COPY 0001-esp_crt_bundle-remove-EC-ACC-certificate.patch /tmp/esp-idf/0001-esp_crt_bundle-remove-EC-ACC-certificate.patch

WORKDIR /tmp/esp-idf
RUN set -x \
    && git apply /tmp/esp-idf/0001-esp_crt_bundle-remove-EC-ACC-certificate.patch \
    && rm -f /tmp/esp-idf/0001-esp_crt_bundle-remove-EC-ACC-certificate.patch \
    && : # last line

FROM connectedhomeip/chip-build:${VERSION}

ENV IDF_PATH=/opt/espressif/esp-idf/
ENV IDF_TOOLS_PATH=/opt/espressif/tools

COPY --from=build /tmp/esp-idf /opt/espressif/esp-idf

# Setup the ESP-IDF
WORKDIR /opt/espressif/esp-idf
RUN set -x \
    && ./install.sh \
    && : # last line
