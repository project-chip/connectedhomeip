ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# Setup the ESP-IDF
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y python libgcrypt20-dev \
    && mkdir -p /opt/espressif \
    && cd /opt/espressif \
    && git clone --progress -b v4.3 https://github.com/espressif/esp-idf.git \
    && cd esp-idf \
    && git submodule update --init --progress \
    && IDF_TOOLS_PATH=/opt/espressif/tools ./install.sh \
    && : # last line

# BEGIN: PATCH BROKEN UPSTREAM
#
# After an upgrade in https://pypi.org/project/cryptography/#history
# ESP sdk starts failing when attempting to run:
#
#   /opt/espressif/tools/python_env/idf4.3_py3.9_env/bin/python \
#       /opt/espressif/esp-idf/components/mbedtls/esp_crt_bundle/gen_crt_bundle.py \
#       --input /opt/espressif/esp-idf/components/mbedtls/esp_crt_bundle/cacrt_all.pem -q
#
# Unfortunately cryptography is brought up as ">=2.1.4" from
# /opt/espressif/esp-idf/requirements.txt, so we get an incompatible version
# Code below reverts to a known working version.

RUN set -x \
    && /opt/espressif/tools/python_env/idf4.3_py3.9_env/bin/pip uninstall -y cryptography \
    && /opt/espressif/tools/python_env/idf4.3_py3.9_env/bin/pip install cryptography==3.4.8 \
    && : # last line
# END: PATCH BROKEN UPSTREAM

ENV IDF_PATH=/opt/espressif/esp-idf/
ENV IDF_TOOLS_PATH=/opt/espressif/tools
