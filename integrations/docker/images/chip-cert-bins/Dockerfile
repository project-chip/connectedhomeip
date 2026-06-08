# Stage 1: Setup dependencies (based on chip-build).
FROM ubuntu:24.04 AS chip-build-cert
LABEL org.opencontainers.image.source=https://github.com/project-chip/connectedhomeip
ARG TARGETPLATFORM

# Ensure TARGETPLATFORM is set
RUN case ${TARGETPLATFORM} in \
    "linux/amd64") \
    echo "Building for linux/amd64" \
    ;; \
    "linux/arm64") \
    echo "Building for linux/arm64" \
    ;; \
    *) \
    if [ -z "$TARGETPLATFORM" ] ;\
    then \
    echo "TARGETPLATFORM not defined! Please run from buildkit (buildx)." \
    && return 1 ;\
    else \
    echo "Unsupported platform ${TARGETPLATFORM}." \
    && return 1 ;\
    fi \
    ;; \
    esac

# base build and check tools and libraries layer
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    autoconf \
    automake \
    bison \
    bluez \
    bridge-utils \
    ccache \
    clang \
    clang-format \
    clang-tidy \
    cmake \
    curl \
    flex \
    ffmpeg \
    g++ \
    gcc \
    generate-ninja \
    git \
    git-lfs \
    gperf \
    gstreamer1.0-plugins-base \
    gstreamer1.0-tools \
    iproute2 \
    jq \
    lcov \
    libavahi-client-dev \
    libavahi-common-dev \
    libavcodec-dev \
    libavformat-dev \
    libavutil-dev \
    libcairo2-dev \
    libcurl4-openssl-dev \
    libdbus-1-dev \
    libdbus-glib-1-dev \
    libdmalloc-dev \
    libevent-dev \
    libgif-dev \
    libgirepository1.0-dev \
    libglib2.0-dev \
    libgstreamer1.0-0 \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    libical-dev \
    libjpeg-dev \
    libmbedtls-dev \
    libncurses5-dev \
    libncursesw5-dev \
    libnspr4-dev \
    libpango1.0-dev \
    libpcsclite-dev \
    libpixman-1-dev \
    libreadline-dev \
    libssl-dev \
    libtool \
    libudev-dev \
    libusb-1.0-0 \
    libusb-dev \
    libxml2-dev \
    make \
    net-tools \
    ninja-build \
    pkg-config \
    python3 \
    python3-dev \
    python3-pip \
    python3-venv \
    rsync \
    shellcheck \
    software-properties-common \
    strace \
    systemd \
    udev \
    unzip \
    wget \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/ \
    && git lfs install \
    && : # last line

RUN set -x \
    && pip3 install --break-system-packages \
    attrs coloredlogs PyGithub pygit future portpicker mobly click cxxfilt ghapi pandas tabulate \
    && : # last line

# Build and install OpenSSL 3.5 LTS (post-quantum ML-DSA / ML-KEM support)
ARG OPENSSL_VERSION=3.5.0
ARG OPENSSL_SHA256=344d0a79f1a9b08029b0744e2cc401a43f9c90acd1044d09a530b4885a8e9fc0
RUN set -x \
    && cd /tmp \
    && wget -q https://github.com/openssl/openssl/releases/download/openssl-${OPENSSL_VERSION}/openssl-${OPENSSL_VERSION}.tar.gz \
    && echo "${OPENSSL_SHA256}  openssl-${OPENSSL_VERSION}.tar.gz" | sha256sum -c - \
    && tar xzf openssl-${OPENSSL_VERSION}.tar.gz \
    && cd openssl-${OPENSSL_VERSION} \
    && ./Configure --prefix=/usr/local --openssldir=/etc/ssl --libdir=lib \
    && make -j$(nproc) \
    && make install_sw \
    && ldconfig \
    && cd / && rm -rf /tmp/openssl-${OPENSSL_VERSION}* \
    && : # last line

# Stage 1.5: Bootstrap Matter.
# COMMITHASH defines the target commit to build from. May be passed in using --build-arg.
ARG COMMITHASH=c1ec2d777456924dcaa59b53351b00d73caf378f
# This does a shallower checkout of the repo
RUN echo "Cloning at commit: ${COMMITHASH}" \
    mkdir /root/connectedhomeip \
    && git init /root/connectedhomeip \
    && cd /root/connectedhomeip \
    && git init . \
    && git remote add origin https://github.com/project-chip/connectedhomeip.git \
    && git fetch --depth 1 origin ${COMMITHASH} \
    && git checkout FETCH_HEAD

WORKDIR /root/connectedhomeip/
# If building locally, uncomment this line if you encounter failed builds due to warnings
# RUN sed -i 's/treat_warnings_as_errors = true/treat_warnings_as_errors = false/' build/config/compiler/BUILD.gn
RUN ./scripts/checkout_submodules.py --allow-changing-global-git-config --shallow --platform linux --jobs $(nproc)
RUN bash scripts/bootstrap.sh
RUN ./scripts/run_in_build_env.sh "./scripts/codepregen.py ./zzz_pregenerated"

# Stage 2: Build.
FROM chip-build-cert AS chip-build-cert-bins

SHELL ["/bin/bash", "-c"]

# Records Matter SDK commit hash to include in the image.
RUN git rev-parse HEAD > /root/.sdk-sha-version

#setup ccache
ARG CCACHE_NOHASHDIR=1
ARG CCACHE_BASEDIR=/root/connectedhomeip/
ARG CCACHE_NODIRECT=1
ARG CCACHE_PREFIX_CPP=/root/connectedhomeip/scripts/helpers/ccache-prefix-cpp.sh
ARG CCACHE_SLOPPINESS=time_macros
ARG CCACHE_DIR=/root/connectedhomeip/.ccache
ARG CCACHE_COMPILERCHECK=content
ARG CCACHE_MAXSIZE=5G
ARG CCACHE_COMPRESS=1
ARG CCACHE_COMPRESSLEVEL=6
ARG CHIP_PW_COMMAND_LAUNCHER=ccache

# Target suffixes shared across all supported architectures.
# The arch prefix (linux-x64 / linux-arm64) is prepended at build time.
RUN case ${TARGETPLATFORM} in \
        "linux/amd64") ARCH=linux-x64 ;; \
        "linux/arm64") ARCH=linux-arm64 ;; \
        *) \
            echo "ERROR: Unsupported platform ${TARGETPLATFORM}" \
            && exit 1 \
        ;; \
    esac \
    && echo "ARCH=${ARCH}" > /tmp/arch.env

ARG BUILD_EXAMPLES_ARGS="--pw-command-launcher=ccache --build-profile=release-size --pregen-dir ./zzz_pregenerated"

RUN printf '%s\n' \
    chip-tool-ipv6only \
    chip-cert-ipv6only-platform-mdns-nfc-commission \
    shell-ipv6only-platform-mdns \
    fabric-admin-rpc-ipv6only \
    fabric-bridge-rpc-ipv6only \
    > /tmp/targets.txt
RUN --mount=type=cache,target=/root/connectedhomeip/.ccache \
    TARGET_ARGS=() \
    && source /tmp/arch.env \
    && set -x \
    && source scripts/activate.sh \
    && while IFS= read -r suffix; do TARGET_ARGS+=("--target" "${ARCH}-${suffix}"); done < /tmp/targets.txt \
    && scripts/build/build_examples.py ${BUILD_EXAMPLES_ARGS} \
    "${TARGET_ARGS[@]}" \
    build \
    && while IFS= read -r suffix; do \
        find out/${ARCH}-${suffix}-release-size -maxdepth 1 -type f -executable -exec mv {} out/ \; \
        && rm -rf out/${ARCH}-${suffix}-release-size; \
    done < /tmp/targets.txt

RUN printf '%s\n' \
    all-clusters-ipv6only \
    all-clusters-ipv6only-nlfaultinject \
    all-clusters-minimal-ipv6only \
    > /tmp/targets.txt
RUN --mount=type=cache,target=/root/connectedhomeip/.ccache \
    TARGET_ARGS=() \
    && source /tmp/arch.env \
    && set -x \
    && source scripts/activate.sh \
    && while IFS= read -r suffix; do TARGET_ARGS+=("--target" "${ARCH}-${suffix}"); done < /tmp/targets.txt \
    && scripts/build/build_examples.py ${BUILD_EXAMPLES_ARGS} \
    "${TARGET_ARGS[@]}" \
    build \
    && while IFS= read -r suffix; do \
        find out/${ARCH}-${suffix}-release-size -maxdepth 1 -type f -executable -exec mv {} out/ \; \
        && rm -rf out/${ARCH}-${suffix}-release-size; \
    done < /tmp/targets.txt

RUN printf '%s\n' \
    light-ipv6only \
    thermostat-ipv6only \
    lock-ipv6only \
    air-purifier-ipv6only \
    bridge-ipv6only \
    > /tmp/targets.txt
RUN --mount=type=cache,target=/root/connectedhomeip/.ccache \
    TARGET_ARGS=() \
    && source /tmp/arch.env \
    && set -x \
    && source scripts/activate.sh \
    && while IFS= read -r suffix; do TARGET_ARGS+=("--target" "${ARCH}-${suffix}"); done < /tmp/targets.txt \
    && scripts/build/build_examples.py ${BUILD_EXAMPLES_ARGS}\
    "${TARGET_ARGS[@]}" \
    build \
    && while IFS= read -r suffix; do \
        find out/${ARCH}-${suffix}-release-size -maxdepth 1 -type f -executable -exec mv {} out/ \; \
        && rm -rf out/${ARCH}-${suffix}-release-size; \
    done < /tmp/targets.txt

RUN printf '%s\n' \
    tv-app-ipv6only \
    tv-casting-app-ipv6only \
    camera-clang-ipv6only \
    camera-controller-ipv6only \
    > /tmp/targets.txt
RUN --mount=type=cache,target=/root/connectedhomeip/.ccache \
    TARGET_ARGS=() \
    && source /tmp/arch.env \
    && set -x \
    && source scripts/activate.sh \
    && while IFS= read -r suffix; do TARGET_ARGS+=("--target" "${ARCH}-${suffix}"); done < /tmp/targets.txt \
    && scripts/build/build_examples.py ${BUILD_EXAMPLES_ARGS} \
    "${TARGET_ARGS[@]}" \
    build \
    && while IFS= read -r suffix; do \
        find out/${ARCH}-${suffix}-release-size -maxdepth 1 -type f -executable -exec mv {} out/ \; \
        && rm -rf out/${ARCH}-${suffix}-release-size; \
    done < /tmp/targets.txt

RUN printf '%s\n' \
    microwave-oven-ipv6only \
    evse-ipv6only \
    water-heater-ipv6only \
    water-leak-detector-ipv6only \
    rvc-ipv6only \
    > /tmp/targets.txt
RUN --mount=type=cache,target=/root/connectedhomeip/.ccache \
    TARGET_ARGS=() \
    && source /tmp/arch.env \
    && set -x \
    && source scripts/activate.sh \
    && while IFS= read -r suffix; do TARGET_ARGS+=("--target" "${ARCH}-${suffix}"); done < /tmp/targets.txt \
    && scripts/build/build_examples.py ${BUILD_EXAMPLES_ARGS} \
    "${TARGET_ARGS[@]}" \
    build \
    && while IFS= read -r suffix; do \
        find out/${ARCH}-${suffix}-release-size -maxdepth 1 -type f -executable -exec mv {} out/ \; \
        && rm -rf out/${ARCH}-${suffix}-release-size; \
    done < /tmp/targets.txt

RUN printf '%s\n' \
    simulated-app1-ipv6only \
    lit-icd-ipv6only \
    energy-gateway-ipv6only \
    closure-ipv6only \
    > /tmp/targets.txt
RUN --mount=type=cache,target=/root/connectedhomeip/.ccache \
    TARGET_ARGS=() \
    && source /tmp/arch.env \
    && set -x \
    && source scripts/activate.sh \
    && while IFS= read -r suffix; do TARGET_ARGS+=("--target" "${ARCH}-${suffix}"); done < /tmp/targets.txt \
    && scripts/build/build_examples.py ${BUILD_EXAMPLES_ARGS}\
    "${TARGET_ARGS[@]}" \
    build \
    && while IFS= read -r suffix; do \
        find out/${ARCH}-${suffix}-release-size -maxdepth 1 -type f -executable -exec mv {} out/ \; \
        && rm -rf out/${ARCH}-${suffix}-release-size; \
    done < /tmp/targets.txt

RUN printf '%s\n' \
    jf-control-app-ipv6only \
    jf-admin-app-ipv6only \
    > /tmp/targets.txt
RUN --mount=type=cache,target=/root/connectedhomeip/.ccache \
    TARGET_ARGS=() \
    && source /tmp/arch.env \
    && set -x \
    && source scripts/activate.sh \
    && while IFS= read -r suffix; do TARGET_ARGS+=("--target" "${ARCH}-${suffix}"); done < /tmp/targets.txt \
    && scripts/build/build_examples.py ${BUILD_EXAMPLES_ARGS}\
    "${TARGET_ARGS[@]}" \
    build \
    && while IFS= read -r suffix; do \
        find out/${ARCH}-${suffix}-release-size -maxdepth 1 -type f -executable -exec mv {} out/ \; \
        && rm -rf out/${ARCH}-${suffix}-release-size; \
    done < /tmp/targets.txt

RUN printf '%s\n' \
    ota-provider-ipv6only \
    ota-requestor-ipv6only \
    > /tmp/targets.txt
RUN --mount=type=cache,target=/root/connectedhomeip/.ccache \
    TARGET_ARGS=() \
    && source /tmp/arch.env \
    && set -x \
    && source scripts/activate.sh \
    && while IFS= read -r suffix; do TARGET_ARGS+=("--target" "${ARCH}-${suffix}"); done < /tmp/targets.txt \
    && scripts/build/build_examples.py ${BUILD_EXAMPLES_ARGS} \
    "${TARGET_ARGS[@]}" \
    build \
    && while IFS= read -r suffix; do \
        find out/${ARCH}-${suffix}-release-size -maxdepth 1 -type f -executable -exec mv {} out/ \; \
        && rm -rf out/${ARCH}-${suffix}-release-size; \
    done < /tmp/targets.txt

RUN --mount=type=cache,target=/root/connectedhomeip/.ccache \
    source scripts/activate.sh \
    && scripts/build_python.sh \
        -m platform \
        -d true \
        --enable_nfc true \
        --enable-ccache

# Generate OTA image from the built OTA requestor app
# This creates ota-requestor-app.ota for OTA testing
RUN case ${TARGETPLATFORM} in \
    "linux/amd64" | "linux/arm64") \
    set -x \
    && cd out \
    && ../src/app/ota_image_tool.py create \
       -v 0xDEAD \
       -p 0xBEEF \
       -vn 2 \
       -vs "2.0" \
       -da sha256 \
       chip-ota-requestor-app \
       ota-requestor-app.ota \
    ;; \
    *) ;; \
    esac

RUN find out -maxdepth 1 -type f -executable -exec strip --strip-all {} \;

# Stage 3: Copy relevant cert bins to a minimal image to reduce size.
FROM ubuntu:24.04
ENV TZ=Etc/UTC
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update -y \
    && DEBIAN_FRONTEND=noninteractive apt-get satisfy -y --no-install-recommends \
    avahi-utils \
    ffmpeg \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-ugly \
    iproute2 \
    libavahi-client-dev \
    libcairo2-dev \
    libcurl4 \
    libdbus-1-dev \
    libevent-dev \
    libgirepository1.0-dev \
    libglib2.0-dev \
    libgstreamer1.0-0 \
    libgstreamer-plugins-base1.0-0 \
    libpcsclite1 \
    libpcsclite-dev \
    libssl-dev \
    meson \
    pcscd \
    python3-dev \
    python3-pip \
    && : # last line

WORKDIR /root/
COPY --from=chip-build-cert-bins /root/.sdk-sha-version .sdk-sha-version
COPY --from=chip-build-cert-bins /root/connectedhomeip/out apps
COPY --from=chip-build-cert-bins /root/connectedhomeip/examples/fabric-admin/scripts/fabric-sync-app.py apps/fabric-sync-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/src/tools/push_av_server apps/push_av_server

# Create symbolic links for now since this allows users to use existing configurations
# for running just `app-name` instead of `apps/app-name`
RUN ln -s apps/* .

# Stage 3.1: Setup the Matter Python environment
COPY --from=chip-build-cert-bins /root/connectedhomeip/src/python_testing python_testing/scripts/sdk
COPY --from=chip-build-cert-bins /root/connectedhomeip/data_model python_testing/data_model
RUN pip install --break-system-packages -r python_testing/scripts/sdk/requirements.txt \
    && rm python_testing/scripts/sdk/requirements.txt

# Install PushAV dependencies
RUN pip install --break-system-packages -r apps/push_av_server/requirements.txt \
    && patch -d $(python3 -c "from importlib.metadata import distribution; print(distribution('hypercorn').locate_file(''))") -p0 < apps/push_av_server/hypercorn.patch

RUN pip install --break-system-packages -r python_testing/scripts/sdk/requirements.nfc.txt \
    && rm python_testing/scripts/sdk/requirements.nfc.txt

# Stage 3.2: Setup the Mock Server
COPY --from=chip-build-cert-bins /root/connectedhomeip/integrations/mock_server mock_server

WORKDIR /root/apps
RUN pip install --break-system-packages --no-cache-dir \
    python_lib/python/obj/scripts/py_matter_idl/matter-idl._build_wheel/matter_idl-*.whl \
    python_lib/python/obj/scripts/py_matter_yamltests/matter-yamltests._build_wheel/matter_yamltests-*.whl \
    python_lib/obj/src/python_testing/matter_testing_infrastructure/matter-testing._build_wheel/matter_testing-*.whl \
    python_lib/obj/src/controller/python/matter-controller-wheels/*.whl \
    && rm -rf ../python_lib \
    && rm -rf python_lib \
    && rm -rf /root/.cache

# Copy device attestation revocation set and device attestation test vectors
WORKDIR /root
RUN mkdir -p credentials/test/revoked-attestation-certificates
COPY --from=chip-build-cert-bins /root/connectedhomeip/credentials/test/revoked-attestation-certificates/dac-provider-test-vectors credentials/test/revoked-attestation-certificates/dac-provider-test-vectors
COPY --from=chip-build-cert-bins /root/connectedhomeip/credentials/test/revoked-attestation-certificates/revocation-sets credentials/test/revoked-attestation-certificates/revocation-sets

RUN apt-get remove -y \
    meson

RUN apt-get autoremove -y
RUN apt-get clean
