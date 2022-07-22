# Copyright (c) 2020 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ARG BUILD_VERSION

# All tools required for compilation belong in chip-build, forms "truth" for CHIP build tooling
FROM connectedhomeip/chip-build-vscode:${BUILD_VERSION}

# This Dockerfile contains things useful for an interactive development environment
ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID
ENV LANG en_US.utf8

# these are installed for terminal/dev convenience.  If more tooling for build is required, please
#  add them to chip-build (in integrations/docker/images/chip-build)
RUN apt-get update
RUN apt-get install -y locales && localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8
RUN apt-get -fy install git vim emacs sudo \
    apt-utils dialog zsh \
    iproute2 procps lsb-release \
    bash-completion \
    build-essential cmake cppcheck valgrind \
    wget curl telnet \
    docker.io \
    iputils-ping net-tools \
    libncurses5 \
    libpython2.7

RUN groupadd -g $USER_GID $USERNAME
RUN useradd -s /bin/bash -u $USER_UID -g $USER_GID -G docker -m $USERNAME
RUN echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME
RUN chmod 0440 /etc/sudoers.d/$USERNAME

RUN mkdir -p /var/downloads
RUN cd /var/downloads
RUN curl -JL https://github.com/microsoft/vscode-cpptools/releases/download/0.27.0/cpptools-linux.vsix > extension.zip
RUN unzip extension.zip
RUN mkdir -p /home/$USERNAME/.vscode-server/extensions
RUN mv extension /home/$USERNAME/.vscode-server/extensions/ms-vscode.cpptools-0.27.0
RUN mkdir -p /home/$USERNAME/bin
RUN curl https://raw.githubusercontent.com/restyled-io/restyler/master/bin/restyle-path -o /home/$USERNAME/bin/restyle-path
RUN chmod +x /home/$USERNAME/bin/restyle-path
RUN chown -R $USERNAME:$USERNAME /home/$USERNAME
RUN echo "PATH=/home/$USERNAME/bin:${PATH}" >> /home/$USERNAME/.bashrc

# $USERNAME needs to own the esp-idf and tools for the examples to build
RUN chown -R $USERNAME:$USERNAME /opt/espressif/esp-idf
RUN chown -R $USERNAME:$USERNAME /opt/espressif/tools

# $USERNAME needs to own west configuration to build nRF Connect examples
RUN chown -R $USERNAME:$USERNAME /opt/NordicSemiconductor/nrfconnect/

# allow read/write access to header and libraries
RUN chown -R $USERNAME:$USERNAME /opt/ubuntu-21.04-aarch64-sysroot/usr/

# allow licenses to be accepted
RUN chown -R $USERNAME:$USERNAME /opt/android/sdk

# AmebaD requires access to change build_info.h
RUN chown -R $USERNAME:$USERNAME /opt/ameba/ambd_sdk_with_chip_non_NDA/

# NXP uses a patch_sdk script to change SDK files
RUN chown -R $USERNAME:$USERNAME /opt/sdk/sdks/

RUN chown -R $USERNAME:$USERNAME /opt/fsl-imx-xwayland/5.15-kirkstone/

# Add access to openocd for VSCode debugging
RUN chown -R $USERNAME:$USERNAME /opt/openocd

# Fix Tizen SDK paths for new user
RUN sed -i '/^TIZEN_SDK_DATA_PATH/d' $TIZEN_SDK_ROOT/sdk.info \
    && echo TIZEN_SDK_DATA_PATH=/home/$USERNAME/tizen-sdk-data >> $TIZEN_SDK_ROOT/sdk.info \
    && ln -sf /home/$USERNAME/.tizen-cli-config $TIZEN_SDK_ROOT/tools/.tizen-cli-config \
    && : # last line

ENV TIZEN_ROOTFS /tizen_rootfs
