ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

RUN set -x \
    # GCC ARM Embedded Toolchain
    && wget -qO- https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 \
        | tar xj -C /var \
    # Device Tree Compiler 1.4.7
    && wget -qO/tmp/dtc.deb http://mirrors.edge.kernel.org/ubuntu/pool/main/d/device-tree-compiler/device-tree-compiler_1.4.7-3ubuntu2_amd64.deb \
    && dpkg -i /tmp/dtc.deb \
    # nRF Tools for flashing software on Nordic devices, and accessing device logs
    && wget -qO- https://www.nordicsemi.com/-/media/Software-and-other-downloads/Desktop-software/nRF-command-line-tools/sw/Versions-10-x-x/10-7-0/nRFCommandLineTools1070Linuxamd64tar.gz \
        | tar xz -C /tmp \
    && dpkg -i /tmp/JLink_Linux_*.deb \
    && dpkg -i /tmp/nRF-Command-Line-Tools_*.deb \
    # nRF Connect SDK dependencies
    && pip3 install --no-cache-dir setuptools wheel cmake west pyyaml \
    # nRF Connect SDK 1.3.0 sources & requirements
    && (mkdir /var/ncs && cd /var/ncs \
    && west init -m https://github.com/nrfconnect/sdk-nrf --mr v1.3.0 \
    && west update \
    && pip3 install --no-cache-dir -r zephyr/scripts/requirements.txt \
    && pip3 install --no-cache-dir -r nrf/scripts/requirements.txt \
    && pip3 install --no-cache-dir -r bootloader/mcuboot/scripts/requirements.txt) \
    # Configuration
    && echo "source /var/ncs/zephyr/zephyr-env.sh" >> ~/.bashrc \
    # Cleanup
    && rm -rf /tmp/* ~/.cache/* \
    && : # last line

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8
ENV ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
ENV GNUARMEMB_TOOLCHAIN_PATH=/var/gcc-arm-none-eabi-9-2020-q2-update