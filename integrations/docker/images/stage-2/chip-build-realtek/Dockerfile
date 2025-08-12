ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    bzip2 \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

# Setup Realtek
ARG REALTEK_DIR=/opt/realtek

RUN set -x \
    && apt-get update \
    && mkdir ${REALTEK_DIR} \
    && cd ${REALTEK_DIR} \
    ##&& git submodule update --depth 1 --init --progress \

