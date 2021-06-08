FROM ubuntu:20.04

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
    sudo \
    g++ \
    gcc \
    git \
    libavahi-client-dev \
    libcairo2-dev \
    libdbus-1-dev \
    libgirepository1.0-dev \
    libglib2.0-dev \
    libssl-dev \
    ninja-build \
    pkg-config \
    protobuf-compiler \
    python \
    python3-dev \
    python3-venv \
    unzip \
    && true \
    && : # last line

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
    rfkill \
    hostapd \
    iptables \
    isc-dhcp-server \
    iproute2 \
    iw \
    net-tools \
    iputils-ping \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

RUN echo "" > /var/lib/dhcp/dhcpd.leases

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
    net-tools \
    openbsd-inetd \
    python3-virtualenv \
    && : # last line

RUN echo "" > /var/lib/dhcp/dhcpd.leases
