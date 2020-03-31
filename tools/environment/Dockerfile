FROM ubuntu:bionic

VOLUME /home/developer/connectedhomeip

RUN apt-get update
RUN apt-get -fy install git vim emacs \
        make automake g++ libtool llvm-9-dev llvm-9-tools llvm-9 llvm-9-runtime \
        curl \
        wget \
        jq \
        clang-9 \
        clang-format-9 \
        clang-tidy-9 \
        lcov \
        shellcheck

RUN useradd -ms /bin/bash developer
RUN mkdir -p /home/developer/connectedhomeip
USER developer
WORKDIR /home/developer

