#!/usr/bin/env bash

# This script cross-compiles libpcap and tcpdump for a specified architecture (default: ARM64)

# Set bash script to exit immediately if any commands fail, any variables are unexpanded, or any commands in a pipeline fail.
set -o errexit
set -o nounset
set -o pipefail

# Check for optional target architecture argument, default to aarch64-linux-gnu if not provided
TARGET_ARCH="${1:-aarch64-linux-gnu}"

# Create a random temporary directory using mktemp
TMP_DIR=$(mktemp -d)
OUT_DIR=$TMP_DIR/out

# Function to download and extract archives
download_and_extract() {
    local url="$1"
    local filepath="$2"
    local tar_dir="$3"

    wget -O "$filepath" "$url"
    tar -C "$tar_dir" -zxvf "$filepath"
}

# Function to clean up downloaded and extracted files
cleanup() {
    local filepath="$1"
    local dirpath="$2"

    rm -rf "$filepath" "$dirpath"
}

# Cross-compile libpcap
LIBPCAP_VERSION=1.10.4
LIBPCAP_DIR=$TMP_DIR/libpcap-$LIBPCAP_VERSION
LIBPCAP_ARCHIVE=$TMP_DIR/libpcap-$LIBPCAP_VERSION.tar.gz

download_and_extract "https://www.tcpdump.org/release/libpcap-$LIBPCAP_VERSION.tar.gz" "$LIBPCAP_ARCHIVE" "$TMP_DIR"
(cd "$LIBPCAP_DIR" && ./configure --prefix="$OUT_DIR" --host="$TARGET_ARCH" --with-pcap=linux)
make -C "$LIBPCAP_DIR" -j"$(nproc)"
make -C "$LIBPCAP_DIR" install
cleanup "$LIBPCAP_ARCHIVE" "$LIBPCAP_DIR"

# Cross-compile tcpdump
TCPDUMP_VERSION=4.99.4
TCPDUMP_DIR=$TMP_DIR/tcpdump-$TCPDUMP_VERSION
TCPDUMP_ARCHIVE=$TMP_DIR/tcpdump-$TCPDUMP_VERSION.tar.gz

download_and_extract "https://www.tcpdump.org/release/tcpdump-$TCPDUMP_VERSION.tar.gz" "$TCPDUMP_ARCHIVE" "$TMP_DIR"
(cd "$TCPDUMP_DIR" && CFLAGS="-static -I$OUT_DIR/include" CPPFLAGS="-static" LDFLAGS="-static -L$OUT_DIR/lib" ./configure --prefix="$OUT_DIR" --host="$TARGET_ARCH")
make -C "$TCPDUMP_DIR"
make -C "$TCPDUMP_DIR" install
cleanup "$TCPDUMP_ARCHIVE" "$TCPDUMP_DIR"

# Prepare the artifact
strip "$OUT_DIR/bin/tcpdump"
mv "$OUT_DIR/bin/tcpdump" .
rm -rf "$TMP_DIR"
