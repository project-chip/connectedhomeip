ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}
FROM connectedhomeip/chip-build-nrf-platform:${VERSION} AS nrf
FROM connectedhomeip/chip-build-efr32:${VERSION} AS efr32
FROM connectedhomeip/chip-build-android:${VERSION} AS android
FROM connectedhomeip/chip-build-esp32:${VERSION}
FROM connectedhomeip/chip-build-esp32-qemu:${VERSION}
COPY --from=nrf /var/nRF5_tools /opt/nrf5_tools
COPY --from=nrf /var/nRF5_SDK_for_Thread_and_Zigbee /opt/nrf5_sdk
COPY --from=nrf /var/gcc-arm-none-eabi-9-2019-q4-major /opt/gcc-arm-none-eabi-9-2019-q4-major
COPY --from=efr32 /sdk_support /opt/efr32_sdk
COPY --from=android /usr/local/share/android-ndk-r21b /opt/android-ndk-r21b
ENV NRF5_SDK_ROOT=/opt/nrf5_sdk
ENV NRF5_TOOLS_ROOT=/opt/nrf5_tools
ENV ARM_GCC_INSTALL_ROOT=/opt/gcc-arm-none-eabi-9-2019-q4-major/bin
ENV EFR32_SDK_ROOT=/opt/efr32_sdk
ENV EFR32_BOARD=BRD4161A
ENV ANDROID_NDK_HOME=/opt/android-ndk-r21b
ENV PW_ENVIRONMENT_ROOT=/home/vscode/pigweed/env
