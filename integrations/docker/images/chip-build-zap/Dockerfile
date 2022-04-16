ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

ENV ZAP_COMMIT=7ab717d08dfe9b0ba9de907fc7c6eb6549c86bf7

ENV CHIP_ORIGIN=https://github.com/abozowski/connectedhomeip
ENV ZAP_ORIGIN=https://github.com/project-chip/zap.git

RUN echo "Must be built in a CHIP repo with the included build script and on a commit found in $CHIP_ORIGIN"

ENV LOCAL_COMMIT_CACHE_FILE=/local_chip_commit.txt
ENV REMOTE_ZAP_CACHE_FILE=/remote_zap_commit.txt
ENV REMOTE_DOCKERFILE_CACHE_FILE=/remote_dockerfile_commit.txt
ENV VERIFY_SCRIPT=zapverify.sh

# Check zap in chip origin
RUN echo 'If you receive a "file not found in build context" error, ensure you are building with the included build.sh to populate the build context'
COPY context.txt $LOCAL_COMMIT_CACHE_FILE
WORKDIR /tmp
RUN git clone $CHIP_ORIGIN
WORKDIR ./connectedhomeip
RUN echo 'If you receive the error "fatal: reference is not a tree:", you are on a commit not found in' $CHIP_ORIGIN
RUN git checkout $(cat $LOCAL_COMMIT_CACHE_FILE)
RUN git ls-tree master third_party/zap/repo | awk '{print $3}'> $REMOTE_ZAP_CACHE_FILE
RUN cat $REMOTE_ZAP_CACHE_FILE

# Check the dockerfile in chip origin
WORKDIR ./integrations/docker/images/chip-build-zap
RUN dfc=$(cat Dockerfile | grep -m 1 'ENV ZAP_COMMIT'); echo ${dfc#*=} > $REMOTE_DOCKERFILE_CACHE_FILE;
RUN cat $REMOTE_DOCKERFILE_CACHE_FILE

# Verify local against origin
WORKDIR /
COPY $VERIFY_SCRIPT $VERIFY_SCRIPT
RUN chmod +x $VERIFY_SCRIPT
RUN ./$VERIFY_SCRIPT
RUN rm -R /tmp/connectedhomeip

# Build zap at the correct commit
WORKDIR /opt
RUN git clone $ZAP_ORIGIN
WORKDIR ./zap
RUN git checkout $ZAP_COMMIT
RUN npm ci
