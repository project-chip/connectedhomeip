ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION} as build
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    wget \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line
WORKDIR /opt
RUN set -x \
    && wget --quiet https://www.nxp.com/lgfiles/IMM/fsl-imx-xwayland-glibc-x86_64-imx-image-multimedia-armv8a-imx8mmevk-toolchain-5.15-kirkstone.sh \
    && chmod a+x fsl-imx-xwayland-glibc-x86_64-imx-image-multimedia-armv8a-imx8mmevk-toolchain-5.15-kirkstone.sh \
    && ./fsl-imx-xwayland-glibc-x86_64-imx-image-multimedia-armv8a-imx8mmevk-toolchain-5.15-kirkstone.sh -y \
    && rm -rf fsl-imx-xwayland-glibc-x86_64-imx-image-multimedia-armv8a-imx8mmevk-toolchain-5.15-kirkstone.sh \
    && : # last line

FROM connectedhomeip/chip-build:${VERSION}

COPY --from=build /opt/fsl-imx-xwayland /opt/fsl-imx-xwayland

ENV IMX_SDK_ROOT=/opt/fsl-imx-xwayland/5.15-kirkstone/
