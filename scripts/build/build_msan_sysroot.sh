#!/bin/bash
# Copyright (c) 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Builds an MSan-instrumented sysroot for Matter MSAN tests.
#
# The sysroot contains libc++/libc++abi and a few common third-party
# dependencies (OpenSSL, zlib, libffi, pcre2, GLib) all compiled with
# -fsanitize=memory so MSAN reports are not poisoned by uninstrumented
# library internals.
#
# Idempotent: if the sysroot is already built and its inputs (this script,
# msan_ignorelist.txt, clang version) have not changed, the script exits
# in milliseconds. First-time build takes ~5-15 minutes.
#
# Prerequisites: source scripts/activate.sh
#
# Usage:
#   scripts/build/build_msan_sysroot.sh [--out-dir PATH] [--force] [--help]
#
# Environment:
#   SYSROOT_MSAN  Override install location (default: ~/.cache/matter/msan_sysroot)

set -euo pipefail

CHIP_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PW_CLANG="$(command -v clang || true)"
PW_CLANGXX="$(command -v clang++ || true)"
DEFAULT_SYSROOT="$HOME/.cache/matter/msan_sysroot"
IGNORELIST="$CHIP_ROOT/build/config/compiler/msan_ignorelist.txt"
SCRIPT_PATH="$(cd "$(dirname "$0")" && pwd)/$(basename "$0")"
SRC="${TMPDIR:-/tmp}/msan-build"
MSAN="-fsanitize=memory -fsanitize-memory-track-origins -fno-omit-frame-pointer -fPIC"

# Pinned for reproducibility and to keep msan_ignorelist.txt source paths
# matching. On bump: verify clean MSAN build, no new false positives, and
# ignorelist paths still match. libc++/libc++abi track Pigweed clang.
OPENSSL_VERSION="3.5.3"
ZLIB_VERSION="1.3.1"
LIBFFI_VERSION="3.4.6"
PCRE2_VERSION="10.43"
GLIB_VERSION="2.80.0"
GLIB_SERIES="${GLIB_VERSION%.*}" # GNOME download path uses major.minor (e.g. 2.80)

usage() {
    cat <<EOF
Usage: $(basename "$0") [--out-dir PATH] [--force] [--check] [--help]

Builds an MSan-instrumented sysroot for Matter MSAN tests.

Options:
  --out-dir PATH   Install sysroot to PATH (overrides SYSROOT_MSAN)
  --force          Rebuild even if the existing sysroot is current
  --check          Report freshness only; exit 0 if current, 2 if missing,
                   3 if stale. Does not start a build.
  --help           Show this message

Environment:
  SYSROOT_MSAN     Default install location override
                   (default: ~/.cache/matter/msan_sysroot)

The script is idempotent: if the sysroot is already built and its inputs
(this script, msan_ignorelist.txt, clang version) have not changed, it
exits in milliseconds. A first-time build takes 5-15 minutes.

build_examples.py reads the resolved sysroot path via host.py and passes
it to GN as msan_sysroot="<path>". Raw \`gn gen\` users must export
SYSROOT_MSAN or pass --args='msan_sysroot="..."' explicitly.
EOF
}

FORCE=0
CHECK_ONLY=0
OUT_DIR=""
while [[ $# -gt 0 ]]; do
    case "$1" in
        --out-dir)
            OUT_DIR="$2"
            shift 2
            ;;
        --force)
            FORCE=1
            shift
            ;;
        --check)
            CHECK_ONLY=1
            shift
            ;;
        --help | -h)
            usage
            exit 0
            ;;
        *)
            echo "Unknown argument: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
done

SYSROOT="${OUT_DIR:-${SYSROOT_MSAN:-$DEFAULT_SYSROOT}}"
STAMP="$SYSROOT/.build_complete"
LOCKFILE="$SYSROOT/.build.lock"

if [[ ! -x "$PW_CLANG" ]] || [[ ! -x "$PW_CLANGXX" ]]; then
    echo "ERROR: Pigweed clang/clang++ not found on PATH" >&2
    echo "  clang:   ${PW_CLANG:-<none>}" >&2
    echo "  clang++: ${PW_CLANGXX:-<none>}" >&2
    echo "Run: source scripts/activate.sh" >&2
    exit 1
fi

LLVM_COMMIT="$("$PW_CLANG" --version | grep -oP '[0-9a-f]{40}' || true)"
if [[ -z "$LLVM_COMMIT" ]]; then
    echo "ERROR: clang on PATH is not Pigweed's (no LLVM commit in --version)" >&2
    echo "  clang: $PW_CLANG" >&2
    echo "Run: source scripts/activate.sh" >&2
    exit 1
fi

if [[ ! -f "$IGNORELIST" ]]; then
    echo "ERROR: $IGNORELIST not found" >&2
    exit 1
fi

# Hash inputs that determine sysroot contents. Path-independent so local
# and CI agree even when CHIP_ROOT differs. Format: each file contributes
# only its content sha (no path), followed by clang --version output.
# This hash is used by this script's --check mode to detect staleness for
# callers.
compute_input_hash() {
    {
        sha256sum <"$SCRIPT_PATH" | cut -d' ' -f1
        sha256sum <"$IGNORELIST" | cut -d' ' -f1
        "$PW_CLANG" --version
    } | sha256sum | cut -d' ' -f1
}

EXPECTED_HASH="$(compute_input_hash)"

# --check mode: report sysroot freshness for callers (e.g. host.py) and exit.
# Exit 0 on fresh, 2 on missing stamp, 3 on stale stamp. No build is started.
if [[ "$CHECK_ONLY" -eq 1 ]]; then
    if [[ ! -f "$STAMP" ]]; then
        echo "MSAN sysroot missing at $SYSROOT (no stamp file)" >&2
        exit 2
    fi
    if [[ "$(cat "$STAMP")" != "$EXPECTED_HASH" ]]; then
        echo "MSAN sysroot stale at $SYSROOT (inputs changed since build)" >&2
        exit 3
    fi
    echo "MSAN sysroot OK at $SYSROOT"
    exit 0
fi

# Stamp check: short-circuit if up-to-date.
if [[ "$FORCE" -eq 0 ]] && [[ -f "$STAMP" ]] && [[ "$(cat "$STAMP")" == "$EXPECTED_HASH" ]]; then
    echo "MSAN sysroot up-to-date at $SYSROOT"
    exit 0
fi

mkdir -p "$SYSROOT" "$SRC"

# Serialize concurrent invocations on the same sysroot dir.
# flock blocks until the lock is acquired; a second concurrent build
# will see the stamp file written by the first and short-circuit on retry.
exec 9>"$LOCKFILE"
if ! flock -x -n 9; then
    echo "Another build is in progress on $SYSROOT, waiting..."
    flock -x 9
fi
# After acquiring the lock, re-check the stamp: another process may
# have completed the build while we were blocked.
if [[ "$FORCE" -eq 0 ]] && [[ -f "$STAMP" ]] && [[ "$(cat "$STAMP")" == "$EXPECTED_HASH" ]]; then
    echo "MSAN sysroot built by concurrent process; reusing"
    exit 0
fi

# Clean up stamp file on failure so the next invocation rebuilds
# from scratch instead of trusting half-finished state.
on_error() {
    echo "ERROR: build failed; removing stamp at $STAMP" >&2
    rm -f "$STAMP"
}
trap on_error ERR

echo ">>> Building MSAN sysroot at $SYSROOT (this takes 5-15 min)"

# libc++ / libc++abi (MSan-instrumented)
echo ">>> libc++ / libc++abi"
if [[ ! -d "$SRC/llvm-project" ]]; then
    git clone --depth 1 https://llvm.googlesource.com/llvm-project "$SRC/llvm-project"
fi
cd "$SRC/llvm-project"
git fetch --depth 1 origin "$LLVM_COMMIT"
git checkout FETCH_HEAD
cmake -GNinja -S "$SRC/llvm-project/runtimes" -B "$SRC/libcxx" \
    -DCMAKE_C_COMPILER="$PW_CLANG" -DCMAKE_CXX_COMPILER="$PW_CLANGXX" \
    -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi" -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_USE_SANITIZER=MemoryWithOrigins -DCMAKE_INSTALL_PREFIX="$SYSROOT" \
    -DLIBCXX_ENABLE_SHARED=OFF -DLIBCXXABI_ENABLE_SHARED=OFF -DLIBCXXABI_USE_LLVM_UNWINDER=OFF
ninja -C "$SRC/libcxx" -j"$(nproc)" cxx cxxabi
ninja -C "$SRC/libcxx" install-cxx install-cxxabi

# OpenSSL
echo ">>> OpenSSL"
cd "$SRC"
[[ -d "openssl-$OPENSSL_VERSION" ]] || { wget -q "https://www.openssl.org/source/openssl-$OPENSSL_VERSION.tar.gz" && tar xzf "openssl-$OPENSSL_VERSION.tar.gz"; }
cd "openssl-$OPENSSL_VERSION"
CC="$PW_CLANG $MSAN" ./Configure linux-x86_64 --prefix="$SYSROOT" --openssldir="$SYSROOT/ssl" no-asm no-shared -DPURIFY
make -j"$(nproc)" build_libs
make install_dev

# zlib
echo ">>> zlib"
cd "$SRC"
[[ -d "zlib-$ZLIB_VERSION" ]] || { wget -q -O - "https://github.com/madler/zlib/releases/download/v$ZLIB_VERSION/zlib-$ZLIB_VERSION.tar.gz" | tar xz; }
cd "zlib-$ZLIB_VERSION"
CC="$PW_CLANG" CFLAGS="$MSAN" LDFLAGS="-fsanitize=memory" ./configure --prefix="$SYSROOT" --static
make -j"$(nproc)" && make install

# libffi
echo ">>> libffi"
cd "$SRC"
[[ -d "libffi-$LIBFFI_VERSION" ]] || { wget -q "https://github.com/libffi/libffi/releases/download/v$LIBFFI_VERSION/libffi-$LIBFFI_VERSION.tar.gz" && tar xzf "libffi-$LIBFFI_VERSION.tar.gz"; }
cd "libffi-$LIBFFI_VERSION"
CC="$PW_CLANG" CXX="$PW_CLANGXX" CFLAGS="$MSAN" CXXFLAGS="$MSAN" LDFLAGS="-fsanitize=memory" \
    ./configure --prefix="$SYSROOT" --disable-shared --enable-static --quiet
make -j"$(nproc)" && make install

# pcre2
echo ">>> pcre2"
cd "$SRC"
[[ -d "pcre2-$PCRE2_VERSION" ]] || { wget -q "https://github.com/PCRE2Project/pcre2/releases/download/pcre2-$PCRE2_VERSION/pcre2-$PCRE2_VERSION.tar.gz" && tar xzf "pcre2-$PCRE2_VERSION.tar.gz"; }
cd "pcre2-$PCRE2_VERSION"
CC="$PW_CLANG" CXX="$PW_CLANGXX" CFLAGS="$MSAN" CXXFLAGS="$MSAN" LDFLAGS="-fsanitize=memory" \
    ./configure --prefix="$SYSROOT" --disable-shared --enable-static --quiet
make -j"$(nproc)" && make install

# GLib
echo ">>> GLib"
cd "$SRC"
[[ -d "glib-$GLIB_VERSION" ]] || { wget -q "https://download.gnome.org/sources/glib/$GLIB_SERIES/glib-$GLIB_VERSION.tar.xz" && tar xf "glib-$GLIB_VERSION.tar.xz"; }

cat >"$SRC/msan-native.ini" <<EOF
[binaries]
c = '$PW_CLANG'
cpp = '$PW_CLANGXX'
ar = 'ar'
strip = 'strip'
pkgconfig = 'pkg-config'

[built-in options]
c_args = ['-fsanitize=memory', '-fsanitize-memory-track-origins', '-fno-omit-frame-pointer', '-fPIC', '-Wno-error=implicit-function-declaration', '-fsanitize-ignorelist=$IGNORELIST']
cpp_args = ['-fsanitize=memory', '-fsanitize-memory-track-origins', '-fno-omit-frame-pointer', '-fPIC', '-fsanitize-ignorelist=$IGNORELIST']
c_link_args = ['-fsanitize=memory']
cpp_link_args = ['-fsanitize=memory']

[properties]
pkg_config_path = '$SYSROOT/lib/pkgconfig:$SYSROOT/lib64/pkgconfig'
EOF

cd "glib-$GLIB_VERSION"
rm -rf builddir
PKG_CONFIG_PATH="$SYSROOT/lib/pkgconfig:$SYSROOT/lib64/pkgconfig" \
    meson setup builddir --native-file "$SRC/msan-native.ini" --prefix="$SYSROOT" \
    --libdir=lib \
    --default-library=static --buildtype=debugoptimized \
    -Dselinux=disabled -Dxattr=false -Dlibmount=disabled -Dnls=disabled \
    -Dtests=false -Dglib_checks=false -Dglib_assert=false \
    -Dintrospection=disabled -Ddtrace=false -Dsystemtap=false \
    -Dman-pages=disabled -Ddocumentation=false -Dbsymbolic_functions=false
ninja -C builddir -j"$(nproc)" && ninja -C builddir install

# Stamp file written last; only exists on full success.
echo "$EXPECTED_HASH" >"$STAMP"

trap - ERR
echo ">>> Sysroot ready at $SYSROOT"
