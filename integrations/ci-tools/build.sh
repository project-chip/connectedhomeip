#!/usr/bin/env bash

#
# In support of Travis CI which communicates via environment variables,
#  this script supports the following:
#
#  TASK    a mnemonic for a supported build/test CI task to be mapped
#            to a set of bash commands
#  IMAGE   the location (under integrations/docker/image) of the desired
#            run.sh
#

me=$(basename "$0")

die() {
  echo "$me: *** ERROR: " "${*}"
  exit 1
}

bootstrap='if [[ ! -f build/default/config.status ]]; then mkdir -p build/default; (cd build/default && ../../bootstrap-configure --enable-debug --enable-coverage); else ./bootstrap -w make; fi'

docker_run_bash_command() {
  integrations/docker/images/"${IMAGE:-chip-build}"/run.sh bash -c "$1"
}

set -x

case "$TASK" in

  # You can add more tasks here, the top one shows an example of running a build inside our build container
  self-test)
    docker_run_bash_command 'echo looks ok to me && echo compound commands look good'
    ;;

  build-ubuntu-linux)
    docker_run_bash_command "$bootstrap"
    docker_run_bash_command 'make V=1 -C build/default'
    ;;

  build-nrf-example-lock-app)
    docker_run_bash_command 'make VERBOSE=1 -C examples/lock-app/nrf5'
    ;;

  build-distribution-check)
    docker_run_bash_command "$bootstrap"
    docker_run_bash_command 'make V=1 -C build/default'
    ;;

  run-unit-and-functional-tests)
    docker_run_bash_command "$bootstrap"
    docker_run_bash_command 'make V=1 -C build/default distcheck'
    ;;

  run-code-coverage)
    docker_run_bash_command "$bootstrap"
    docker_run_bash_command 'make V=1 -C build/default coverage'
    docker_run_bash_command 'bash <(curl -s https://codecov.io/bash)'
    ;;

  run-crypto-tests)
    docker_run_bash_command "$bootstrap"
    docker_run_bash_command 'make V=1 -C build/default/src/crypto check'
    ;;

  run-setup-payload-tests)
    docker_run_bash_command "$bootstrap"
    docker_run_bash_command 'make V=1 -C build/default/src/setup_payload check'
    ;;

  *)
    die "Unknown task: $TASK."
    ;;

esac
