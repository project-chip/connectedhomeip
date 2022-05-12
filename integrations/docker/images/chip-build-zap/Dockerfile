ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

ENV ZAP_COMMIT=e9277b989f1d128bc881e88ecbf7a3353c841c39

ENV ZAP_ORIGIN=https://github.com/project-chip/zap.git

WORKDIR /opt
RUN git clone $ZAP_ORIGIN
WORKDIR ./zap
RUN git checkout $ZAP_COMMIT
RUN npm ci
