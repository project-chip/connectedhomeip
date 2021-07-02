FROM ubuntu:focal

ARG OT_BR_POSIX_CHECKOUT=master

ENV DEBIAN_FRONTEND noninteractive
ENV PLATFORM ubuntu
ENV DOCKER 1
ENV OT_BR_POSIX_CHECKOUT=${OT_BR_POSIX_CHECKOUT}

RUN mkdir /app

WORKDIR /app

# TODO: Use multi stage build for smaller image size.

RUN apt-get update \
  && apt-get install --no-install-recommends -y sudo git ca-certificates psmisc dhcpcd5 wpasupplicant wireless-tools \
                                                gdb python3 python3-pip libcairo2-dev libjpeg-dev libgif-dev python3-dev \
  && apt-get install -y avahi-daemon libavahi-client3 avahi-utils iproute2 libglib2.0-dev libgirepository1.0-dev libdbus-1-dev gdb \
  && ln -fs /usr/share/zoneinfo/UTC /etc/localtime \
  && git clone https://github.com/openthread/ot-br-posix . \
  && git checkout $OT_BR_POSIX_CHECKOUT \
  && git submodule update --init --depth=1 \
  && ./script/bootstrap \
  && ./script/setup \
  && chmod 644 /etc/bind/named.conf.options \
  && apt-get purge -y --auto-remove -o APT::AutoRemove::RecommendsImportant=false git psmisc ninja-build cmake wget \
                          libreadline-dev libncurses-dev libcpputest-dev libavahi-common-dev \
                          libavahi-client-dev libboost-dev libboost-filesystem-dev libboost-system-dev libjsoncpp-dev \
                          libnetfilter-queue-dev \
  && apt-get purge -y --auto-remove -o APT::AutoRemove::RecommendsImportant=false cmake cpputest doxygen \
  && mkdir -p /etc/wpa_supplicant \
  && echo "ctrl_interface=/run/wpa_supplicant" >> /etc/wpa_supplicant/wpa_supplicant.conf \
  && echo "update_config=1" >> /etc/wpa_supplicant/wpa_supplicant.conf \
  && rm -rf /var/lib/apt/lists/*

ENTRYPOINT service dbus start && \
           otbr-agent -I wpan0 spinel+hdlc+uart:///dev/ttyUSB0 || sleep infinity

EXPOSE 80
