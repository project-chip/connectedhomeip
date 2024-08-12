ARG VERSION=1
FROM ghcr.io/project-chip/chip-build-esp32:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    ninja-build git libgcrypt20-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

WORKDIR /opt/espressif/qemu
# Setup QEMU emulator for ESP32 platform
RUN set -x \
    && git clone --depth 1 -b esp-develop-9.0.0-20240606 https://github.com/espressif/qemu.git ../qemu-src \
    && ../qemu-src/configure --target-list=xtensa-softmmu --enable-debug --disable-strip --disable-user --disable-capstone --disable-vnc --disable-sdl --disable-gtk \
    && make -j$(nproc) \
    && : # last line

ENV QEMU_ESP32_DIR=/opt/espressif/qemu
ENV QEMU_ESP32=/opt/espressif/qemu/qemu-system-xtensa
