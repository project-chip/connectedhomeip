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

bootstrap='scripts/build/bootstrap.sh'

bootstrap_mbedtls='scripts/build/bootstrap_mbedtls.sh'

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
    docker_run_bash_command 'scripts/build/default.sh'
    ;;

  build-nrf-example-lock-app)
    docker_run_bash_command 'scripts/examples/nrf_lock_app.sh'
    ;;

  build-distribution-check)
    docker_run_bash_command "$bootstrap"
    docker_run_bash_command 'scripts/build/distribution_check.sh'
    ;;

  run-unit-and-functional-tests)
    docker_run_bash_command "$bootstrap"
    docker_run_bash_command 'scripts/tests/all_tests.sh'
    ;;

  run-code-coverage)
    docker_run_bash_command "$bootstrap"
    docker_run_bash_command 'scripts/tools/codecoverage.sh'
    docker_run_bash_command 'bash <(curl -s https://codecov.io/bash)'
    ;;

  run-crypto-tests)
    docker_run_bash_command "$bootstrap"
    docker_run_bash_command 'scripts/tests/crypto_tests.sh'
    ;;

  run-setup-payload-tests)
    docker_run_bash_command "$bootstrap"
    docker_run_bash_command 'scripts/tests/setup_payload_tests.sh'
    ;;

  run-mbedtls-crypto-tests)
    docker_run_bash_command "$bootstrap_mbedtls"
    docker_run_bash_command 'scripts/tests/mbedtls_tests.sh'
    docker_run_bash_command 'scripts/tests/crypto_tests.sh'
    ;;

  *)
    die "Unknown task: $TASK."
    ;;

esac
