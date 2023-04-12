FROM ubuntu:focal

ARG OT_BR_POSIX_CHECKOUT=master

ENV DEBIAN_FRONTEND noninteractive
ENV PLATFORM ubuntu
ENV DOCKER 1
ENV OT_BR_POSIX_CHECKOUT=${OT_BR_POSIX_CHECKOUT}

RUN mkdir /app

WORKDIR /app

RUN apt-cache policy

# TODO: Use multi stage build for smaller image size.
RUN apt-get update \
  && apt-get install --no-install-recommends -y \
  avahi-daemon \
  avahi-utils \
  ca-certificates \
  dhcpcd5 \
  gdb \
  git \
  iproute2 \
  libavahi-client3 \
  libcairo2-dev \
  libdbus-1-dev \
  libgif-dev \
  libgirepository1.0-dev \
  libglib2.0-dev \
  libjpeg-dev \
  psmisc \
  python3-dev \
  python3-pip \
  python3 \
  sudo \
  wireless-tools \
  wpasupplicant \
  && ln -fs /usr/share/zoneinfo/UTC /etc/localtime \
  && git clone https://github.com/openthread/ot-br-posix . \
  && git checkout $OT_BR_POSIX_CHECKOUT \
  && git submodule update --init --depth=1 \
  && ./script/bootstrap \
  && ./script/setup \
  && apt-get purge -y --auto-remove -o APT::AutoRemove::RecommendsImportant=false \
  git psmisc ninja-build cmake wget libreadline-dev libncurses-dev libcpputest-dev \
  libavahi-common-dev libavahi-client-dev libboost-dev libboost-filesystem-dev \
  libboost-system-dev libjsoncpp-dev libnetfilter-queue-dev cmake cpputest doxygen \
  && mkdir -p /etc/wpa_supplicant \
  && echo "ctrl_interface=/run/wpa_supplicant" >> /etc/wpa_supplicant/wpa_supplicant.conf \
  && echo "update_config=1" >> /etc/wpa_supplicant/wpa_supplicant.conf \
  && rm -rf /var/lib/apt/lists/* \
  && pip3 install --no-cache-dir click==8.0.3

COPY CHIPCirqueDaemon.py /bin/CHIPCirqueDaemon.py
COPY entrypoint.sh /opt/entrypoint.sh

WORKDIR /

ENTRYPOINT ["/opt/entrypoint.sh"]

EXPOSE 80
