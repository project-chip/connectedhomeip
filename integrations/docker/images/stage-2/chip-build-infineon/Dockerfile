ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

# ------------------------------------------------------------------------------
# Install prerequisites
RUN apt update -y \
    && apt install -y curl git make file libglib2.0-0 libusb-1.0-0 libncurses5 sudo \
    && apt clean

# ------------------------------------------------------------------------------
# Download and extract ModusToolbox 2.3
RUN curl --fail --location --silent --show-error https://itoolspriv.infineon.com/itbhs/api/packages/com.ifx.tb.tool.modustoolbox/Versions/2.4.0.5972-public/artifacts/ModusToolbox_2.4.0.5972-linux-install.tar.gz/download?noredirect -o /tmp/ModusToolbox_2.4.0.5972-linux-install.tar.gz \
    && tar -C /opt -zxf /tmp/ModusToolbox_2.4.0.5972-linux-install.tar.gz \
    && rm /tmp/ModusToolbox_2.4.0.5972-linux-install.tar.gz

# ------------------------------------------------------------------------------
# Execute post-build scripts
RUN /opt/ModusToolbox/tools_2.4/modus-shell/postinstall

# NOTE: udev rules are NOT installed:
#   /opt/ModusToolbox/tools_2.4/fw-loader/udev_rules/install_rules.sh
# because docker containers do not support udev

# ------------------------------------------------------------------------------
# Set environment variable required by ModusToolbox application makefiles
ENV CY_TOOLS_PATHS="/opt/ModusToolbox/tools_2.4"
