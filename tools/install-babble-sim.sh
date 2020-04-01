#!/usr/bin/env bash

mkdir -p ${HOME}/bsim && cd ${HOME}/bsim
curl https://storage.googleapis.com/git-repo-downloads/repo > ./repo  && chmod a+x ./repo
./repo init -u https://github.com/BabbleSim/manifest.git -m everything.xml -b master
./repo sync
make everything -j 8

echo -e
echo "**Define the following two environment variables to point to your BabbleSim installation**"
echo "export BSIM_OUT_PATH=${HOME}/bsim/"
echo "export BSIM_COMPONENTS_PATH=${HOME}/bsim/components/"