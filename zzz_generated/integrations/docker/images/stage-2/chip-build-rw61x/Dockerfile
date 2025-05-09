ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION} as build
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    wget unzip \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

WORKDIR /opt/sdk
# Setup RW61x SDK
RUN set -x \
    && mkdir -p rw61x \
    && wget https://www.nxp.com/lgfiles/bsps/SDK_2_13_3_RD-RW612-BGA.zip \
    && unzip SDK_2_13_3_RD-RW612-BGA.zip -d rw61x \
    && rm -rf SDK_2_13_3_RD-RW612-BGA.zip \
    && : # last line

FROM ghcr.io/project-chip/chip-build:${VERSION}

COPY --from=build /opt/sdk/ /opt/sdk/

ENV NXP_SDK_ROOT=/opt/sdk/rw61x
