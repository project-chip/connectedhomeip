ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION} as build
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    wget=1.20.3-1ubuntu2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line
WORKDIR /opt
RUN set -x \
    && wget --quiet -O fsl-l5.10.52-2.1.0-sdk.tar.gz https://www.nxp.com/lgfiles/IMM/fsl-l5.10.52-2.1.0-sdk.tar.gz \
    && tar zxvf fsl-l5.10.52-2.1.0-sdk.tar.gz \
    && ./fsl-imx-xwayland-glibc-x86_64-imx-image-core-cortexa53-crypto-imx8mmevk-toolchain-5.10-hardknott.sh -y \
    && : # last line

FROM connectedhomeip/chip-build:${VERSION}

COPY --from=build /opt/fsl-imx-xwayland /opt/fsl-imx-xwayland

ENV IMX_SDK_ROOT=/opt/fsl-imx-xwayland/5.10-hardknott/
