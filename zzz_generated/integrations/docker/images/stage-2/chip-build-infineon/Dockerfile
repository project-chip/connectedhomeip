ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

# ------------------------------------------------------------------------------
# Install prerequisites
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --fix-missing --no-install-recommends \
    curl \
    git \
    make \
    file \
    libglib2.0-0 \
    libusb-1.0-0 sudo \
    libxcb-xinerama0 \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-render-util0 \
    libxkbcommon-x11-0 \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

# ------------------------------------------------------------------------------
# Download ModusToolbox 3.2
RUN curl --fail --location --silent --show-error https://itoolspriv.infineon.com/itbhs/api/packages/com.ifx.tb.tool.modustoolbox/Versions/3.2.0.16028-public/artifacts/ModusToolbox_3.2.0.16028-linux-install.deb/download?noredirect -o /tmp/ModusToolbox_3.2.0.16028-linux-install.deb

# ------------------------------------------------------------------------------
# Install ModusToolbox 3.2
RUN apt install /tmp/ModusToolbox_3.2.0.16028-linux-install.deb

# ------------------------------------------------------------------------------
# Remove ModusToolbox deb file
RUN rm /tmp/ModusToolbox_3.2.0.16028-linux-install.deb

# ------------------------------------------------------------------------------
# Run below command to Initialize the CY_TOOLS_PATHS environment variable defined in the /etc/profile.d/modustoolbox_3.2.sh file
RUN bash --login
