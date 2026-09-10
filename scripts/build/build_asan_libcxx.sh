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

# Builds an ASan-instrumented libc++/libc++abi/libunwind for Matter ASAN
# tests using -fsanitize-address-field-padding=1.
#
# The flag inserts intra-object redzones into class layouts so ASan can
# detect writes that overflow from one struct member into the next without
# leaving the object. The flag is ABI-breaking: every TU that touches a
# given class (including out-of-line code in libc++.a) must agree on the
# layout, so libc++ itself needs to be rebuilt with the same flag.
#
# The Pigweed-shipped libc++ is built without field padding, so its
# basic_ios/basic_streambuf/etc. members live at different offsets than
# user code sees, producing intra-object-overflow false positives in
# completely innocent code (ifstream::init, etc.).
#
# Idempotent: if the install dir is already built and its inputs (this
# script, clang version) have not changed, the script exits in
# milliseconds. First-time build takes ~3-5 minutes.
#
# Prerequisites: source scripts/activate.sh
#
# Usage:
#   scripts/build/build_asan_libcxx.sh [--out-dir PATH] [--force] [--check] [--help]
#
# Environment:
#   SYSROOT_ASAN_LIBCXX  Override install location
#                        (default: ~/.cache/matter/asan_libcxx)

set -euo pipefail

CHIP_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PW_CLANG="$(command -v clang || true)"
PW_CLANGXX="$(command -v clang++ || true)"
DEFAULT_SYSROOT="$HOME/.cache/matter/asan_libcxx"
SCRIPT_PATH="$(cd "$(dirname "$0")" && pwd)/$(basename "$0")"
SRC="${TMPDIR:-/tmp}/asan-libcxx-build"

usage() {
    cat <<EOF
Usage: $(basename "$0") [--out-dir PATH] [--force] [--check] [--help]

Builds an ASan-instrumented libc++/libc++abi/libunwind for Matter ASAN tests.

Options:
  --out-dir PATH   Install to PATH (overrides SYSROOT_ASAN_LIBCXX)
  --force          Rebuild even if the existing install is current
  --check          Report freshness only; exit 0 if current, 2 if missing,
                   3 if stale. Does not start a build.
  --help           Show this message

Environment:
  SYSROOT_ASAN_LIBCXX  Default install location override
                       (default: ~/.cache/matter/asan_libcxx)

The script is idempotent: if the install is already built and its inputs
(this script, clang version) have not changed, it exits in milliseconds.
A first-time build takes 3-5 minutes.

build_examples.py reads the resolved path via host.py and passes it to
GN as asan_libcxx_dir="<path>". Raw \`gn gen\` users must export
SYSROOT_ASAN_LIBCXX or pass --args='asan_libcxx_dir="..."' explicitly.
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

SYSROOT="${OUT_DIR:-${SYSROOT_ASAN_LIBCXX:-$DEFAULT_SYSROOT}}"
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

# Hash inputs that determine install contents. Path-independent so local
# and CI agree even when CHIP_ROOT differs. The script's own contents are
# included so any change to the build recipe (CMake args, libc++ patch,
# flag list) triggers a rebuild.
compute_input_hash() {
    {
        sha256sum <"$SCRIPT_PATH" | cut -d' ' -f1
        "$PW_CLANG" --version
    } | sha256sum | cut -d' ' -f1
}

EXPECTED_HASH="$(compute_input_hash)"

# --check mode: report freshness for callers (e.g. host.py) and exit.
# Exit 0 on fresh, 2 on missing stamp, 3 on stale stamp. No build is started.
if [[ "$CHECK_ONLY" -eq 1 ]]; then
    if [[ ! -f "$STAMP" ]]; then
        echo "ASAN libcxx missing at $SYSROOT (no stamp file)" >&2
        exit 2
    fi
    if [[ "$(cat "$STAMP")" != "$EXPECTED_HASH" ]]; then
        echo "ASAN libcxx stale at $SYSROOT (inputs changed since build)" >&2
        exit 3
    fi
    echo "ASAN libcxx OK at $SYSROOT"
    exit 0
fi

# Stamp check: short-circuit if up-to-date.
if [[ "$FORCE" -eq 0 ]] && [[ -f "$STAMP" ]] && [[ "$(cat "$STAMP")" == "$EXPECTED_HASH" ]]; then
    echo "ASAN libcxx up-to-date at $SYSROOT"
    exit 0
fi

mkdir -p "$SYSROOT" "$SRC"

# Serialize concurrent invocations on the same install dir. flock blocks
# until the lock is acquired; a second concurrent build will see the
# stamp file written by the first and short-circuit on retry.
exec 9>"$LOCKFILE"
if ! flock -x -n 9; then
    echo "Another build is in progress on $SYSROOT, waiting..."
    flock -x 9
fi
if [[ "$FORCE" -eq 0 ]] && [[ -f "$STAMP" ]] && [[ "$(cat "$STAMP")" == "$EXPECTED_HASH" ]]; then
    echo "ASAN libcxx built by concurrent process; reusing"
    exit 0
fi

# Clean up stamp file on failure so the next invocation rebuilds from
# scratch instead of trusting half-finished state.
on_error() {
    echo "ERROR: build failed; removing stamp at $STAMP" >&2
    rm -f "$STAMP"
}
trap on_error ERR

echo ">>> Building ASAN libcxx at $SYSROOT (this takes 3-5 min)"
echo ">>> Pigweed clang LLVM revision: $LLVM_COMMIT"

# Fetch llvm-project at the exact commit the Pigweed clang was built from.
# Using the same commit guarantees the libc++ headers and ABI match what
# clang's driver expects when it links the runtimes.
if [[ ! -d "$SRC/llvm-project" ]]; then
    git -c init.defaultBranch=main init -q "$SRC/llvm-project"
    git -C "$SRC/llvm-project" remote add origin https://github.com/llvm/llvm-project.git
fi
git -C "$SRC/llvm-project" fetch --depth=1 origin "$LLVM_COMMIT"
git -C "$SRC/llvm-project" checkout -q --force FETCH_HEAD

# Patch libc++'s __compressed_pair_padding<T> to skip the
# `char __padding_[sizeof(T) - __datasizeof_v<T>]` array when field-padding
# makes __datasizeof exceed sizeof. Without this, every TU that includes
# std::shared_ptr fails to compile under -fsanitize-address-field-padding=1
# with `array is too large (18'446'744'073'709'551'608 elements)` because
# the subtraction underflows.
#
# Upstream libc++ accepts only the `==` case as "no padding to claim"; the
# field-padding flag legitimately produces sizeof <= __datasizeof, which is
# the same situation (nothing to fill). Generalising the predicate to `<=`
# is safe and minimal.
CP_HEADER="$SRC/llvm-project/libcxx/include/__memory/compressed_pair.h"
if ! grep -q 'sizeof(_ToPad) <= __datasizeof_v<_ToPad>' "$CP_HEADER"; then
    sed -i 's|sizeof(_ToPad) == __datasizeof_v<_ToPad>|sizeof(_ToPad) <= __datasizeof_v<_ToPad>|' "$CP_HEADER"
    if ! grep -q 'sizeof(_ToPad) <= __datasizeof_v<_ToPad>' "$CP_HEADER"; then
        echo "ERROR: failed to patch compressed_pair.h for field-padding compatibility" >&2
        echo "  (upstream may have refactored the predicate; update this script)" >&2
        exit 1
    fi
fi

echo ">>> Configuring libc++ / libc++abi / libunwind"
rm -rf "$SRC/build"
# CMAKE_*_FLAGS carries -fsanitize=address for everything. The field-padding
# flag goes only to libcxx/libcxxabi via LIBCXX_/LIBCXXABI_ADDITIONAL_*; if
# applied to libunwind, its UnwindCursor<> static_assert fails because the
# inserted redzones grow the cursor past the size unw_cursor_t reserves.
cmake -G Ninja -S "$SRC/llvm-project/runtimes" -B "$SRC/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER="$PW_CLANG" \
    -DCMAKE_CXX_COMPILER="$PW_CLANGXX" \
    -DCMAKE_INSTALL_PREFIX="$SYSROOT" \
    -DLLVM_ENABLE_RUNTIMES='libcxx;libcxxabi;libunwind' \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DCMAKE_C_FLAGS='-fsanitize=address -fno-omit-frame-pointer' \
    -DCMAKE_CXX_FLAGS='-fsanitize=address -fno-omit-frame-pointer' \
    -DLIBCXX_ADDITIONAL_COMPILE_FLAGS='-fsanitize-address-field-padding=1' \
    -DLIBCXXABI_ADDITIONAL_COMPILE_FLAGS='-fsanitize-address-field-padding=1' \
    -DLIBCXX_ENABLE_SHARED=OFF -DLIBCXX_ENABLE_STATIC=ON \
    -DLIBCXXABI_ENABLE_SHARED=OFF -DLIBCXXABI_ENABLE_STATIC=ON \
    -DLIBUNWIND_ENABLE_SHARED=OFF -DLIBUNWIND_ENABLE_STATIC=ON \
    -DLIBCXX_USE_COMPILER_RT=ON \
    -DLIBCXXABI_USE_COMPILER_RT=ON \
    -DLIBCXXABI_USE_LLVM_UNWINDER=ON \
    -DLIBCXX_INCLUDE_BENCHMARKS=OFF -DLIBCXX_INCLUDE_TESTS=OFF \
    -DLIBCXXABI_INCLUDE_TESTS=OFF -DLIBUNWIND_INCLUDE_TESTS=OFF

echo ">>> Building libc++ / libc++abi / libunwind"
ninja -C "$SRC/build" -j"$(nproc)" cxx cxxabi unwind

echo ">>> Installing to $SYSROOT"
ninja -C "$SRC/build" install-cxx install-cxxabi install-unwind

# Stamp last so a partial build leaves the dir without a valid stamp and
# the next invocation rebuilds.
echo "$EXPECTED_HASH" >"$STAMP"
echo ">>> Done. ASAN libcxx installed at $SYSROOT"
