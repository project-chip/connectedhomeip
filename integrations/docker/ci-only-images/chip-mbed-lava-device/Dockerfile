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
    python3 \
    python3-pip \
    python3-dev \
    python3-venv \
    unzip \
    jq \
    && true \
    && : # last line

# Install WiFi AP creation tools
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
    && : # last line

RUN echo "" > /var/lib/dhcp/dhcpd.leases

# Install net-tools required for functional tests
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
    net-tools \
    openbsd-inetd \
    python3-virtualenv \
    && : # last line

# Install GIT hub CLI
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy gnupg curl \
    && curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg | gpg --dearmor -o /usr/share/keyrings/githubcli-archive-keyring.gpg \
    && echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" | tee /etc/apt/sources.list.d/github-cli.list >/dev/null \
    && apt-get update \
    && apt-get install -fy gh \
    && : # last line

RUN rm -rf /var/lib/apt/lists/
