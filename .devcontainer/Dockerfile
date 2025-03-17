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
ENV LANG=en_US.utf8


# These are installed for terminal/dev convenience. If more tooling for build is required, please
# add them to chip-build (in integrations/docker/images/chip-build).
RUN apt-get update \
    && apt-get install -y locales \
    && localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8 \
    && apt-get -fy install vim emacs sudo \
    apt-utils dialog zsh \
    lsb-release \
    bash-completion \
    valgrind \
    docker.io \
    iputils-ping \
    icecc \
    && :

RUN (getent passwd $USER_UID && userdel -f $(getent passwd $USER_UID | cut -d: -f1) || true) \
    && (getent group $USER_GID && groupdel -f $(getent group $USER_GID | cut -d: -f1) || true) \
    && (getent passwd $USERNAME && userdel -f $USERNAME || true) \
    && (getent group $USERNAME && groupdel -f $USERNAME || true) \
    && groupadd -g $USER_GID $USERNAME \
    && useradd --no-log-init -s /bin/bash -u $USER_UID -g $USER_GID -G docker,sudo -m $USERNAME \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME \
    && :

RUN curl https://raw.githubusercontent.com/restyled-io/restyler/master/bin/restyle-path -o /usr/local/bin/restyle-path \
    && chmod +x /usr/local/bin/restyle-path \
    && :

RUN mkdir -p /opt/android/sdk \
    && chown -R $USERNAME:$USERNAME \
    /opt/android/sdk `# NXP uses a patch_sdk script to change SDK files` \
    $ANDROID_HOME \
    $IDF_TOOLS_PATH \
    && find $AMEBA_PATH -name "inc_lp" -print0 | xargs -0 chown -R $USERNAME:$USERNAME \
    && find $AMEBA_PATH -name "inc_hp" -print0 | xargs -0 chown -R $USERNAME:$USERNAME \
    && find $AMEBA_PATH -name "project_lp" -print0 | xargs -0 chown -R $USERNAME:$USERNAME \
    && find $AMEBA_PATH -name "project_hp" -print0 | xargs -0 chown -R $USERNAME:$USERNAME \
    && chmod -R +x \
    $ANDROID_HOME/cmdline-tools/10.0/bin `# sdkmanager for accepting licenses`\
    && chmod -R +w \
    $IDF_TOOLS_PATH \
    && find $AMEBA_PATH -name "inc_lp" -print0 | xargs -0 chmod -R +w \
    && find $AMEBA_PATH -name "inc_hp" -print0 | xargs -0 chmod -R +w \
    && find $AMEBA_PATH -name "project_lp" -print0 | xargs -0 chmod -R +w \
    && find $AMEBA_PATH -name "project_hp" -print0 | xargs -0 chmod -R +w \
    # Safe directory is preferred over chown.
    && git config --global --add safe.directory "*" \
    && :

# Fix Tizen SDK paths for new user
RUN sed -i '/^TIZEN_SDK_DATA_PATH/d' $TIZEN_SDK_ROOT/sdk.info \
    && echo TIZEN_SDK_DATA_PATH=/home/$USERNAME/tizen-sdk-data >> $TIZEN_SDK_ROOT/sdk.info \
    && ln -sf /home/$USERNAME/.tizen-cli-config $TIZEN_SDK_ROOT/tools/.tizen-cli-config \
    && : # last line

ENV TIZEN_ROOTFS=/tizen_rootfs

# Fast Model GDB plugins path for debugging support
ENV FAST_MODEL_PLUGINS_PATH=/opt/FastModelsPortfolio_11.16/plugins/Linux64_GCC-9.3

# Set up ccache as a pigweed command launcher when using the scripts/build/build_examples.py
# script. Also, set up icecc as the command prefix for ccache. Such setup allows to benefit
# from compilation caching and distributed compilation at the same time.
#
# NOTE: In order to use distributed compilation with icecc, one should run
#       "scripts/icecc.sh start" before starting the build.
ENV CHIP_PW_COMMAND_LAUNCHER=ccache
ENV CCACHE_PREFIX=icecc
ENV PATH=/usr/lib/ccache:$PATH
