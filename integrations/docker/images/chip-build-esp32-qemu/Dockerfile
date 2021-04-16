ARG VERSION=latest
FROM connectedhomeip/chip-build-esp32:${VERSION}

# Setup QEMU emulator for ESP32 platform
RUN set -x \
    && mkdir -p /opt/espressif \
    && git clone --progress --depth 1 --branch esp-develop-20210220 https://github.com/espressif/qemu.git /opt/espressif/qemu-src \
    && mkdir -p /opt/espressif/qemu \
    && (cd /opt/espressif/qemu \
    && ../qemu-src/configure --target-list=xtensa-softmmu --enable-debug --enable-sanitizers --disable-strip --disable-user --disable-capstone --disable-vnc --disable-sdl --disable-gtk \
    && make -j8) \
    && : # last line

ENV QEMU_ESP32_DIR=/opt/espressif/qemu
ENV QEMU_ESP32=/opt/espressif/qemu/xtensa-softmmu/qemu-system-xtensa
