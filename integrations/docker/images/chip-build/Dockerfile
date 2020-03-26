# start with Ubuntu 18.04LTS
FROM ubuntu:bionic

RUN set -x \
 && apt-get update \
 && apt-get install -fy apt-transport-https curl gnupg2 \
 && curl -s https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
 && echo deb https://apt.llvm.org/bionic/ llvm-toolchain-bionic-9 main >> /etc/apt/sources.list.d/llvm-toolchain.list \
 && apt-get update \
 && apt install \
 && apt-get install -fy \
      git \
      make \
      autoconf \
      automake \
      libtool \
      curl \
      jq \
      clang-9 \
      clang-format-9 \
      clang-tidy-9 \
      lcov \
      shellcheck
