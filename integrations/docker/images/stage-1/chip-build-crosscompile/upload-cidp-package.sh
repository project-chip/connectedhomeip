#!/usr/bin/env bash

# This script is intended to be run post-install (NOTE: install takes a LONG time)
# of start-sysroot-vm.

set -e

rsync -avL -e 'ssh -p 5555' \
  --exclude='/lib/firmware' \
  --exclude='/lib/git-core' \
  --exclude='/lib/modules' \
  --exclude='/lib/ssl/private' \
  --exclude='/lib/systemd' \
  ubuntu@localhost:/lib ubuntu-24.04-aarch64-sysroot

rsync -avL -e 'ssh -p 5555' \
  --exclude='/usr/lib/firmware' \
  --exclude='/usr/lib/git-core' \
  ubuntu@localhost:/usr/lib ubuntu-24.04-aarch64-sysroot/usr

rsync -avL -e 'ssh -p 5555' \
  --exclude='/usr/lib/modules' \
  ubuntu@localhost:/usr/include ubuntu-24.04-aarch64-sysroot/usr


tar cvfJ ubuntu-24.04-aarch64-sysroot.tar.xz ubuntu-24.04-aarch64-sysroot

TODAY=$(date '+%Y.%m.%d')

cat >cipd.yaml <<TEXTEND
package: experimental/matter/sysroot/ubuntu-24.04-aarch64
description: AArch64 sysroot for cross-compiling matter SDK examples
install_mode: copy
data:
  - file: ubuntu-24.04-aarch64-sysroot.tar.xz
TEXTEND

# NOTE: sufficient permissions are required to be able to upload this.
#       Login is a prerequisite:
#
#    cipd auth-login
#
cipd create -pkg-def=cipd.yaml -tag "version:v$TODAY"

