ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION} as build

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    wget=1.20.3-1ubuntu2 \
    unzip=6.0-25ubuntu1 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

WORKDIR /opt/sdk
# Setup the K32W SDK
RUN set -x \
    && wget -O /tmp/sdk.jar https://mcuxpresso.nxp.com/eclipse/sdk/2.6.4/plugins/com.nxp.mcuxpresso.sdk.sdk_2.x_k32w061dk6_2.6.4.201911251446.jar \
    && unzip /tmp/sdk.jar \
    && unzip sdks/1190028246d9243d9a9e27ca783413a8.zip -d sdks \
    && rm -rf sdks/1190028246d9243d9a9e27ca783413a8.zip \
    && : # last line

FROM connectedhomeip/chip-build:${VERSION}

COPY --from=build /opt/sdk/sdks/ /opt/sdk/sdks/

ENV NXP_K32W061_SDK_ROOT=/opt/sdk/sdks
