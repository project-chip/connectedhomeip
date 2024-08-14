ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

RUN apt update

RUN apt install picocom curl -y
RUN apt install netcat-traditional -y

COPY setup.sh /tmp

RUN cd /tmp \
    && bash setup.sh /opt/bouffalolab_sdk \
    && rm setup.sh \
    && : # last line

ENV BOUFFALOLAB_SDK_ROOT=/opt/bouffalolab_sdk
