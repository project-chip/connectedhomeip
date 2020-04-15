#!/bin/sh

die() {
  echo " *** ERROR: " "${*}"
  exit 1
}

echo "#\!/bin/bash\n\n" > ./build/scripts/build_command.sh
chmod +x ./build/scripts/build_command.sh

case "$TASK" in

  build-linux)
    echo "./bootstrap && mkdir -p build/default && (cd build/default && ../../configure --enable-coverage) && make -C build/default \"${@:-distcheck}\" && git clean -Xdf && (cd examples/lock-app/nrf5 && make)" >> ./build/scripts/build_command.sh 
    ./integrations/docker/images/chip-build/run.sh "build/scripts/build_command.sh"
    ;;

  preflight-shell-check)
    ./integrations/docker/images/chip-build/run.sh "integrations/shellcheck_tree.sh"
    ;;

  *)
    die "Unknown task: $TASK."
    ;;

esac
