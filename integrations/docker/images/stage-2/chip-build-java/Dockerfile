ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

# Ensure some java is installed
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
    openjdk-8-jdk \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

# Download and install kotlin compiler
RUN set -x \
    && cd /usr/lib \
    && wget -q https://github.com/JetBrains/kotlin/releases/download/v2.1.10/kotlin-compiler-2.1.10.zip \
    && unzip kotlin-compiler-*.zip \
    && rm kotlin-compiler-*.zip \
    && rm -f kotlinc/bin/*.bat \
    && : # last line

ENV PATH $PATH:/usr/lib/kotlinc/bin
ENV JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
