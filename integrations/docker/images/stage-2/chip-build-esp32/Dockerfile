ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION} as build
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    git \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

RUN set -x \
    && git clone --recursive -b v5.1.2 --depth 1 --shallow-submodule https://github.com/espressif/esp-idf.git /tmp/esp-idf \
    && : # last line

FROM ghcr.io/project-chip/chip-build:${VERSION}

ENV IDF_PATH=/opt/espressif/esp-idf/
ENV IDF_TOOLS_PATH=/opt/espressif/tools

COPY --from=build /tmp/esp-idf /opt/espressif/esp-idf

# Setup the ESP-IDF
WORKDIR /opt/espressif/esp-idf
RUN set -x \
    && ./install.sh \
    && : # last line
