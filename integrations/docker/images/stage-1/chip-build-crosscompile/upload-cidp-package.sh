#!/usr/bin/env bash

# This script is intended to be run post-install (NOTE: install takes a LONG time)
# of start-sysroot-vm.

set -e

echo "COPYING /lib ..."
rsync -ahL --info=progress2 --info=name0 -e 'ssh -p 5555' \
  --exclude='/lib/firmware' \
  --exclude='/lib/git-core' \
  --exclude='/lib/modules' \
  --exclude='/lib/ssl/private' \
  --exclude='/lib/systemd' \
  ubuntu@localhost:/lib ubuntu-24.04-aarch64-sysroot

echo "COPYING /usr/lib ..."
rsync -ahL --info=progress2 --info=name0 -e 'ssh -p 5555' \
  --exclude='lib/firmware' \
  --exclude='lib/git-core' \
  --exclude='lib/modules' \
  --exclude='lib/ssl/private' \
  --exclude='lib/systemd' \
  ubuntu@localhost:/usr/lib ubuntu-24.04-aarch64-sysroot/usr

echo "COPYING /usr/include ..."
rsync -ahL --info=progress2 --info=name0 -e 'ssh -p 5555' \
  ubuntu@localhost:/usr/include ubuntu-24.04-aarch64-sysroot/usr


TODAY=$(date '+%Y.%m.%d')

echo "Creating cipd definition"
cat >cipd.yaml <<TEXTEND
package: experimental/matter/sysroot/ubuntu-24.04-aarch64
description: AArch64 sysroot for cross-compiling matter SDK examples
install_mode: copy
data:
  - dir: ubuntu-24.04-aarch64-sysroot
TEXTEND

# NOTE: sufficient permissions are required to be able to upload this.
#       Login is a prerequisite:
#
#    cipd auth-login
#
echo "Uploading package tagged 'v$TODAY' ..."
cipd create -pkg-def=cipd.yaml -tag "version:v$TODAY"

