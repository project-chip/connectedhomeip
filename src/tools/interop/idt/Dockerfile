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
# generic deps
RUN apt-get update
RUN apt-get install -y \
    apt-utils \
    git \
    pkg-config \
    wget
# python
RUN apt-get install -y python3.11
RUN apt-get install -y \
    python3-pip \
    python3.11-venv
# bt deps
RUN apt-get install -y \
    glib-2.0 \
    libglib2.0-dev
RUN apt-get install -y \
    libbluetooth-dev \
    libboost-python-dev \
    libboost-thread-dev
# adb
RUN apt-get install -y \
    adb \
    usbutils
RUN usermod -aG plugdev $(whoami)
# pcaps / net tools
RUN apt-get install -y \
    aircrack-ng \
    kmod \
    net-tools \
    pciutils \
    tcpdump
RUN echo "wireshark-common wireshark-common/install-setuid boolean true" | debconf-set-selections
RUN DEBIAN_FRONTEND=noninteractive apt-get -y install wireshark
# otbr
RUN git clone https://github.com/openthread/ot-br-posix
# python deps
RUN python3 -m venv /env
COPY requirements.txt /
RUN /bin/bash -c "source /env/bin/activate && pip install -r requirements.txt"
# env
RUN echo "source /env/bin/activate; pip -V; source /idt/scripts/alias.sh;" >> /root/.bashrc
ENTRYPOINT ["/bin/bash"]
