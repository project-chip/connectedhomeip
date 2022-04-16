#!/bin/bash

local_chip_commit=$(cat $LOCAL_COMMIT_CACHE_FILE)
local_dockerfile_commit=$ZAP_COMMIT
remote_zap_commit=$(cat $REMOTE_ZAP_CACHE_FILE)
remote_dockerfile_commit=$(cat $REMOTE_DOCKERFILE_CACHE_FILE)

echo 'local_chip_commit' $local_chip_commit
echo 'local_dockerfile_commit' $local_dockerfile_commit
echo 'remote_zap_commit' $remote_zap_commit
echo 'remote_dockerfile_commit' $remote_dockerfile_commit

if [[ $remote_zap_commit != $local_dockerfile_commit ]] || [[ $remote_dockerfile_commit != $local_dockerfile_commit ]]; then
    echo 'This build is verifying against origin ' $CHIP_ORIGIN
    echo 'Origin has the following at commit ' $local_chip_commit
    echo '  zap submodule at ' $remote_zap_commit
    echo '  Dockerfile at ' $remote_dockerfile_commit
    echo 'Origin is not in sync with this build specifying' $local_dockerfile_commit
    echo 'If the zap commit requested for the build is ahead of the submodule in origin:'
    echo '  please update the submodule in origin and checkout.'
    echo 'If the zap commit requested in the build is behind the submodule in origin:'
    echo '  please update this dockerfile to match the submodule and push to origin before building.'
else
    echo 'Origin matches build'
    exit 0
fi
# Fail closed!
exit 1
