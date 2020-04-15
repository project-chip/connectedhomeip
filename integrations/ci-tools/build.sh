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
  echo $1 >>./build/scripts/build_command.sh
}

docker_run_bash_command() {
  ./integrations/docker/images/chip-build/run.sh "build/scripts/build_command.sh"
}

case "$TASK" in

  build-linux)
    write_bash_template
    write_bash_command './bootstrap && mkdir -p build/default && (cd build/default && ../../configure --enable-coverage) && make -C build/default "${@:-distcheck}" && git clean -Xdf && (cd examples/lock-app/nrf5 && make)'
    docker_run_bash_command
    ;;

  preflight-shell-check)
    ./integrations/docker/images/chip-build/run.sh "integrations/shellcheck_tree.sh"
    ;;

  *)
    die "Unknown task: $TASK."
    ;;

esac
