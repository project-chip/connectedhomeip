#!/usr/bin/env bash

me=$(basename "$0")
here=$(cd "$(dirname "$0")" && pwd)

die() {
  echo "$me: *** ERROR: " "${*}"
  exit 1
}

# move to ToT, I don't work anywhere else
cd "$here/../.." || die 'ack!, where am I?!?'

#
# find and compile tests directories
#
readarray -t TESTDIRS < <(git ls-files \*/tests/Makefile.am)

#
# pull off the Makefile.am from the list
#
read -r -a TESTDIRS <<<"${TESTDIRS[@]%/*}"

# build the yml
TESTS_YML=""
for dir in "${TESTDIRS[@]}"
do
  RUN=$(base64<<<"make
-C
build/default/$dir
check-TESTS")
  TESTS_YML+="
        - stage: Tests
          name: \"check-TESTS in $dir\"
          env: TASK=run RUN=$RUN"

done

#
# .travis.yml template
#
cat<<EOF
### THIS FILE IS GENERATED, PLEASE DO NOT EDIT DIRECTLY
###  The generator lives at integrations/ci-tools/travis_yml.sh
language: minimal

branches:
    except:
        - /^restyled.*$/

services:
    - docker

script:
    - ./integrations/ci-tools/build.sh

# Note: To add new non-test job types, add it to the matrix below,
#  with a unique TASK varable, then update the accompanying build.sh
#  to do what you want during that task

jobs:
    include:
        - stage: Build
          name: "Build Ubuntu Linux Xenial LTS"
          env: TASK="build-ubuntu-linux"
        - stage: Build
          name: "Build NRF Example Lock App"
          env: TASK="build-nrf-example-lock-app"${TESTS_YML[@]}
        - stage: Deployment Checks
          name: "Run Code Coverage"
          env: TASK="run-code-coverage"
        - stage: Deployment Checks
          name: "Distribution Check"
          env: TASK="build-distribution-check"

EOF
