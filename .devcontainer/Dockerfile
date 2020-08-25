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
    wget curl telnet bsdtar \
    docker.io \
    iputils-ping net-tools
RUN groupadd -g $USER_GID $USERNAME
RUN useradd -s /bin/bash -u $USER_UID -g $USER_GID -G docker -m $USERNAME
RUN echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME
RUN chmod 0440 /etc/sudoers.d/$USERNAME

RUN ln -s /usr/bin/clang-format-9 /usr/local/bin/clang-format

RUN mkdir -p /var/downloads
RUN cd /var/downloads
RUN curl -JL https://github.com/microsoft/vscode-cpptools/releases/download/0.27.0/cpptools-linux.vsix | bsdtar -xvf - extension
RUN mkdir -p /home/$USERNAME/.vscode-server/extensions
RUN mv extension /home/$USERNAME/.vscode-server/extensions/ms-vscode.cpptools-0.27.0
RUN mkdir -p /home/$USERNAME/bin
RUN curl https://raw.githubusercontent.com/restyled-io/restyler/master/bin/restyle-path -o /home/$USERNAME/bin/restyle-path
RUN chmod +x /home/$USERNAME/bin/restyle-path
RUN chown -R $USERNAME:$USERNAME /home/$USERNAME
RUN echo "PATH=/home/$USERNAME/bin:${PATH}" >> /home/$USERNAME/.bashrc
# $USERNAME needs to own the esp-idf and tools for the examples to build
RUN chown -R $USERNAME:$USERNAME /var/esp-idf
RUN chown -R $USERNAME:$USERNAME /var/.espressif
