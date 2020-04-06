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
    pkg-config \
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
