ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION} AS build
LABEL org.opencontainers.image.source=https://github.com/project-chip/connectedhomeip

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    xz-utils \
    && : # last line

WORKDIR /opt/nxp/

RUN set -x \
    && git clone https://github.com/NXP/nxp_matter_support.git \
    && cd nxp_matter_support \
    # Checkout commit aligned with updated west manifest using MCUX SDK 25.06.00,
    # including patches for Matter support in specific components
    && git checkout deb06197654ed2e4a688be05467a0954b91fd61d \
    && pip3 install --break-system-packages -U --no-cache-dir west \
    && ./scripts/update_nxp_sdk.py --platform common \
    && : # last line

FROM ghcr.io/project-chip/chip-build:${VERSION}

COPY --from=build /opt/nxp/ /opt/nxp/

ENV NXP_SDK_PATH=/opt/nxp
ENV NXP_UPDATE_SDK_SCRIPT_DOCKER=/opt/nxp/nxp_matter_support/scripts/update_nxp_sdk.py
