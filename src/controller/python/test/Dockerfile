
FROM ubuntu:focal

ENV DEBIAN_FRONTEND noninteractive

COPY /whl/python/*.whl /whl/
COPY entrypoint.sh /entrypoint.sh
COPY mobile-device-test.py /usr/bin/

RUN apt-get update && \
    apt-get install -y ca-certificates libglib2.0-dev avahi-daemon libavahi-client3 \
    python3 python3-dev python3-pip libcairo2-dev libdbus-1-dev libgirepository1.0-dev \
    libjpeg-dev libgif-dev && \
    pip3 install /whl/*.whl

ENTRYPOINT ["/entrypoint.sh"]
