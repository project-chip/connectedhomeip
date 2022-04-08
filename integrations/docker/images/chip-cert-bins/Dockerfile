FROM ubuntu:focal as chip-build-cert-bins
ARG TARGETPLATFORM
VOLUME "/var/source"

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
    python3.9 \
    python3.9-dev \
    python3.9-venv \
    rsync \
    shellcheck \
    strace \
    systemd \
    udev \
    unzip \
    wget \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/ \
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
        *) ;; \
    esac

# Python 3.9 and PIP
RUN set -x \
    && DEBIAN_FRONTEND=noninteractive  apt-get update \
    && DEBIAN_FRONTEND=noninteractive  apt-get install -y libgirepository1.0-dev \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y software-properties-common \
    && add-apt-repository universe \
    && curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py \
    && python3.9 get-pip.py \
    && update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.9 1 \
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

# Set python to python3
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 10

RUN mkdir /root/connectedhomeip
ADD ./ /root/connectedhomeip/
# RUN git clone --depth 1 https://github.com/project-chip/connectedhomeip.git
WORKDIR /root/connectedhomeip/
RUN scripts/build/gn_bootstrap.sh
RUN gn gen out/debug --args='chip_mdns="platform" chip_inet_config_enable_ipv4=false'
RUN ninja -C out/debug

RUN scripts/examples/gn_build_example.sh examples/all-clusters-app/linux/ examples/all-clusters-app/linux/out/all-clusters-app chip_inet_config_enable_ipv4=false
RUN scripts/examples/gn_build_example.sh examples/bridge-app/linux/ examples/bridge-app/linux/out/bridge-app chip_inet_config_enable_ipv4=false
RUN scripts/examples/gn_build_example.sh examples/tv-app/linux/ examples/tv-app/linux/out/tv-app chip_inet_config_enable_ipv4=false
RUN scripts/examples/gn_build_example.sh examples/tv-casting-app/linux/ examples/tv-casting-app/linux/out/tv-casting-app chip_inet_config_enable_ipv4=false
RUN scripts/examples/gn_build_example.sh examples/lighting-app/linux/ examples/lighting-app/linux/out/lighting-app chip_inet_config_enable_ipv4=false
RUN scripts/examples/gn_build_example.sh examples/thermostat/linux/ examples/thermostat/linux/out/thermostat chip_inet_config_enable_ipv4=false
RUN scripts/examples/gn_build_example.sh examples/ota-provider-app/linux examples/ota-provider-app/linux/out/host 'chip_config_network_layer_ble=false'
RUN scripts/examples/gn_build_example.sh examples/ota-requestor-app/linux examples/ota-requestor-app/linux/out/host 'chip_config_network_layer_ble=false'
RUN scripts/examples/gn_build_example.sh examples/door-lock-app/linux/ out/door-lock-app chip_inet_config_enable_ipv4=false

RUN npm --prefix third_party/zap/repo/ ci
RUN scripts/examples/gn_build_test_example.sh app1

# Stage 2: Copy relevant test harness bins to a new image
FROM ubuntu:21.10
ENV TZ=America/Los_Angeles
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update -y
RUN apt-get install -y libssl-dev libdbus-1-dev libglib2.0-dev libavahi-client-dev avahi-utils iproute2
WORKDIR /root/
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/debug/chip-tool chip-tool
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/debug/chip-shell chip-shell
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/debug/chip-cert chip-cert
COPY --from=chip-build-cert-bins /root/connectedhomeip/examples/all-clusters-app/linux/out/all-clusters-app/chip-all-clusters-app all-cluster-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/examples/lighting-app/linux/out/lighting-app/chip-lighting-app lighting-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/examples/tv-casting-app/linux/out/tv-casting-app/chip-tv-casting-app tv-casting-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/examples/tv-app/linux/out/tv-app/chip-tv-app tv-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/examples/bridge-app/linux/out/bridge-app/chip-bridge-app bridge-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/examples/thermostat/linux/out/thermostat/thermostat-app thermostat-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/examples/ota-provider-app/linux/out/host/chip-ota-provider-app ota-provider-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/examples/ota-requestor-app/linux/out/host/chip-ota-requestor-app ota-requestor-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/door-lock-app/chip-door-lock-app door-lock-app
COPY --from=chip-build-cert-bins /root/connectedhomeip/out/app1/chip-app1 chip-app1
