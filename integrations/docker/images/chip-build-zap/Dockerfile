ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

ENV ZAP_COMMIT=7ab717d08dfe9b0ba9de907fc7c6eb6549c86bf7

ENV ZAP_ORIGIN=https://github.com/project-chip/zap.git

WORKDIR /opt
RUN git clone $ZAP_ORIGIN
WORKDIR ./zap
RUN git checkout $ZAP_COMMIT
RUN npm ci
