ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION} as build

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    wget=1.20.3-1ubuntu2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

SHELL ["/bin/bash", "-o", "pipefail", "-c"]

# ------------------------------------------------------------------------------
# Install FVP Corstone 300
RUN set -x \
    && wget -q https://developer.arm.com/-/media/Arm%20Developer%20Community/Downloads/OSS/FVP/Corstone-300/FVP_Corstone_SSE-300_11.16_26.tgz \
    && tar -xzf FVP_Corstone_SSE-300_11.16_26.tgz \
    && ./FVP_Corstone_SSE-300.sh --i-agree-to-the-contained-eula -d /opt/FVP_Corstone_SSE-300 -f --no-interactive \
    && rm -r FVP_Corstone_SSE-300_11.16_26.tgz FVP_Corstone_SSE-300.sh license_terms \
    && : # last line

FROM connectedhomeip/chip-build:${VERSION}

COPY --from=build /opt/FVP_Corstone_SSE-300/ /opt/FVP_Corstone_SSE-300/

# Required packages for building, running and testing
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
    expect \
    telnet \
    srecord \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

# ------------------------------------------------------------------------------
# Configure environment variables
ENV FVP_CORSTONE_300_PATH=/opt/FVP_Corstone_SSE-300

ENV PATH="${PATH}:${FVP_CORSTONE_300_PATH}/models/Linux64_GCC-6.4"
