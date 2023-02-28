ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# Download and install kotlin compiler
RUN set -x \
    && cd /usr/lib \
    && wget -q https://github.com/JetBrains/kotlin/releases/download/v1.3.31/kotlin-compiler-1.3.31.zip \
    && unzip kotlin-compiler-*.zip \
    && rm kotlin-compiler-*.zip \
    && rm -f kotlinc/bin/*.bat \
    && : # last line

ENV PATH $PATH:/usr/lib/kotlinc/bin
