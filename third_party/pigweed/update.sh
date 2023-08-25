#!/usr/bin/env bash
# Update the pigweed submodule and the CHIP CIPD manifest.

# Update the submodule.
cd "$(dirname "${BASH_SOURCE[0]}")/repo"
git fetch origin master
git checkout origin/master

# Copy the CIPD manifest but change the Python line so we don't use CIPD
# Python on Linux.
cat pw_env_setup/py/pw_env_setup/cipd_setup/pigweed.json |
    perl -pe 's[(infra/3pp/tools/cpython3/\$\{os)\}][$1=mac}];' > \
        ../../../scripts/pigweed.json

# Reformat the CIPD manifest.
cd ../../..
./scripts/helpers/restyle-diff.sh
