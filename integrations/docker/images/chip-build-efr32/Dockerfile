ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# GNU ARM Embedded toolchain, cross compiler for various platform builds
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    gcc-arm-none-eabi=15:9-2019-q4-0ubuntu1 \
    binutils-arm-none-eabi=2.34-4ubuntu1+13ubuntu1 \
    git-lfs \
    openjdk-11-jdk \
    ccache=3.7.7-1 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line


#Clone Gecko SDK 4.2.0 (d4854d2)
RUN git clone --depth=1 --branch=v4.2.0 https://github.com/SiliconLabs/gecko_sdk.git

ENV GSDK_ROOT=/gecko_sdk/

# SLC-cli install
RUN wget https://www.silabs.com/documents/login/software/slc_cli_linux.zip && \
    unzip ./slc_cli_linux.zip -d ./ && \
    rm ./slc_cli_linux.zip && \
    cd ./slc_cli && \
    pip3 install -r ./requirements.txt

ENV PATH="${PATH}:/slc_cli/"

RUN slc configuration  --sdk="$GSDK_ROOT" && \
    slc signature trust --sdk "$GSDK_ROOT"


RUN pip3 install lark jinja2 stringcase
