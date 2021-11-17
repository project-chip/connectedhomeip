ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# Docker dependencies
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
       libclang1-9 \
       libclang-cpp9 \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

# Latest version of doxygen
RUN set -x \
    && cd /opt \
    && wget https://www.doxygen.nl/files/doxygen-1.9.2.linux.bin.tar.gz \
    && tar xfvz doxygen-1.9.2.linux.bin.tar.gz \
    && rm doxygen-1.9.2.linux.bin.tar.gz \
    && ln -s /opt/doxygen-1.9.2/bin/doxygen /usr/bin/ \
    && : # last line
