ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# Setup the K32W SDK
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y wget unzip \
    && rm -rf /var/lib/apt/lists/ \
    && mkdir -p /opt/sdk \
    && cd /opt/sdk \
    && wget https://mcuxpresso.nxp.com/eclipse/sdk/2.6.4/plugins/com.nxp.mcuxpresso.sdk.sdk_2.x_k32w061dk6_2.6.4.201911251446.jar \
    && unzip com.nxp.mcuxpresso.sdk.sdk_2.x_k32w061dk6_2.6.4.201911251446.jar \
    && rm -rf com.nxp.mcuxpresso.sdk.sdk_2.x_k32w061dk6_2.6.4.201911251446.jar \
    && cd sdks \
    && unzip 1190028246d9243d9a9e27ca783413a8.zip \
    && rm -rf 1190028246d9243d9a9e27ca783413a8.zip \
    && : # last line

ENV K32W061_SDK_ROOT=/opt/sdk/sdks
