#!/bin/bash

die() {
  echo " *** ERROR: " "${*}"
  exit 1
}

write_bash_template() {
  echo '#!/bin/bash
    ' >./build/scripts/build_command.sh
  chmod +x ./build/scripts/build_command.sh
}

write_bash_command() {
  echo "$1" >>./build/scripts/build_command.sh
}

docker_run_bash_command() {
  ./integrations/docker/images/chip-build/run.sh "build/scripts/build_command.sh"
}

case "$TASK" in

  # You can add more tasks here, the top one shows an example of running a build inside our build container
  build-linux)
    write_bash_template
    write_bash_command './bootstrap && mkdir -p build/default && (cd build/default && ../../configure --enable-coverage) && make -C build/default "${@:-distcheck}" && git clean -Xdf && (cd examples/lock-app/nrf5 && make)'
    docker_run_bash_command
    ;;

  build-main)
    write_bash_template
    write_bash_command './bootstrap && make -C build/default'
    docker_run_bash_command
    ;;

  build-distribution-check)
    write_bash_template
    write_bash_command './bootstrap && make -C build/default distcheck'
    docker_run_bash_command
    ;;

  run-unit-and-functional-tests)
    write_bash_template
    write_bash_command './bootstrap && make -C build/default check'
    docker_run_bash_command
    ;;

  run-crypto-tests)
    write_bash_template
    write_bash_command './bootstrap && make -C build/default/src/crypto check'
    docker_run_bash_command
    ;;

  run-setup-payload-tests)
    write_bash_template
    write_bash_command './bootstrap && make -C build/default/src/setup_payload check'
    docker_run_bash_command
    ;;

  *)
    die "Unknown task: $TASK."
    ;;

esac
