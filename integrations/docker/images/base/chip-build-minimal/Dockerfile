# This minimal build image is intentionally not based on chip-build
FROM ubuntu:24.04
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

# ARG NINJA_VERSION=v1.11.1
ARG GN_HASH=5a004f9427a050c6c393c07ddb85cba8ff3849fa

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get upgrade -y \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    generate-ninja \
    git pkg-config \
    ninja-build \
    python3-venv \
    && git config --global advice.detachedHead false

# CHIP build dependencies
RUN set -x \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    libssl-dev libglib2.0-dev
