ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}
FROM connectedhomeip/chip-build-nrf-platform:${VERSION} AS nrf
FROM connectedhomeip/chip-build-esp32:${VERSION}
FROM connectedhomeip/chip-build-esp32-qemu:${VERSION}
COPY --from=nrf /var/nRF5_tools /var/nRF5_tools
COPY --from=nrf /var/nRF5_SDK_for_Thread_and_Zigbee /var/nRF5_SDK_for_Thread_and_Zigbee
COPY --from=nrf /var/gcc-arm-none-eabi-9-2019-q4-major /var/gcc-arm-none-eabi-9-2019-q4-major
ENV NRF5_SDK_ROOT=/var/nRF5_SDK_for_Thread_and_Zigbee
ENV NRF5_TOOLS_ROOT=/var/nRF5_tools
ENV ARM_GCC_INSTALL_ROOT=/var/gcc-arm-none-eabi-9-2019-q4-major/bin/
