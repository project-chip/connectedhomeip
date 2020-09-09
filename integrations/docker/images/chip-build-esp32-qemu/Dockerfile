ARG VERSION=latest
FROM connectedhomeip/chip-build-esp32:${VERSION}

# Setup QEMU emulator for ESP32 platform
RUN set -x \
    && mkdir -p /opt/expressif \
    && git clone --progress https://github.com/espressif/qemu.git /opt/expressif/qemu \
    && (cd /opt/expressif/qemu \
    && ./configure --target-list=xtensa-softmmu --enable-debug --enable-sanitizers --disable-strip --disable-user --disable-capstone --disable-vnc --disable-sdl --disable-gtk \
    && make -j8) \
    && : # last line

ENV QEMU_ESP32_DIR=/opt/expressif/qemu
ENV QEMU_ESP32=/opt/expressif/qemu/xtensa-softmmu/qemu-system-xtensa
