ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION} as build
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip


# Setup SLT CLI (same as scripts/setup/silabs/install-packages.py) and install simplicity-sdk + wiseconnect via sdk-pkg.slt
RUN set -x \
    && mkdir -p /tmp/slt_install \
    && wget -q https://www.silabs.com/documents/public/software/slt-cli-1.0.1-linux-x64.zip -O /tmp/slt.zip \
    && unzip -q /tmp/slt.zip -d /tmp/slt_install \
    && rm /tmp/slt.zip \
    && chmod +x /tmp/slt_install/slt \
    && : # last line

# Copy sdk-pkg.slt and Simplicity SDK keep-list (build context must be repo root)
COPY scripts/setup/silabs/sdk-pkg.slt /tmp/sdk-pkg.slt
COPY scripts/setup/silabs/simplicity_sdk_keep_folders.txt /tmp/keep_folders.txt

# Install packages via SLT (simplicity-sdk, wiseconnect, slc-cli) then copy to /tmp for final image
RUN set -x \
    && /tmp/slt_install/slt install -f /tmp/sdk-pkg.slt \
    && /tmp/slt_install/slt install slc-cli \
    && cp -a $(/tmp/slt_install/slt where simplicity-sdk) /tmp/simplicity_sdk \
    && cp -a $(/tmp/slt_install/slt where wiseconnect) /tmp/wifi_sdk \
    && cp -a $(/tmp/slt_install/slt where slc-cli) /tmp/slc_cli \
    && rm -rf /tmp/slt_install /tmp/sdk-pkg.slt \
    && : # last line

# Trim Simplicity SDK: keep only top-level folders and .a libraries listed in simplicity_sdk_keep_folders.txt; remove all other .a
RUN grep -v '^#' /tmp/keep_folders.txt | grep -v '^$' | grep -v '\.a$' | sort -u > /tmp/keep_list.txt \
    && for d in /tmp/simplicity_sdk/*/ ; do \
         name=$(basename "$d"); \
         grep -qFx "$name" /tmp/keep_list.txt || rm -rf "$d"; \
       done \
    && grep -v '^#' /tmp/keep_folders.txt | grep -v '^$' | grep '\.a$' | sort -u > /tmp/keep_libs.txt \
    && ( cd /tmp/simplicity_sdk && find . -name "*.a" -type f | while read f; do \
         rel="${f#./}"; \
         grep -qFx "$rel" /tmp/keep_libs.txt || rm -f "$f"; \
       done ) \
    && rm /tmp/keep_folders.txt /tmp/keep_list.txt /tmp/keep_libs.txt \
    && find /tmp/simplicity_sdk/openthread -name "*efr32mg21*" -delete \
    && find /tmp/simplicity_sdk/boards/hardware/board/config -mindepth 1 -maxdepth 1 -type d ! \( -name '*brd4186c*' -o -name '*brd4187c*' -o -name '*brd4186a*' -o -name '*brd4187a*' -o -name '*brd2601b*' -o -name '*brd2703a*' -o -name '*brd2704a*' -o -name '*component*' \
    -o -name '*brd4316a*' -o -name '*brd4317a*' -o -name '*brd4318a*' -o -name '*brd4319a*' -o -name '*brd4116a*' -o -name '*brd4117a*' -o -name '*brd4118a*' -o -name '*brd2608a*' -o -name '*brd4350a*' -o -name '*brd4351a*' -o -name '*brd2709a*' \
    -o -name '*brd1019a*' -o -name '*brd2719a*' -o -name '*brd4407a*' -o -name '*brd4408a*' \) -exec rm -rf {} + \
    && find /tmp/simplicity_sdk/devices/platform/Device/SiliconLabs  -mindepth 1 -maxdepth 1 -type d ! \( -name 'EFR32MG24' -o -name 'EFR32MG26' -o -name 'MGM24' -o -name 'MGM26' -o -name 'SIMG301' \) -exec rm -rf {} + \
    && : # last line

# Trim WiSeConnect SDK (from SLT install above) to save space
RUN rm -rf /tmp/wifi_sdk/.git /tmp/wifi_sdk/examples /tmp/wifi_sdk/components/device/stm32 \
    && : # last line

# Final SDK container for compiling using Silabs SDK
FROM ghcr.io/project-chip/chip-build:${VERSION}

ADD requirements.txt /tmp/requirements.txt

# GNU ARM Embedded toolchain, cross compiler for various platform builds
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    gcc-arm-none-eabi \
    binutils-arm-none-eabi \
    openjdk-17-jdk-headless \
    ccache \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    # Install Python Packages
    && pip3 install --break-system-packages -r /tmp/requirements.txt \
    && rm /tmp/requirements.txt \
    && : # last line

# Keep GSDK_ROOT name until rename transition to SISDK is completed
ENV GSDK_ROOT=/opt/silabs/simplicity_sdk/
ENV SISDK_ROOT=/opt/silabs/simplicity_sdk/
ENV WIFI_SDK_ROOT=/opt/silabs/wifi_sdk/
ENV PATH="${PATH}:/opt/silabs/slc_cli/"

COPY --from=build /tmp/simplicity_sdk /opt/silabs/simplicity_sdk
COPY --from=build /tmp/wifi_sdk /opt/silabs/wifi_sdk
COPY --from=build /tmp/slc_cli /opt/silabs/slc_cli
