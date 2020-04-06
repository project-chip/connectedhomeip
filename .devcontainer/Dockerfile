# start with Ubuntu 18.04LTS
FROM ubuntu:bionic

# base build and check tools and libraries layer
RUN set -x \
    && apt-get update \
    && apt-get install -fy \
    git \
    curl \
    jq \
    make \
    autoconf \
    automake \
    libtool \
    g++ \
    clang-9 \
    clang-format-9 \
    clang-tidy-9 \
    lcov \
    shellcheck \
    libssl-dev

# BabbleSim layer
RUN set -x \
    && apt-get install -fy \
    gcc-multilib \
    libfftw3-dev \
    && (mkdir -p /var/bsim \
    && cd /var/bsim \
    && curl https://storage.googleapis.com/git-repo-downloads/repo > ./repo  && chmod a+x ./repo \
    && ./repo init -u https://github.com/BabbleSim/manifest.git -m everything.xml -b v1.0.3 --depth=1 \
    && ./repo sync \
    && make everything -j 8)

# above is same as "FROM rwalkerapple/chip-build"

# development environment stuff, command line helpers, etc.
ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID
ENV LANG en_US.utf8

# these are installed for terminal/dev convenience.  If more tooling for build is required, please
#  add them to chip-build (in integrations/docker/images/chip-build)
RUN apt-get update
RUN apt-get install -y locales && localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8
RUN apt-get -fy install git vim emacs sudo \
    apt-utils dialog \
    iproute2 procps lsb-release \
    bash-completion \
    build-essential cmake cppcheck valgrind \
    wget

RUN groupadd --gid $USER_GID $USERNAME
RUN useradd -s /bin/bash --uid $USER_UID --gid $USER_GID -m $USERNAME
RUN echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME
RUN chmod 0440 /etc/sudoers.d/$USERNAME

RUN ln -s /usr/bin/clang-format-9 /usr/local/bin/clang-format
