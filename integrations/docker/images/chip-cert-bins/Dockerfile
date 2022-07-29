# Stage 1: Setup dependencies (based on chip-build).
FROM ubuntu:22.04 as chip-build-cert
ARG TARGETPLATFORM
# COMMITHASH defines the target commit to build from. May be passed in using --build-arg.
ARG COMMITHASH=e556daac2e1ed3a141034a6dcc7e410e4cd1f8f6

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
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
    autoconf \
    automake \
    bison \
    bridge-utils \
    clang \
    clang-format \
    clang-tidy \
    curl \
    flex \
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
    && rm -rf /var/lib/apt/lists/ \
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
    && DEBIAN_FRONTEND=noninteractive  apt-get update \
    && DEBIAN_FRONTEND=noninteractive  apt-get install -y libgirepository1.0-dev \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y software-properties-common \
    && add-apt-repository universe \
    && curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py \
    && python3 get-pip.py \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

RUN set -x \
    && pip3 install attrs coloredlogs PyGithub pygit future portpicker mobly click cxxfilt ghapi pandas tabulate \
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
    && cmake ../ \
    && make -j8 \
    && make install \
    && cd ../.. \
    && rm -rf bloaty \
    && : # last line

# NodeJS: install a newer version than what apt-get would read
# This installs the latest LTS version of nodejs
RUN case ${TARGETPLATFORM} in \
        "linux/amd64") \
            set -x \
			&& mkdir node_js \
			&& cd node_js \
			&& wget https://nodejs.org/dist/v12.19.0/node-v12.19.0-linux-x64.tar.xz \
			&& tar xfvJ node-v12.19.0-linux-x64.tar.xz \
			&& mv node-v12.19.0-linux-x64 /opt/ \
			&& ln -s /opt/node-v12.19.0-linux-x64 /opt/node \
			&& ln -s /opt/node/bin/* /usr/bin \
			&& cd .. \
			&& rm -rf node_js \
			;; \
        "linux/arm64")\
            set -x \
			&& mkdir node_js \
			&& cd node_js \
			&& wget https://nodejs.org/dist/v12.19.0/node-v12.19.0-linux-arm64.tar.xz \
			&& tar xfvJ node-v12.19.0-linux-arm64.tar.xz \
			&& mv node-v12.19.0-linux-arm64 /opt/ \
			&& ln -s /opt/node-v12.19.0-linux-arm64 /opt/node \
			&& ln -s /opt/node/bin/* /usr/bin \
			&& cd .. \
			&& rm -rf node_js \
			;; \
        *) ;; \
    esac

# Stage 1.5: Bootstrap Matter.
RUN mkdir /root/connectedhomeip
RUN git clone https://github.com/project-chip/connectedhomeip.git /root/connectedhomeip
WORKDIR /root/connectedhomeip/
RUN git checkout ${COMMITHASH}
RUN scripts/build/gn_bootstrap.sh
RUN gn gen out/debug --args='chip_mdns="platform" chip_inet_config_enable_ipv4=false'
RUN ninja -C out/debug

# Stage 2: Build.
from chip-build-cert as chip-build-cert-bins
SHELL ["/bin/bash", "-c"]
# Records Matter SDK commit hash to include in the image.
RUN git rev-parse HEAD > /root/.sdk-sha-version
RUN case ${TARGETPLATFORM} in \
        "linux/amd64") \
            set -x \
			&& source scripts/activate.sh \
			&& scripts/build/build_examples.py \
			--target linux-x64-all-clusters-ipv6only \
			--target linux-x64-all-clusters-minimal-ipv6only \
			--target linux-x64-bridge-ipv6only \
			--target linux-x64-tv-app-ipv6only \
			--target linux-x64-tv-casting-app-ipv6only \
			--target linux-x64-light-ipv6only \
			--target linux-x64-thermostat-ipv6only \
			--target linux-x64-ota-provider-ipv6only \
			--target linux-x64-ota-requestor-ipv6only \
			--target linux-x64-lock-ipv6only \
			build \
			&& mv out/linux-x64-all-clusters-ipv6only/chip-all-clusters-app out/chip-all-clusters-app \
			&& mv out/linux-x64-all-clusters-minimal-ipv6only/chip-all-clusters-minimal-app out/chip-all-clusters-minimal-app \
                        && mv out/linux-x64-bridge-ipv6only/chip-bridge-app out/chip-bridge-app \
			&& mv out/linux-x64-tv-app-ipv6only/chip-tv-app out/chip-tv-app \
			&& mv out/linux-x64-tv-casting-app-ipv6only/chip-tv-casting-app out/chip-tv-casting-app \
			&& mv out/linux-x64-light-ipv6only/chip-lighting-app out/chip-lighting-app \
			&& mv out/linux-x64-thermostat-ipv6only/thermostat-app out/thermostat-app \
			&& mv out/linux-x64-ota-provider-ipv6only/chip-ota-provider-app out/chip-ota-provider-app \
			&& mv out/linux-x64-ota-requestor-ipv6only/chip-ota-requestor-app out/chip-ota-requestor-app \
			&& mv out/linux-x64-lock-ipv6only/chip-lock-app out/chip-lock-app \
		        ;; \
        "linux/arm64")\
            set -x \
			&& source scripts/activate.sh \
			&& scripts/build/build_examples.py \
			--target linux-arm64-all-clusters-ipv6only \
			--target linux-arm64-all-clusters-minimal-ipv6only \
			--target linux-arm64-bridge-ipv6only \
			--target linux-arm64-tv-app-ipv6only \
			--target linux-arm64-tv-casting-app-ipv6only \
			--target linux-arm64-light-ipv6only \
			--target linux-arm64-thermostat-ipv6only \
			--target linux-arm64-ota-provider-ipv6only \
			--target linux-arm64-ota-requestor-ipv6only \
			--target linux-arm64-lock-ipv6only \
			build \
			&& mv out/linux-arm64-all-clusters-ipv6only/chip-all-clusters-app out/chip-all-clusters-app \
			&& mv out/linux-arm64-all-clusters-minimal-ipv6only/chip-all-clusters-minimal-app out/chip-all-clusters-minimal-app \
                        && mv out/linux-arm64-bridge-ipv6only/chip-bridge-app out/chip-bridge-app \
			&& mv out/linux-arm64-tv-app-ipv6only/chip-tv-app out/chip-tv-app \
			&& mv out/linux-arm64-tv-casting-app-ipv6only/chip-tv-casting-app out/chip-tv-casting-app \
			&& mv out/linux-arm64-light-ipv6only/chip-lighting-app out/chip-lighting-app \
			&& mv out/linux-arm64-thermostat-ipv6only/thermostat-app out/thermostat-app \
			&& mv out/linux-arm64-ota-provider-ipv6only/chip-ota-provider-app out/chip-ota-provider-app \
			&& mv out/linux-arm64-ota-requestor-ipv6only/chip-ota-requestor-app out/chip-ota-requestor-app \
			&& mv out/linux-arm64-lock-ipv6only/chip-lock-app out/chip-lock-app \
			;; \
        *) ;; \
    esac

RUN npm --prefix third_party/zap/repo/ ci
RUN scripts/examples/gn_build_test_example.sh app1

# Stage 3: Copy relevant cert bins to a minimal image to reduce size.
FROM ubuntu:22.04
ENV TZ=Etc/UTC
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update -y
RUN apt-get install -y libssl-dev libdbus-1-dev libglib2.0-dev libavahi-client-dev avahi-utils iproute2
WORKDIR /root/
COPY --from=chip-build-cert-bins /root/.sdk-sha-version .sdk-sha-version
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/debug/chip-tool chip-tool
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/debug/chip-shell chip-shell
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/debug/chip-cert chip-cert
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-all-clusters-app chip-all-clusters-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-all-clusters-minimal-app chip-all-clusters-minimal-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-lighting-app chip-lighting-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-tv-casting-app chip-tv-casting-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-tv-app chip-tv-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-bridge-app chip-bridge-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/thermostat-app thermostat-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-ota-provider-app chip-ota-provider-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-ota-requestor-app chip-ota-requestor-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/chip-lock-app chip-lock-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/app1/chip-app1 chip-app1
