ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION} AS build
LABEL org.opencontainers.image.source=https://github.com/project-chip/connectedhomeip

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    xz-utils \
    && : # last line

WORKDIR /opt/nxp/

ARG NXP_SDK_EXAMPLE_PATH=nxp_matter_support/github_sdk/sdk_next/repo/mcuxsdk/examples

RUN set -x \
    && git clone --branch release/v1.4.0.1 https://github.com/NXP/nxp_matter_support.git \
    && pip3 install --break-system-packages -U --no-cache-dir west \
    && ./nxp_matter_support/scripts/update_nxp_sdk.py \
    && cd $NXP_SDK_EXAMPLE_PATH \
    && : # last line

FROM ghcr.io/project-chip/chip-build:${VERSION}

COPY --from=build /opt/nxp/ /opt/nxp/

ENV NXP_SDK_PATH=/opt/nxp
ENV NXP_UPDATE_SDK_SCRIPT_DOCKER=/opt/nxp/nxp_matter_support/scripts/update_nxp_sdk.py
