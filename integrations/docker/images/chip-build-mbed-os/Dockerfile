ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# ------------------------------------------------------------------------------
# Install system tools via apt
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
    python3-setuptools \
    python3-usb \
    software-properties-common \
    build-essential \
    astyle \
    mercurial \
    && rm -rf /var/lib/apt/lists \
    && : # last line

# ------------------------------------------------------------------------------
# Install Python modules
RUN set -x \
    && pip3 install --no-cache-dir -U mbed-cli mbed-tools \
    && : # last line

# ------------------------------------------------------------------------------
# Install openocd
RUN set -x \
    && (mkdir -p /opt/openocd \
    && cd /opt/openocd \
    && wget --progress=dot:giga https://github.com/cypresssemiconductorco/openocd/releases/download/release-v4.2.0/openocd-4.2.0.1430-linux.tar.gz \
    && tar --strip-components=1 -xzf openocd-4.2.0.1430-linux.tar.gz \
    && rm openocd-4.2.0.1430-linux.tar.gz) \
    && : # last line

# ------------------------------------------------------------------------------
# Configure environment variables
ENV OPENOCD_PATH=/opt/openocd/

ENV PATH="${PATH}:${OPENOCD_PATH}/bin"
