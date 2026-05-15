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
# Unpack the sysroots, while also removing some rather large items in them that
# are generally not required for compilation
# Executables are also removed as those confuse cmake. It tries to run `make` from the sysroot and fails.
RUN set -x \
  && SYSROOT_VERSION="build-2026.05.07" \
  && git clone --depth 1 https://chromium.googlesource.com/chromium/tools/depot_tools.git /opt/depot_tools \
  # TODO: Remove experimental solution to create the sysroot file in cross-compile image
  && echo "experimental/matter/sysroot/ubuntu-24.04-aarch64 version:${SYSROOT_VERSION}" > ensure_file.txt \
  && echo "experimental/matter/sysroot/ubuntu-24.04-armhf version:${SYSROOT_VERSION}" >> ensure_file.txt \
  && ./depot_tools/cipd ensure -ensure-file ensure_file.txt -root ./ \
  && : # last line

FROM ghcr.io/project-chip/chip-build:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

COPY --from=build /opt/ubuntu-24.04-aarch64-sysroot/ /opt/ubuntu-24.04-aarch64-sysroot/
COPY --from=build /opt/ubuntu-24.04-armhf-sysroot/ /opt/ubuntu-24.04-armhf-sysroot/

ENV SYSROOT_AARCH64=/opt/ubuntu-24.04-aarch64-sysroot
ENV SYSROOT_ARMHF=/opt/ubuntu-24.04-armhf-sysroot
