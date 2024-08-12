ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION} as build
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    unzip \
    wget \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

WORKDIR /opt/sdk

RUN set -x \
    && python3 -m pip install --break-system-packages -U --no-cache-dir west==1.2.0 \
    && : # last line

RUN set -x \
    && west init -m https://github.com/nxp-mcuxpresso/mcux-sdk --mr "MCUX_2.6.14_K32W0" \
    && west update -o=--depth=1 -n -f smart \
    && chmod +x core/tools/imagetool/sign_images.sh \
    && ln -sf ../rtos core \
    && ln -sf ../middleware core \
    && cp -R examples/* core/boards && rm -rf examples \
    && : # last line

WORKDIR /opt/k32w1_sdk

RUN set -x \
    && west init -m https://github.com/nxp-mcuxpresso/mcux-sdk --mr "MCUX_2.16.000" \
    && west update -o=--depth=1 -n -f smart \
    && : # last line

FROM ghcr.io/project-chip/chip-build:${VERSION}

COPY --from=build /opt/sdk/ /opt/sdk/
COPY --from=build /opt/k32w1_sdk/ /opt/k32w1_sdk/

ENV NXP_K32W0_SDK_ROOT=/opt/sdk/core
ENV NXP_SDK_ROOT=/opt/k32w1_sdk
