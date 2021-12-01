FROM alpine:3.15 as build

RUN apk --no-cache add \
    bash=5.1.8-r0 \
    git=2.34.1-r0 \
    wget=1.21.2-r2

WORKDIR /opt
# Unpack the sysroot, while also removing some rather large items in it that
# are generally not required for compilation
RUN set -x \
    && git clone --depth 1 https://chromium.googlesource.com/chromium/tools/depot_tools.git /opt/depot_tools \
    # TODO: Remove experimental solution to create the sysroot file in cross-compile image
    && echo 'experimental/matter/sysroot/ubuntu-21.04-aarch64 latest' > ensure_file.txt \
    && ./depot_tools/cipd ensure -ensure-file ensure_file.txt -root ./ \
    && tar xfvJ ubuntu-21.04-aarch64-sysroot.tar.xz \
    && rm -rf /opt/ubuntu-21.04-aarch64-sysroot/usr/lib/firmware \
    && rm -rf /opt/ubuntu-21.04-aarch64-sysroot/usr/lib/git-core \
    && rm -rf /opt/ubuntu-21.04-aarch64-sysroot/usr/lib/modules \
    && rm -rf /opt/ubuntu-21.04-aarch64-sysroot/lib/firmware \
    && rm -rf /opt/ubuntu-21.04-aarch64-sysroot/lib/git-core \
    && rm -rf /opt/ubuntu-21.04-aarch64-sysroot/lib/modules \
    && : # last line

FROM alpine:3.15

COPY --from=build /opt/ubuntu-21.04-aarch64-sysroot/ /opt/ubuntu-21.04-aarch64-sysroot/

ENV SYSROOT_AARCH64=/opt/ubuntu-21.04-aarch64-sysroot
