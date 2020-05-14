ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# Setup QEMU emulator for ESP32 platform
RUN set -x \
    && cd /var \
    && git clone --progress https://github.com/espressif/qemu.git qemu_esp32 \
    && (cd /var/qemu_esp32 \
    && ./configure --target-list=xtensa-softmmu --enable-debug --enable-sanitizers --disable-strip --disable-user --disable-capstone --disable-vnc --disable-sdl --disable-gtk \
    && make -j8) \
    && : # last line

ENV QEMU_ESP32_DIR=/var/qemu_esp32
ENV QEMU_ESP32=/var/qemu_esp32/xtensa-softmmu/qemu-system-xtensa
