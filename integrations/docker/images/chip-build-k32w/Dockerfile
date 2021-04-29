ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

WORKDIR /opt/sdk

# Setup the K32W SDK
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    wget=1.20.3-1ubuntu1 unzip=6.0-25ubuntu1 \
    && rm -rf /var/lib/apt/lists/ \
    && wget https://mcuxpresso.nxp.com/eclipse/sdk/2.6.3/plugins/com.nxp.mcuxpresso.sdk.sdk_2.x_k32w061dk6_2.6.3.201911251446.jar \
    && unzip com.nxp.mcuxpresso.sdk.sdk_2.x_k32w061dk6_2.6.3.201911251446.jar \
    && unzip sdks/5faab205f2663647c5c7ce05c382d2a8.zip -d sdks/ \
    && rm -f com.nxp.mcuxpresso.sdk.sdk_2.x_k32w061dk6_2.6.3.201911251446.jar \
    && rm -f sdk/5faab205f2663647c5c7ce05c382d2a8.zip \
    && : # last line

ENV K32W061_SDK_ROOT=/opt/sdk/sdks
