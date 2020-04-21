#!/usr/bin/env bash

#
# In support of Travis CI which communicates via environment variables,
#  this script supports the following:
#
#  TASK    a mnemonic for a supported build/test CI task to be mapped
#            to a set of bash commands
#  IMAGE   the location (under integrations/docker/image) of the desired
#            run.sh
#  ENV     a comma-separated list of --env arguments to be passed to
#            docker run, e.g. "GITHUB_TOKEN,FOO=BAR" will convey
#            as "--env GITHUB_TOKEN --env FOO=BAR"
#

me=$(basename "$0")
here=$(cd "$(dirname "$0")" && pwd)

die() {
  echo "$me: *** ERROR: " "${*}"
  exit 1
}

# move to ToT, I don't work anywhere else
cd "$here/../.." || die 'ack!, where am I?!?'

bootstrap='
if [[ ! -f build/default/config.status ]]; then
   mkdir -p build/default;
   (cd build/default && ../../bootstrap-configure --enable-debug --enable-coverage);
else
   ./bootstrap -w make;
fi'

docker_run_command() {
  integrations/docker/images/"${IMAGE:-chip-build}"/run.sh "${ENV[@]}" -- "$@"
}

docker_run_bash_command() {
  docker_run_command bash -c "$1"
}

# convert ENV to an array of words: X,Y,Z => ( X Y Z )
read -r -a ENV <<<"${ENV//,/ }"
# convert ENV array to an array of args ( X Y ) => ( --env X --env Y )
read -r -a ENV <<<"${ENV[@]/#/--env }"

case "$TASK" in

  self-test)
    docker_run_bash_command 'echo looks ok to me && echo compound commands look good'
    ;;

  self-test-env)
    docker_run_command bash -c "echo run me with ENV=HI=THERE,; env | echo HI=$'$'HI"
    ;;

  # You can add more tasks here, the top one shows an example of running
  #  a build inside our build container
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
