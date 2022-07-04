ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    git=1:2.25.1-1ubuntu3.4 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line