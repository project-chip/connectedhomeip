ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

ENV ZAP_COMMIT=1136effb0e0ee8cd1f6bfc730492db799f878cba

ENV ZAP_ORIGIN=https://github.com/project-chip/zap.git

WORKDIR /opt
RUN git clone $ZAP_ORIGIN
WORKDIR ./zap
RUN git checkout $ZAP_COMMIT
RUN npm ci
