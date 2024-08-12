ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION} as build
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    wget \
    xz-utils \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line
WORKDIR /opt
RUN set -x \
    && wget --quiet https://www.nxp.com/lgfiles/IMM/6.1-langdale-imx8mm-matter-toolchain.sh \
    && chmod a+x 6.1-langdale-imx8mm-matter-toolchain.sh \
    && ./6.1-langdale-imx8mm-matter-toolchain.sh -y \
    && rm -rf 6.1-langdale-imx8mm-matter-toolchain.sh \
    && : # last line
RUN set -x \
    && cd /opt/fsl-imx-xwayland \
    # && find . -iname debug -exec rm -rf "{}" + || true  \
    && find . -iname ".debug" -exec rm -rf "{}" + || true \
    && : # last line

FROM ghcr.io/project-chip/chip-build:${VERSION}


COPY --from=build /opt/fsl-imx-xwayland /opt/fsl-imx-xwayland

ENV IMX_SDK_ROOT=/opt/fsl-imx-xwayland/6.1-langdale/
