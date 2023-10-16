ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

# ------------------------------------------------------------------------------
# Add group/user for tizen
ARG USER_NAME=tizen
ARG USER_UID=1000
ARG USER_GID=$USER_UID
ENV USER_HOME /home/$USER_NAME

RUN set -x \
    && groupadd -g $USER_GID $USER_NAME \
    && useradd -m $USER_NAME -s /bin/bash -u $USER_UID -g $USER_GID -G sudo -l \
    && : # last line

# ------------------------------------------------------------------------------
# Install dependencies
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    cpio \
    libncurses5 \
    obs-build \
    openjdk-8-jre-headless \
    wget \
    zip \
    # Cleanup
    && apt-get clean \
    && rm -rf /var/lib/apt/lists \
    && : # last line

# ------------------------------------------------------------------------------
# Install tizen
ENV TIZEN_VERSION 7.0
ENV TIZEN_SDK_ROOT /opt/tizen-sdk

COPY tizen-sdk-installer $TIZEN_SDK_ROOT/files/installer
RUN set -x \
    && bash $TIZEN_SDK_ROOT/files/installer/install.sh \
    --tizen-sdk-path $TIZEN_SDK_ROOT \
    --tizen-sdk-data-path $USER_HOME/tizen-sdk-data \
    --tizen-version $TIZEN_VERSION  \
    --override-secret-tool \
    && : # last line

# ------------------------------------------------------------------------------
# Set environment
ENV TIZEN_SDK_TOOLCHAIN $TIZEN_SDK_ROOT/tools/arm-linux-gnueabi-gcc-9.2
ENV TIZEN_SDK_SYSROOT $TIZEN_SDK_ROOT/platforms/tizen-$TIZEN_VERSION/mobile/rootstraps/mobile-$TIZEN_VERSION-device.core
ENV PATH="$TIZEN_SDK_TOOLCHAIN/bin:$TIZEN_SDK_ROOT/tools/ide/bin:$TIZEN_SDK_ROOT/tools:$PATH"

# ------------------------------------------------------------------------------
# Switch to the non-root user
USER $USER_NAME
WORKDIR $USER_HOME
