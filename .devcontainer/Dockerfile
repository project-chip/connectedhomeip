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
FROM ghcr.io/project-chip/chip-build-vscode:${BUILD_VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

# This Dockerfile contains things useful for an interactive development environment
ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID
ENV LANG en_US.utf8

# these are installed for terminal/dev convenience.  If more tooling for build is required, please
#  add them to chip-build (in integrations/docker/images/chip-build)
RUN apt-get update \
    && apt-get install -y locales \
    && localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8 \
    && apt-get -fy install git vim emacs sudo \
    apt-utils dialog zsh \
    iproute2 procps lsb-release \
    bash-completion \
    build-essential cmake cppcheck valgrind \
    wget curl telnet \
    docker.io \
    iputils-ping net-tools \
    libncurses5 \
    libncursesw5 \
    libpython2.7 \
    && :

RUN groupadd -g $USER_GID $USERNAME \
    && useradd -s /bin/bash -u $USER_UID -g $USER_GID -G docker,sudo -m $USERNAME \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME \
    && :

RUN curl https://raw.githubusercontent.com/restyled-io/restyler/master/bin/restyle-path -o /usr/local/bin/restyle-path \
    && chmod +x /usr/local/bin/restyle-path \
    && :

RUN mkdir -p /opt/sdk/sdks/ \
    && chown -R $USERNAME:$USERNAME \
    /opt/sdk/sdks/ `# NXP uses a patch_sdk script to change SDK files` \
    /opt/NordicSemiconductor/nrfconnect/ `# $USERNAME needs to own west configuration to build nRF Connect examples` \
    $IDF_PATH `# $USERNAME needs to own the esp-idf and tools for the examples to build` \
    $IDF_TOOLS_PATH \
    $SYSROOT_AARCH64 `# allow read/write access to header and libraries` \
    $ANDROID_HOME `# allow licenses to be accepted` \
    $AMEBA_PATH `# AmebaD requires access to change build_info.h` \
    $IMX_SDK_ROOT \
    && :

# Fix Tizen SDK paths for new user
RUN sed -i '/^TIZEN_SDK_DATA_PATH/d' $TIZEN_SDK_ROOT/sdk.info \
    && echo TIZEN_SDK_DATA_PATH=/home/$USERNAME/tizen-sdk-data >> $TIZEN_SDK_ROOT/sdk.info \
    && ln -sf /home/$USERNAME/.tizen-cli-config $TIZEN_SDK_ROOT/tools/.tizen-cli-config \
    && : # last line

ENV TIZEN_ROOTFS /tizen_rootfs

# Fast Model GDB plugins path for debugging support
ENV FAST_MODEL_PLUGINS_PATH /opt/FastModelsPortfolio_11.16/plugins/Linux64_GCC-9.3
