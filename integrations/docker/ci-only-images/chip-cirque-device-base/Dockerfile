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
  avahi-daemon=0.7-4ubuntu7.1 \
  avahi-utils=0.7-4ubuntu7.1 \
  ca-certificates=20211016~20.04.1 \
  dhcpcd5=7.1.0-2build1 \
  gdb=9.2-0ubuntu1~20.04.1 \
  git=1:2.25.1-1ubuntu3.5 \
  iproute2=5.5.0-1ubuntu1 \
  libavahi-client3=0.7-4ubuntu7.1 \
  libcairo2-dev=1.16.0-4ubuntu1 \
  libdbus-1-dev=1.12.16-2ubuntu2.2 \
  libgif-dev=5.1.9-1 \
  libgirepository1.0-dev=1.64.1-1~ubuntu20.04.1 \
  libglib2.0-dev=2.64.6-1~ubuntu20.04.4 \
  libjpeg-dev=8c-2ubuntu8 \
  psmisc=23.3-1 \
  python3-dev=3.8.2-0ubuntu2 \
  python3-pip=20.0.2-5ubuntu1.6 \
  python3=3.8.2-0ubuntu2 \
  sudo=1.8.31-1ubuntu1.2 \
  wireless-tools=30~pre9-13ubuntu1 \
  wpasupplicant=2:2.9-1ubuntu4.3 \
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
