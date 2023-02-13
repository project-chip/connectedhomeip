FROM debian:bullseye

ENV DEBIAN_FRONTEND=noninteractive

SHELL ["/bin/bash", "-c"]

WORKDIR /

ARG ARCH="arm64"

ARG TARGET_DEPS="libavahi-client-dev:${ARCH}=0.8-5* libssl-dev:${ARCH}=1.1.1n-0* libsystemd-dev:${ARCH}=247.3-7* libwrap0-dev:${ARCH}=7.6.q-31 uthash-dev:${ARCH}=2.1.0-1 \
  libsqlite3-dev:${ARCH}=3.34.1-3 libedit-dev:${ARCH}=3.1-20191231-2+b1 libyaml-cpp0.6:${ARCH}=0.6.3-9 libmosquitto-dev:${ARCH}=2.0.11-1\
  libreadline-dev:${ARCH}=8.1-1 libncurses5-dev:${ARCH}=6.2+20201114-2 libncursesw5-dev:${ARCH}=6.2+20201114-2 libyaml-cpp-dev:${ARCH}=0.6.3-9 \
  libboost-atomic-dev:${ARCH}=1.74.0.3 libboost-chrono-dev:${ARCH}=1.74.0.3 libboost-date-time-dev:${ARCH}=1.74.0.3 \
  libboost-filesystem-dev:${ARCH}=1.74.0.3 libboost-regex-dev:${ARCH}=1.74.0.3 libboost-program-options-dev:${ARCH}=1.74.0.3 \
  libboost-serialization-dev:${ARCH}=1.74.0.3 libboost-system-dev:${ARCH}=1.74.0.3 libboost-thread-dev:${ARCH}=1.74.0.3 \
  libboost-log-dev:${ARCH}=1.74.0.3 procps:${ARCH}=2:3.3.17-5 libmbedtls-dev:${ARCH}=2.16.9-0.1 libglib2.0-dev:${ARCH}=2.66.8-1"

ARG HOST_DEPS="ninja-build=1.10.1-1 git=1:2.30.2-1* git-lfs=2.13.2-1+b5 \
  curl=7.74.0-1.3* pkg-config=0.29.2-1 \
  ruby=1:2.7+2 ruby-dev=1:2.7+2 lcov=1.14-2 \
  gcc=4:10.2.1-1 g++=4:10.2.1-1 gdb-multiarch=10.1-1.7 \
  build-essential=12.9 \
  clang=1:11.0-51+nmu5 nlohmann-json3-dev=3.9.1-1 \
  libdbus-glib-1-dev=0.110-6 libssl-dev=1.1.1n-0* \
  libgirepository1.0-dev=1.66.1-1+b1"

# configure additional arches in dpkg/apt
RUN for arch in ${ARCH} ; do \
  dpkg --add-architecture $arch ; \
  echo $arch ; \
  done

RUN apt-get update \
  && apt-get install --no-install-recommends -y curl gnupg2 ca-certificates \
  && apt-get update \
  && apt-get install --no-install-recommends -y \
  ${HOST_DEPS} \
  $(for dep in ${TARGET_DEPS} ; do \
  for arch in ${ARCH} ; do echo $dep; done ; \
  done) \
  $(for arch in ${ARCH} ; do echo crossbuild-essential-${arch} ; done)\
  && rm -rf /var/lib/apt/lists/*

# Install python 3.9
RUN curl -L https://repo.anaconda.com/miniconda/Miniconda3-py39_4.12.0-Linux-x86_64.sh --output /tmp/Miniconda.sh \
  && echo 78f39f9bae971ec1ae7969f0516017f2413f17796670f7040725dd83fcff5689 /tmp/Miniconda.sh > /tmp/Miniconda.sh.sha256 \
  && sha256sum -c /tmp/Miniconda.sh.sha256 \
  && bash /tmp/Miniconda.sh -b -p /usr/local/miniconda \
  && /usr/local/miniconda/condabin/conda init bash \
  && ln -sf /usr/local/miniconda/bin/python3.9 /usr/bin/python3 \
  && ln -sf /usr/local/miniconda/bin/pip3 /usr/bin/pip3

# Install gcvor
RUN pip3 install --no-cache-dir gcovr==5.2

# Rust Version to install
ENV RUST_VERSION=1.64.0
# Rust and Cargo home directories
ENV RUSTUP_HOME=/opt/rustup-home
ENV CARGO_HOME=/opt/cargo-home

# Install Rust and Cargo
RUN curl https://sh.rustup.rs -sSf --output /tmp/sh.rustup.rs \
  && chmod +x /tmp/sh.rustup.rs \
  && if [[ $ARCH == *"armhf"* ]]; then export RUST_TRIPLES="$RUST_TRIPLES armv7-unknown-linux-gnueabihf"; fi \
  && if [[ $ARCH == *"amd64"* ]]; then export RUST_TRIPLES="$RUST_TRIPLES x86_64-unknown-linux-gnu"; fi \
  && if [[ $ARCH == *"arm64"* ]]; then export RUST_TRIPLES="$RUST_TRIPLES aarch64-unknown-linux-gnu"; fi \
  && ./tmp/sh.rustup.rs -y --profile minimal --target ${RUST_TRIPLES} --default-toolchain ${RUST_VERSION}\
  && rm /tmp/sh.rustup.rs \
  && /opt/cargo-home/bin/cargo install cargo2junit \
  && /opt/cargo-home/bin/cargo install --version 1.41.3 cargo-deb \
  && chmod -R a+rw ${RUSTUP_HOME} ${CARGO_HOME} \
  && find ${RUSTUP_HOME} ${CARGO_HOME} -type d -exec chmod a+x {} \;
ENV PATH="${CARGO_HOME}/bin:${PATH}"

# Install cmake 3.21.6
RUN curl -L https://github.com/Kitware/CMake/releases/download/v3.21.6/cmake-3.21.6-Linux-x86_64.sh --output /tmp/cmake-3.21.6-Linux-x86_64.sh \
  && echo d460a33c42f248388a8f2249659ad2f5eab6854bebaf4f57c1df49ded404e593  /tmp/cmake-3.21.6-Linux-x86_64.sh > /tmp/cmake-3.21.6-Linux-x86_64.sh.sha256 \
  && sha256sum -c /tmp/cmake-3.21.6-Linux-x86_64.sh.sha256 \
  && chmod +x /tmp/cmake-3.21.6-Linux-x86_64.sh \
  && /tmp/cmake-3.21.6-Linux-x86_64.sh --prefix=/usr/local --skip-license \
  && rm /tmp/cmake*