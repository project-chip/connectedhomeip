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

RUN set -x \
    && python3 -m pip install -U --no-cache-dir west==1.0.0 \
    && west init -m https://github.com/nxp-mcuxpresso/mcux-sdk --mr "MCUX_2.6.13_K32W0" \
    && west update \
    && chmod +x core/tools/imagetool/sign_images.sh \
    && ln -sf ../rtos core \
    && ln -sf ../middleware core \
    && cp -R examples/* core/boards && rm -rf examples \
    && : # last line

RUN set -x \
    && mkdir -p k32w1 \
    && wget https://cache.nxp.com/lgfiles/bsps/SDK_2_12_7_K32W148-EVK.zip \
    && unzip SDK_2_12_7_K32W148-EVK.zip -d k32w1 \
    && rm -rf SDK_2_12_7_K32W148-EVK.zip

FROM ghcr.io/project-chip/chip-build:${VERSION}

COPY --from=build /opt/sdk/ /opt/sdk/

ENV NXP_K32W0_SDK_ROOT=/opt/sdk/core
ENV NXP_K32W1_SDK_ROOT=/opt/sdk/k32w1
