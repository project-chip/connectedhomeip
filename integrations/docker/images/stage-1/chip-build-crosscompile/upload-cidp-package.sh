#!/usr/bin/env bash

# This script is intended to be run post-install (NOTE: install takes a LONG time)
# of start-sysroot-vm.

set -e

OUTPUT_DIR=ubuntu-24.04-aarch64-sysroot
PACKAGE=experimental/matter/sysroot/ubuntu-24.04-aarch64

echo "Copying a sysroot in $OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR"

RSYNC_RSH="ssh -p 5555"

echo "COPYING from VM (ubuntu@localhost on port 5555) ..."
rsync \
    --archive \
    --human-readable \
    --links \
    --delete-during \
    --delete-excluded \
    --safe-links \
    --info=progress2 \
    --info=name0 \
    --exclude='lib/aarch64-linux-gnu/dri' \
    --exclude='lib/firmware' \
    --exclude='lib/git-core' \
    --exclude='lib/modules' \
    --exclude='lib/ssl/private' \
    --exclude='lib/systemd' \
    ubuntu@localhost:/lib \
    ubuntu@localhost:/usr/include \
    ubuntu@localhost:/usr/lib \
    "$OUTPUT_DIR"

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
