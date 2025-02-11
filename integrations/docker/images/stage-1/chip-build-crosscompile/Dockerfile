ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION} as build
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

RUN set -x \
  && apt-get update \
  && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
  git \
  xz-utils \
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/ \
  && : # last line

WORKDIR /opt
# Unpack the sysroot, while also removing some rather large items in it that
# are generally not required for compilation
RUN set -x \
  && git clone --depth 1 https://chromium.googlesource.com/chromium/tools/depot_tools.git /opt/depot_tools \
  # TODO: Remove experimental solution to create the sysroot file in cross-compile image
  && echo 'experimental/matter/sysroot/ubuntu-24.04-aarch64 version:build-2025.01.28' > ensure_file.txt \
  && ./depot_tools/cipd ensure -ensure-file ensure_file.txt -root ./ \
  && rm -rf /opt/ubuntu-24.04-aarch64-sysroot/usr/lib/firmware \
  && rm -rf /opt/ubuntu-24.04-aarch64-sysroot/usr/lib/git-core \
  && rm -rf /opt/ubuntu-24.04-aarch64-sysroot/usr/lib/modules \
  && rm -rf /opt/ubuntu-24.04-aarch64-sysroot/lib/firmware \
  && rm -rf /opt/ubuntu-24.04-aarch64-sysroot/lib/git-core \
  && rm -rf /opt/ubuntu-24.04-aarch64-sysroot/lib/modules \
  && : # last line

FROM ghcr.io/project-chip/chip-build:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

COPY --from=build /opt/ubuntu-24.04-aarch64-sysroot/ /opt/ubuntu-24.04-aarch64-sysroot/

ENV SYSROOT_AARCH64=/opt/ubuntu-24.04-aarch64-sysroot
