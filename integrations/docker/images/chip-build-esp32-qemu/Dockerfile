ARG VERSION=latest
FROM connectedhomeip/chip-build-esp32:${VERSION}

WORKDIR /opt/espressif/qemu

# Setup QEMU emulator for ESP32 platform
RUN set -x \
    && git clone --depth 1 --branch esp-develop-20210220 https://github.com/espressif/qemu.git /opt/espressif/qemu-src \
    && ../qemu-src/configure --target-list=xtensa-softmmu --enable-debug --enable-sanitizers --disable-strip --disable-user --disable-capstone --disable-vnc --disable-sdl --disable-gtk \
    && make -j8 \
    && : # last line

ENV QEMU_ESP32_DIR=/opt/espressif/qemu
ENV QEMU_ESP32=/opt/espressif/qemu/xtensa-softmmu/qemu-system-xtensa
