#!/bin/bash -eu
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
#
# Generates OSS-Fuzz fuzz-target wrappers for GN-built pw_fuzzer FuzzTest binaries.
#
# A single FuzzTest binary hosts many FUZZ_TEST() cases and is run as a libFuzzer
# target one case at a time (--fuzz=<Suite.Case>), whereas OSS-Fuzz expects one fuzz
# target per executable. This bridges the two: for each binary it
#   * copies the binary into $OUT/bin/ -- OSS-Fuzz scans only the root of $OUT for fuzz
#     targets, so the shared binary there is not treated as a (broken, multi-case) target;
#   * enumerates the cases via `--list_fuzz_tests`; and
#   * writes one wrapper per case, named "{binary}@{Suite.Case}", that execs the shared
#     binary with `--fuzz=<Suite.Case>`.
#
# The wrapper embeds the literal token LLVMFuzzerTestOneInput so OSS-Fuzz's grep-based
# fuzz-target detector (infra/base-images/base-runner/targets_list) recognizes it. This
# mirrors OSS-Fuzz's own Bazel recipe (infra/base-images/base-builder/compile_fuzztests.sh)
# for GN-built binaries.
#
# Usage: gen_pw_fuzztest_oss_fuzz_wrappers.sh <OUT_DIR> <binary> [<binary> ...]

set -euo pipefail

if [[ $# -lt 2 ]]; then
    echo "usage: $0 <OUT_DIR> <binary> [<binary> ...]" >&2
    exit 1
fi

OUT="$1"
shift

for fuzz_main_file in "$@"; do
    if [[ ! -x "$fuzz_main_file" ]]; then
        echo "error: not an executable: $fuzz_main_file" >&2
        exit 1
    fi
    fuzz_basename="$(basename "$fuzz_main_file")"

    # Enumerate the FUZZ_TEST cases (strip FuzzTest's "[*] Fuzz test: " prefix).
    mapfile -t fuzz_tests < <("$fuzz_main_file" --list_fuzz_tests |
        sed -n 's/^\[\*\] Fuzz test: //p')
    if [[ ${#fuzz_tests[@]} -eq 0 ]]; then
        echo "error: $fuzz_basename listed no fuzz tests" >&2
        exit 1
    fi

    # Keep the shared binary out of OSS-Fuzz target discovery (which scans only the root of
    # $OUT) by placing it in a subdirectory. It stays executable, so the wrappers exec it
    # directly -- no runtime chmod, which would fail on read-only fuzzing mounts.
    mkdir -p "$OUT/bin"
    cp "$fuzz_main_file" "$OUT/bin/"

    for fuzz_entrypoint in "${fuzz_tests[@]}"; do
        target_fuzzer="$fuzz_basename@$fuzz_entrypoint"
        cat >"$OUT/$target_fuzzer" <<EOF
#!/bin/sh
# LLVMFuzzerTestOneInput for fuzzer detection.
this_dir=\$(dirname "\$0")
exec "\$this_dir/bin/$fuzz_basename" --fuzz=$fuzz_entrypoint -- "\$@"
EOF
        chmod +x "$OUT/$target_fuzzer"
        echo "  wrapper: $target_fuzzer"
    done
done
