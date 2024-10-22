# Stage 1: Setup dependencies (based on chip-build).
FROM ubuntu:24.04 AS chip-build-cert
LABEL org.opencontainers.image.source=https://github.com/project-chip/connectedhomeip
ARG TARGETPLATFORM
# COMMITHASH defines the target commit to build from. May be passed in using --build-arg.
ARG COMMITHASH=c1ec2d777456924dcaa59b53351b00d73caf378f

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

# Below should be the same as chip-build except arm64 logic for cmake and node.

# base build and check tools and libraries layer
RUN set -x \
    && apt-get update \
    && apt-get upgrade -y \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
    autoconf \
    automake \
    bison \
    bluez \
    bridge-utils \
    clang \
    clang-format \
    clang-tidy \
    curl \
    flex \
    gcc \
    g++ \
    git \
    gperf \
    iproute2 \
    jq \
    lcov \
    libavahi-client-dev \
    libavahi-common-dev \
    libcairo2-dev \
    libdbus-1-dev \
    libdbus-glib-1-dev \
    libgif-dev \
    libglib2.0-dev \
    libical-dev \
    libjpeg-dev \
    libdmalloc-dev \
    libmbedtls-dev \
    libncurses5-dev \
    libncursesw5-dev \
    libnspr4-dev \
    libpango1.0-dev \
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
    openjdk-8-jdk \
    pkg-config \
    python3 \
    python3-dev \
    python3-venv \
    rsync \
    shellcheck \
    strace \
    systemd \
    udev \
    unzip \
    wget \
    git-lfs \
    zlib1g-dev \
    && git lfs install \
    && : # last line

# Cmake (Mbed OS requires >=3.19.0-rc3 version which is not available in Ubuntu 20.04 repository)
RUN case ${TARGETPLATFORM} in \
    "linux/amd64") \
    set -x \
    && (cd /tmp \
    && wget --progress=dot:giga https://github.com/Kitware/CMake/releases/download/v3.19.3/cmake-3.19.3-Linux-x86_64.sh \
    && sh cmake-3.19.3-Linux-x86_64.sh --exclude-subdir --prefix=/usr/local \
    && rm -rf cmake-3.19.3-Linux-x86_64.sh) \
    && exec bash \
    ;; \
    "linux/arm64") \
    set -x \
    && (cd /tmp \
    && wget --progress=dot:giga https://github.com/Kitware/CMake/releases/download/v3.19.3/cmake-3.19.3-Linux-aarch64.sh \
    && sh cmake-3.19.3-Linux-aarch64.sh --exclude-subdir --prefix=/usr/local \
    && rm -rf cmake-3.19.3-Linux-aarch64.sh) \
    && exec bash \
    ;; \
    *) \
    test -n "$TARGETPLATFORM" \
    echo "Unsupported platform ${TARGETPLATFORM}" \
    ;; \
    esac

# Python 3 and PIP
RUN set -x \
    && DEBIAN_FRONTEND=noninteractive  apt-get install -y \
    libgirepository1.0-dev \
    software-properties-common \
    && add-apt-repository universe \
    && curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py \
    && python3 get-pip.py --break-system-packages \
    && : # last line

RUN set -x \
    && pip3 install attrs coloredlogs PyGithub pygit future portpicker mobly click cxxfilt ghapi pandas tabulate --break-system-packages \
    && : # last line

# build and install gn
RUN set -x \
    && git clone https://gn.googlesource.com/gn \
    && cd gn \
    && python3 build/gen.py \
    && ninja -C out \
    && cp out/gn /usr/local/bin \
    && cd .. \
    && rm -rf gn \
    && : # last line

# Install bloat comparison tools
RUN set -x \
    && git clone https://github.com/google/bloaty.git \
    && mkdir -p bloaty/build \
    && cd bloaty/build \
    && cmake -G Ninja ../ \
    && ninja \
    && ninja install \
    && cd ../.. \
    && rm -rf bloaty \
    && : # last line

# Stage 1.5: Bootstrap Matter.
RUN mkdir /root/connectedhomeip
RUN git clone https://github.com/project-chip/connectedhomeip.git /root/connectedhomeip
WORKDIR /root/connectedhomeip/
RUN git checkout ${COMMITHASH}
RUN ./scripts/checkout_submodules.py --allow-changing-global-git-config --shallow --platform linux
RUN bash scripts/bootstrap.sh

# Stage 2: Build.
FROM chip-build-cert AS chip-build-cert-bins

SHELL ["/bin/bash", "-c"]

# Records Matter SDK commit hash to include in the image.
RUN git rev-parse HEAD > /root/.sdk-sha-version

RUN case ${TARGETPLATFORM} in \
    "linux/amd64") \
    set -x \
    && source scripts/activate.sh \
    && scripts/build/build_examples.py \
    --target linux-x64-chip-tool-ipv6only-platform-mdns \
    --target linux-x64-shell-ipv6only-platform-mdns \
    --target linux-x64-chip-cert-ipv6only-platform-mdns \
    --target linux-x64-all-clusters-ipv6only \
    --target linux-x64-all-clusters-ipv6only-nlfaultinject \
    --target linux-x64-all-clusters-minimal-ipv6only \
    --target linux-x64-bridge-ipv6only \
    --target linux-x64-tv-app-ipv6only \
    --target linux-x64-tv-casting-app-ipv6only \
    --target linux-x64-light-ipv6only \
    --target linux-x64-thermostat-ipv6only \
    --target linux-x64-ota-provider-ipv6only \
    --target linux-x64-ota-requestor-ipv6only \
    --target linux-x64-lock-ipv6only \
    --target linux-x64-simulated-app1-ipv6only \
    --target linux-x64-lit-icd-ipv6only \
    --target linux-x64-energy-management-ipv6only \
    --target linux-x64-microwave-oven-ipv6only \
    --target linux-x64-rvc-ipv6only \
    --target linux-x64-fabric-bridge-rpc-ipv6only \
    --target linux-x64-fabric-admin-rpc-ipv6only \
    --target linux-x64-light-data-model-no-unique-id-ipv6only \
    --target linux-x64-network-manager-ipv6only \
    build \
    && mv out/linux-x64-chip-tool-ipv6only-platform-mdns/chip-tool out/chip-tool \
    && mv out/linux-x64-shell-ipv6only-platform-mdns/chip-shell out/chip-shell \
    && mv out/linux-x64-chip-cert-ipv6only-platform-mdns/chip-cert out/chip-cert \
    && mv out/linux-x64-all-clusters-ipv6only/chip-all-clusters-app out/chip-all-clusters-app \
    && mv out/linux-x64-all-clusters-ipv6only-nlfaultinject/chip-all-clusters-app out/chip-all-clusters-app-nlfaultinject \
    && mv out/linux-x64-all-clusters-minimal-ipv6only/chip-all-clusters-minimal-app out/chip-all-clusters-minimal-app \
    && mv out/linux-x64-bridge-ipv6only/chip-bridge-app out/chip-bridge-app \
    && mv out/linux-x64-tv-app-ipv6only/chip-tv-app out/chip-tv-app \
    && mv out/linux-x64-tv-casting-app-ipv6only/chip-tv-casting-app out/chip-tv-casting-app \
    && mv out/linux-x64-light-ipv6only/chip-lighting-app out/chip-lighting-app \
    && mv out/linux-x64-thermostat-ipv6only/thermostat-app out/thermostat-app \
    && mv out/linux-x64-ota-provider-ipv6only/chip-ota-provider-app out/chip-ota-provider-app \
    && mv out/linux-x64-ota-requestor-ipv6only/chip-ota-requestor-app out/chip-ota-requestor-app \
    && mv out/linux-x64-lock-ipv6only/chip-lock-app out/chip-lock-app \
    && mv out/linux-x64-simulated-app1-ipv6only/chip-app1 out/chip-app1 \
    && mv out/linux-x64-lit-icd-ipv6only/lit-icd-app out/lit-icd-app \
    && mv out/linux-x64-energy-management-ipv6only/chip-energy-management-app out/chip-energy-management-app \
    && mv out/linux-x64-microwave-oven-ipv6only/chip-microwave-oven-app out/chip-microwave-oven-app \
    && mv out/linux-x64-rvc-ipv6only/chip-rvc-app out/chip-rvc-app \
    && mv out/linux-x64-fabric-bridge-rpc-ipv6only/fabric-bridge-app out/fabric-bridge-app \
    && mv out/linux-x64-fabric-admin-rpc-ipv6only/fabric-admin out/fabric-admin \
    && mv out/linux-x64-light-data-model-no-unique-id-ipv6only/chip-lighting-app out/chip-lighting-data-model-no-unique-id-app \
    && mv out/linux-x64-network-manager-ipv6only/matter-network-manager-app out/matter-network-manager-app \
    ;; \
    "linux/arm64")\
    set -x \
    && source scripts/activate.sh \
    && scripts/build/build_examples.py \
    --target linux-arm64-chip-tool-ipv6only-platform-mdns \
    --target linux-arm64-shell-ipv6only-platform-mdns \
    --target linux-arm64-chip-cert-ipv6only-platform-mdns \
    --target linux-arm64-all-clusters-ipv6only \
    --target linux-arm64-all-clusters-ipv6only-nlfaultinject \
    --target linux-arm64-all-clusters-minimal-ipv6only \
    --target linux-arm64-bridge-ipv6only \
    --target linux-arm64-tv-app-ipv6only \
    --target linux-arm64-tv-casting-app-ipv6only \
    --target linux-arm64-light-ipv6only \
    --target linux-arm64-thermostat-ipv6only \
    --target linux-arm64-ota-provider-ipv6only \
    --target linux-arm64-ota-requestor-ipv6only \
    --target linux-arm64-lock-ipv6only \
    --target linux-arm64-simulated-app1-ipv6only \
    --target linux-arm64-lit-icd-ipv6only \
    --target linux-arm64-energy-management-ipv6only \
    --target linux-arm64-microwave-oven-ipv6only \
    --target linux-arm64-rvc-ipv6only \
    --target linux-arm64-fabric-bridge-rpc-ipv6only \
    --target linux-arm64-fabric-admin-rpc-ipv6only \
    --target linux-arm64-light-data-model-no-unique-id-ipv6only \
    --target linux-arm64-network-manager-ipv6only \
    build \
    && mv out/linux-arm64-chip-tool-ipv6only-platform-mdns/chip-tool out/chip-tool \
    && mv out/linux-arm64-shell-ipv6only-platform-mdns/chip-shell out/chip-shell \
    && mv out/linux-arm64-chip-cert-ipv6only-platform-mdns/chip-cert out/chip-cert \
    && mv out/linux-arm64-all-clusters-ipv6only/chip-all-clusters-app out/chip-all-clusters-app \
    && mv out/linux-arm64-all-clusters-ipv6only-nlfaultinject/chip-all-clusters-app out/chip-all-clusters-app-nlfaultinject \
    && mv out/linux-arm64-all-clusters-minimal-ipv6only/chip-all-clusters-minimal-app out/chip-all-clusters-minimal-app \
    && mv out/linux-arm64-bridge-ipv6only/chip-bridge-app out/chip-bridge-app \
    && mv out/linux-arm64-tv-app-ipv6only/chip-tv-app out/chip-tv-app \
    && mv out/linux-arm64-tv-casting-app-ipv6only/chip-tv-casting-app out/chip-tv-casting-app \
    && mv out/linux-arm64-light-ipv6only/chip-lighting-app out/chip-lighting-app \
    && mv out/linux-arm64-thermostat-ipv6only/thermostat-app out/thermostat-app \
    && mv out/linux-arm64-ota-provider-ipv6only/chip-ota-provider-app out/chip-ota-provider-app \
    && mv out/linux-arm64-ota-requestor-ipv6only/chip-ota-requestor-app out/chip-ota-requestor-app \
    && mv out/linux-arm64-lock-ipv6only/chip-lock-app out/chip-lock-app \
    && mv out/linux-arm64-simulated-app1-ipv6only/chip-app1 out/chip-app1 \
    && mv out/linux-arm64-lit-icd-ipv6only/lit-icd-app out/lit-icd-app \
    && mv out/linux-arm64-energy-management-ipv6only/chip-energy-management-app out/chip-energy-management-app \
    && mv out/linux-arm64-microwave-oven-ipv6only/chip-microwave-oven-app out/chip-microwave-oven-app \
    && mv out/linux-arm64-rvc-ipv6only/chip-rvc-app out/chip-rvc-app \
    && mv out/linux-arm64-fabric-bridge-rpc-ipv6only/fabric-bridge-app out/fabric-bridge-app \
    && mv out/linux-arm64-fabric-admin-rpc-ipv6only/fabric-admin out/fabric-admin \
    && mv out/linux-arm64-light-data-model-no-unique-id-ipv6only/chip-lighting-app out/chip-lighting-data-model-no-unique-id-app \
    && mv out/linux-arm64-network-manager-ipv6only/matter-network-manager-app out/matter-network-manager-app \
    ;; \
    *) ;; \
    esac

RUN source scripts/activate.sh && scripts/build_python.sh -m platform -d true -i out/python_env

# Stage 3: Copy relevant cert bins to a minimal image to reduce size.
FROM ubuntu:24.04
ENV TZ=Etc/UTC
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update -y
RUN apt-get install -y libssl-dev libdbus-1-dev libglib2.0-dev libavahi-client-dev avahi-utils iproute2 libcairo2-dev libgirepository1.0-dev python3-pip
WORKDIR /root/
COPY --from=chip-build-cert-bins /root/.sdk-sha-version .sdk-sha-version
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-tool chip-tool
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-shell chip-shell
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-cert chip-cert
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-all-clusters-app chip-all-clusters-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-all-clusters-app-nlfaultinject chip-all-clusters-app-nlfaultinject
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-all-clusters-minimal-app chip-all-clusters-minimal-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-lighting-app chip-lighting-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-tv-casting-app chip-tv-casting-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-tv-app chip-tv-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-bridge-app chip-bridge-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/thermostat-app thermostat-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-ota-provider-app chip-ota-provider-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-ota-requestor-app chip-ota-requestor-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-lock-app chip-lock-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-app1 chip-app1
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/lit-icd-app lit-icd-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-energy-management-app chip-energy-management-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-microwave-oven-app chip-microwave-oven-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-rvc-app chip-rvc-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/examples/fabric-admin/scripts/fabric-sync-app.py fabric-sync-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/fabric-bridge-app fabric-bridge-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/fabric-admin fabric-admin
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-lighting-data-model-no-unique-id-app chip-lighting-data-model-no-unique-id-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/matter-network-manager-app matter-network-manager-app

# Stage 3.1: Setup the Matter Python environment
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/python_lib python_lib
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/python_env python_env
COPY --from=chip-build-cert-bins /root/connectedhomeip/src/python_testing python_testing/scripts/sdk
COPY --from=chip-build-cert-bins /root/connectedhomeip/data_model python_testing/data_model

COPY --from=chip-build-cert-bins /root/connectedhomeip/scripts/tests/requirements.txt /tmp/requirements.txt
RUN pip install --break-system-packages -r /tmp/requirements.txt && rm /tmp/requirements.txt

COPY --from=chip-build-cert-bins /root/connectedhomeip/src/python_testing/requirements.txt /tmp/requirements.txt
RUN pip install --break-system-packages -r /tmp/requirements.txt && rm /tmp/requirements.txt

# PIP requires MASON package compilation, which seems to require a JDK
RUN set -x && DEBIAN_FRONTEND=noninteractive apt-get update; apt-get install -fy openjdk-8-jdk

RUN pip install --break-system-packages --no-cache-dir python_lib/controller/python/chip*.whl
