ARG chip_build_esp32_VERSION
FROM connectedhomeip/chip-build-esp32:${chip_build_esp32_VERSION}

# Setup QEMU emulator for ESP32 platform
RUN set -x \
    && mkdir -p /opt/espressif \
    && git clone --progress https://github.com/espressif/qemu.git /opt/espressif/qemu \
    && (cd /opt/espressif/qemu \
    && ./configure --target-list=xtensa-softmmu --enable-debug --enable-sanitizers --disable-strip --disable-user --disable-capstone --disable-vnc --disable-sdl --disable-gtk \
    && make -j8) \
    && : # last line

ENV QEMU_ESP32_DIR=/opt/espressif/qemu
ENV QEMU_ESP32=/opt/espressif/qemu/xtensa-softmmu/qemu-system-xtensa
