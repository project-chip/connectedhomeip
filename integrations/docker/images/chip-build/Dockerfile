# start with Ubuntu 18.04LTS
FROM ubuntu:bionic

RUN set -x \
    && apt-get update \
    && apt install \
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
    gcc-multilib \
    libfftw3-dev \
    libssl-dev \
    git-core \
    bash-completion

RUN mkdir -p /var/bsim
WORKDIR /var/bsim

RUN cd /var/bsim
RUN curl https://storage.googleapis.com/git-repo-downloads/repo > ./repo  && chmod a+x ./repo
RUN ./repo init -u https://github.com/BabbleSim/manifest.git -m everything.xml -b master
RUN ./repo sync
RUN make everything -j 8
