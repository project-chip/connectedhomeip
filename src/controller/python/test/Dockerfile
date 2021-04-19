
FROM ubuntu:focal

ENV DEBIAN_FRONTEND noninteractive

COPY /out/debug/controller/python/chip-0.0-cp37-abi3-linux_x86_64.whl /whl/
COPY /test/entrypoint.sh /entrypoint.sh
COPY /test/mobile-device-test.py /usr/bin/

RUN apt-get update && \
    apt-get install --no-install-recommends -y ca-certificates libglib2.0-dev avahi-daemon libavahi-client3 \
    python3 python3-dev python3-pip libcairo2-dev libdbus-1-dev libgirepository1.0-dev \
    libjpeg-dev libgif-dev gdb && \
    pip3 install /whl/chip-0.0-cp37-abi3-linux_x86_64.whl

ENTRYPOINT ["/entrypoint.sh"]
