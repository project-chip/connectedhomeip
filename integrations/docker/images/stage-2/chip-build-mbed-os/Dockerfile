ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION} as build
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    wget \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
# ------------------------------------------------------------------------------
# # Install openocd
# RUN set -x \
#     && mkdir -p /opt/openocd \
#     && wget -O - --progress=dot:giga https://github.com/cypresssemiconductorco/openocd/releases/download/release-v4.2.0/openocd-4.2.0.1430-linux.tar.gz | tar --strip-components=1 -xz -C /opt/openocd \
#     && : # last line

# FROM ghcr.io/project-chip/chip-build:${VERSION}

# COPY --from=build /opt/openocd/ /opt/openocd/

# ------------------------------------------------------------------------------
# Install Python modules
RUN set -x \
    && pip3 install --break-system-packages --no-cache-dir -U mbed-cli==1.10.5 mbed-tools==7.44.0 \
    && : # last line

# ------------------------------------------------------------------------------
# Configure mbed build system
RUN set -x \
    && mbed config -G GCC_ARM_PATH /opt/mbed-os-toolchain/gcc-arm-none-eabi-9-2019-q4-major/bin/ \
    && mbed toolchain -G -s GCC_ARM \
    && : # last line

# ------------------------------------------------------------------------------
# Configure environment variables
ENV OPENOCD_PATH=/opt/openocd/

ENV PATH="${PATH}:${OPENOCD_PATH}/bin"
