# This minimal build image is intentionally not based on chip-build
FROM ubuntu:focal
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

# ARG NINJA_VERSION=v1.11.1
ARG GN_HASH=5a004f9427a050c6c393c07ddb85cba8ff3849fa

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get upgrade -y \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential ca-certificates git pkg-config python3-venv ninja-build \
    && git config --global advice.detachedHead false

# RUN set -x && cd /var/tmp \
#     && git clone --branch "$NINJA_VERSION" https://github.com/ninja-build/ninja.git \
#     && ( cd ninja && ./configure.py --bootstrap && install -m 0755 ninja /usr/local/bin/ ) \
#     && rm -rf ninja

RUN set -x && cd /var/tmp \
    && git clone https://gn.googlesource.com/gn \
    && ( cd gn && git checkout "$GN_HASH" && CXX=g++ build/gen.py && ninja -C out && install -m 0755 out/gn /usr/local/bin/ ) \
    && rm -rf gn

# CHIP build dependencies
RUN set -x \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    libssl-dev libglib2.0-dev
