FROM debian:buster

ENV DEBIAN_FRONTEND=noninteractive

SHELL ["/bin/bash", "-c"]

ARG ARCH="armhf"

ARG TARGET_DEPS="libavahi-client-dev libssl-dev libsystemd-dev libwrap0-dev uthash-dev \
  libsqlite3-dev libedit-dev libyaml-cpp0.6 libmosquitto-dev\
  libreadline-dev libncurses5-dev libncursesw5-dev libyaml-cpp-dev \
  libboost-atomic-dev libboost-chrono-dev libboost-date-time-dev \
  libboost-filesystem-dev libboost-regex-dev libboost-program-options-dev \
  libboost-serialization-dev libboost-system-dev libboost-thread-dev \
  libboost-log-dev procps libmbedtls-dev libglib2.0-dev"

ARG HOST_DEPS="ninja-build git git-lfs curl pkg-config \
  ruby ruby-dev lcov \
  gcc g++ gdb-multiarch \
  build-essential \
  clang nlohmann-json3-dev \
  libdbus-glib-1-dev libssl-dev libgirepository1.0-dev"

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
  for arch in ${ARCH} ; do echo $dep:$arch ; done ; \
  done) \
  $(for arch in ${ARCH} ; do echo crossbuild-essential-${arch} ; done)\
  && rm -rf /var/lib/apt/lists/*

# Install python 3.9
RUN curl -L https://repo.anaconda.com/miniconda/Miniconda3-py39_4.12.0-Linux-x86_64.sh --output /tmp/Miniconda.sh \
  && cd /tmp \
  && echo 78f39f9bae971ec1ae7969f0516017f2413f17796670f7040725dd83fcff5689 Miniconda.sh > /tmp/Miniconda.sh.sha256 \
  && sha256sum -c /tmp/Miniconda.sh.sha256 \
  && bash /tmp/Miniconda.sh -b -p /usr/local/miniconda \
  && /usr/local/miniconda/condabin/conda init bash \
  && ln -sf /usr/local/miniconda/bin/python3.9 /usr/bin/python3 \
  && ln -sf /usr/local/miniconda/bin/pip3 /usr/bin/pip3

# Install gcvor
RUN pip3 install gcovr==5.2

# Rust Version to install
ENV RUST_VERSION=1.64.0
# Rust and Cargo home directories
ENV RUSTUP_HOME=/opt/rustup-home
ENV CARGO_HOME=/opt/cargo-home

# Install Rust and Cargo
RUN curl https://sh.rustup.rs -sSf --output /tmp/sh.rustup.rs \
  && cd /tmp && chmod +x sh.rustup.rs \
  && if [[ $ARCH == *"armhf"* ]]; then export RUST_TRIPLES="$RUST_TRIPLES armv7-unknown-linux-gnueabihf"; fi \
  && if [[ $ARCH == *"amd64"* ]]; then export RUST_TRIPLES="$RUST_TRIPLES x86_64-unknown-linux-gnu"; fi \
  && ./sh.rustup.rs -y --profile minimal --target ${RUST_TRIPLES} --default-toolchain ${RUST_VERSION} \
  && rm /tmp/sh.rustup.rs \
  && /opt/cargo-home/bin/cargo install cargo2junit \
  && /opt/cargo-home/bin/cargo install cargo-deb --git https://github.com/kornelski/cargo-deb.git --branch main \
  && chmod -R a+rw ${RUSTUP_HOME} ${CARGO_HOME} \
  && find ${RUSTUP_HOME} ${CARGO_HOME} -type d -exec chmod a+x {} \;
ENV PATH="${CARGO_HOME}/bin:${PATH}"

# Install cmake 1.19.2
RUN curl -L https://github.com/Kitware/CMake/releases/download/v3.19.2/cmake-3.19.2-Linux-x86_64.sh --output /tmp/cmake-3.19.2-Linux-x86_64.sh \
  && cd /tmp \
  && echo 2fc84c1bd5a5fa8850426905a76147fbf897cf67ef324b009bcdb7eceafa9662 cmake-3.19.2-Linux-x86_64.sh > /tmp/cmake-3.19.2-Linux-x86_64.sh.sha256 \
  && sha256sum -c cmake-3.19.2-Linux-x86_64.sh.sha256 \
  && chmod +x /tmp/cmake*.sh \
  && /tmp/cmake*.sh --prefix=/usr/local --skip-license \
  && rm /tmp/cmake*