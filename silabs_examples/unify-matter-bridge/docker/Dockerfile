ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# Rust Version to install
ENV RUST_VERSION=1.60.0
# Rust and Cargo home directories
ENV RUSTUP_HOME=/opt/rustup-home
ENV CARGO_HOME=/opt/cargo-home
# Install Rust and Cargo
RUN curl https://sh.rustup.rs -sSf --output /tmp/sh.rustup.rs \
  && cd /tmp && chmod +x sh.rustup.rs \
  && ./sh.rustup.rs -y --profile minimal --default-toolchain ${RUST_VERSION}\
  && rm /tmp/sh.rustup.rs \
  && /opt/cargo-home/bin/cargo install cargo2junit \
  # remove the main branch reference once the maintainer tagged 81d73b4
  && /opt/cargo-home/bin/cargo install cargo-deb --git https://github.com/kornelski/cargo-deb.git --branch main \
  && chmod -R a+rw ${RUSTUP_HOME} ${CARGO_HOME} \
  && find ${RUSTUP_HOME} ${CARGO_HOME} -type d -exec chmod a+x {} \;
ENV PATH="${CARGO_HOME}/bin:${PATH}"

#Target dependencies
RUN apt update && \
  apt install -y \
  libsqlite3-dev libedit-dev libyaml-cpp0.6 libmosquitto-dev\
  libyaml-cpp-dev \
  libboost-atomic-dev libboost-chrono-dev libboost-date-time-dev \
  libboost-filesystem-dev libboost-regex-dev libboost-program-options-dev \
  libboost-serialization-dev libboost-system-dev libboost-thread-dev \
  libboost-log-dev nlohmann-json3-dev

#Build host dependencies
RUN ln -s /usr/bin/python3 /usr/bin/python
RUN apt install -y ruby ruby-dev 
