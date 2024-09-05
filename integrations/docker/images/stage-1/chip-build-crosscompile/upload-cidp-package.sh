#!/usr/bin/env bash

# This script is intended to be run post-install (NOTE: install takes a LONG time)
# of start-sysroot-vm.

set -e

OUTPUT_DIR=ubuntu-24.04-aarch64-sysroot
PACKAGE=experimental/matter/sysroot/ubuntu-24.04-aarch64

echo "Copying a sysroot in $OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR"

RSYNC_RSH="ssh -p 5555"
RSYNC_OPTIONS="
--archive
--human-readable
--links
--delete-during
--delete-excluded
--safe-links
--info=progress2
--info=name0
"

echo "COPYING /usr/lib ..."
rsync $RSYNC_OPTIONS \
  --exclude='lib/aarch64-linux-gnu/dri' \
  --exclude='lib/firmware' \
  --exclude='lib/git-core' \
  --exclude='lib/modules' \
  --exclude='lib/ssl/private' \
  --exclude='lib/systemd' \
  ubuntu@localhost:/usr/lib "$OUTPUT_DIR"

echo "COPYING /usr/include ..."
rsync $RSYNC_OPTIONS \
  ubuntu@localhost:/usr/include "$OUTPUT_DIR"

# lib is just a symlink in ubuntu-24.04
echo "CREATING /lib symbolic link ..."
ln -s usr/lib $OUTPUT_DIR/lib 

TODAY=$(date '+%Y.%m.%d')

echo "Creating cipd definition"
cat >cipd.yaml <<TEXTEND
package: $PACKAGE
description: AArch64 sysroot for cross-compiling matter SDK examples
install_mode: copy
data:
  - dir: "$OUTPUT_DIR"
TEXTEND

# NOTE: sufficient permissions are required to be able to upload this.
#       Login is a prerequisite:
#
#    cipd auth-login
#
echo "Uploading package tagged 'v$TODAY' to $PACKAGE ..."
cipd create -pkg-def=cipd.yaml -tag "version:v$TODAY"

