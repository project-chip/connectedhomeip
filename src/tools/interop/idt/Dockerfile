#
#    Copyright (c) 2023 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

FROM debian:bookworm
RUN apt-get update && \
    apt-get install -y \
    adb \
    aircrack-ng \
    apt-utils \
    bison \
    byacc \
    dnsutils \
    flex \
    gcc-aarch64-linux-gnu \
    gcc-arm-linux-gnueabi \
    git \
    glib-2.0 \
    kmod \
    libbluetooth-dev \
    libboost-python-dev \
    libboost-thread-dev \
    libglib2.0-dev \
    net-tools \
    pciutils \
    pkg-config \
    python3-pip \
    python3.11 \
    python3.11-venv \
    tcpdump \
    usbutils \
    wget && \
    echo "wireshark-common wireshark-common/install-setuid boolean true" | debconf-set-selections && \
    DEBIAN_FRONTEND=noninteractive apt-get -y install wireshark && \
    rm -rf /var/lib/apt/lists/*
RUN usermod -aG plugdev $(whoami)
RUN git clone https://github.com/openthread/ot-br-posix
COPY requirements.txt /
RUN /bin/bash -c "python3 -m venv /env && source /env/bin/activate && pip install -r requirements.txt"
RUN echo "source /env/bin/activate; pip -V; source /idt/scripts/alias.sh;" >> /root/.bashrc
ENTRYPOINT ["/bin/bash"]
