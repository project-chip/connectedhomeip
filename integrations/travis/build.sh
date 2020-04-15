#!/bin/sh

die() {
  echo " *** ERROR: " "${*}"
  exit 1
}

case "$TASK" in

  build-linux)
    ./integrations/docker/run_docker_build.sh
    ;;

  preflight-shell-check)
    ./integrations/docker/images/chip-build/run.sh "integrations/shellcheck_tree.sh"
    ;;

  *)
    die "Unknown task: $TASK."
    ;;

esac
