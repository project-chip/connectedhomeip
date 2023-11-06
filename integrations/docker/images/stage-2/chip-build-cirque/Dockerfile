ARG VERSION=latest
FROM ghcr.io/project-chip/chip-build:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

# Bazel
RUN set -x \
   && apt-get update \
   && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
   curl gnupg \
   && curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor > bazel.gpg \
   && mv bazel.gpg /etc/apt/trusted.gpg.d/ \
   && echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list \
   && apt-get update \
   && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
   bazel \
   && : # aids diffs

# Docker
RUN set -x \
   && apt-get update \
   && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
   curl gnupg-agent apt-transport-https ca-certificates \
   software-properties-common \
   && curl -fsSL https://download.docker.com/linux/ubuntu/gpg | apt-key add - \
   && python3.8 `which add-apt-repository` \
   "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable" \
   && apt-get update \
   && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
   docker-ce docker-ce-cli containerd.io \
   && : # aids diffs

# Other Cirque prereqs
RUN set -x \
   && apt-get update \
   && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
   sudo socat psmisc tigervnc-standalone-server xorg xauth \
   python3-pip python3-venv libdbus-glib-1-dev \
   uuid-runtime libgirepository1.0-dev \
   && : # aids diffs

COPY requirements_nogrpc.txt /requirements.txt

RUN set -x \
   && pip3 install -r requirements.txt \
   && xinit -- /usr/bin/Xvnc \
   && : # aids diffs
