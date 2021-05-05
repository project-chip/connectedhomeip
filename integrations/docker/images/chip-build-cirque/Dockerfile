ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
# Bazel
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
       curl=7.68.0-1ubuntu2.5 gnupg=2.2.19-3ubuntu2.1 \
    && curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor > /etc/apt/trusted.gpg.d/bazel.gpg \
    && echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
       bazel=4.0.0 \
    && rm -rf /var/lib/apt/lists/ \
    && : # aids diffs

# Docker
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
       curl=7.68.0-1ubuntu2.5 gnupg-agent=2.2.19-3ubuntu2.1 apt-transport-https=2.0.5 ca-certificates=20210119~20.04.1 \
       software-properties-common=0.98.9.4 \
    && curl -fsSL https://download.docker.com/linux/ubuntu/gpg | apt-key add - \
    && add-apt-repository \
       "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable" \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
       docker-ce=5:20.10.6~3-0~ubuntu-focal docker-ce-cli=5:20.10.6~3-0~ubuntu-focal containerd.io=1.4.4-1 \
    && rm -rf /var/lib/apt/lists/ \
    && : # aids diffs

# Other Cirque prereqs
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
       sudo=1.8.31-1ubuntu1.2 socat=1.7.3.3-2 psmisc=23.3-1 tigervnc-standalone-server=1.10.1+dfsg-3 tigervnc-viewer=1.10.1+dfsg-3 \
       python3-pip=20.0.2-5ubuntu1.3 python3-venv=3.8.2-0ubuntu2 python3-setuptools=45.2.0-1 libdbus-glib-1-dev=0.110-5fakssync1 \
       uuid-runtime=2.34-0.1ubuntu9.1 libgirepository1.0-dev=1.64.1-1~ubuntu20.04.1 \
    && rm -rf /var/lib/apt/lists/ \
    && : # aids diffs

COPY requirements_nogrpc.txt /requirements.txt

RUN set -x \
    && pip3 install --no-cache-dir -r requirements.txt \
    && : # aids diffs
