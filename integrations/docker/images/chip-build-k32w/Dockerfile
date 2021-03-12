ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# Setup the K32W SDK
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y wget unzip \
    && mkdir -p /opt/sdk \
    && cd /opt/sdk \
    && wget https://mcuxpresso.nxp.com/eclipse/sdk/2.6.2/plugins/com.nxp.mcuxpresso.sdk.sdk_2.x_k32w061dk6_2.6.2.201911251446.jar \
    && unzip com.nxp.mcuxpresso.sdk.sdk_2.x_k32w061dk6_2.6.2.201911251446.jar \
    && cd sdks \
    && unzip bea4b051e4c58da8d3bff93011a97d1d.zip \
    && : # last line

ENV K32W061_SDK_ROOT=/opt/sdk/sdks
