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
    && west init -m https://github.com/nxp-mcuxpresso/mcux-sdk --mr "release/2.6.x_k32w0" \
    && west update \
    && chmod +x core/tools/imagetool/sign_images.sh \
    && ln -sf ../rtos core \
    && ln -sf ../middleware core \
    && cp -R examples/* core/boards && rm -rf examples \
    && : # last line

FROM ghcr.io/project-chip/chip-build:${VERSION}

COPY --from=build /opt/sdk/ /opt/sdk/

ENV NXP_K32W0_SDK_ROOT=/opt/sdk/core
