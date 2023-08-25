#!/bin/bash

cd ./third_party/silabs/matter_support/
export COMMIT_ID="$(git rev-parse HEAD)"
git fetch origin
git checkout origin/main
git merge-base --is-ancestor "$COMMIT_ID" HEAD

if [[ $? -eq 0 ]]; then
    git checkout "$COMMIT_ID"
    exit 0
else
    echo "THIS COMMIT : $COMMIT_ID IS OUT OF TREE!!!!!"
    echo "Please use a commit that is present on the main branch only."
    exit 1
fi
