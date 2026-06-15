#!/bin/bash

#    Copyright (c) 2026 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

set -e

# Configuration
CRASH_REMOTE_DIR="/opt/usr/share/crash/dump"
LOCAL_TMP_DIR="out/tizen-crashes-tmp"
SEARCH_HOURS=24
CLEANUP=true
VERBOSE=false
LOCAL_MODE=false

# ============================================================================
# Utility Functions
# ============================================================================

function help() {
    cat <<EOF
Usage: $0 [options]

Options:
    --hours NUM     - Filter crashes from the last NUM hours (default: 24)
    --target SDB_ID - SDB identifier (e.g. 192.168.0.1:26101)
    --out-dir DIR   - Build output directory (e.g. out/tizen-arm)
    --local         - Local mode: analyze dumps from out/tizen-*/dump/ (used in CI)
    --verbose       - Print more information during execution
    --no-clean      - Keep temporary files after analysis
    --help          - Print help
EOF
}

function parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            --hours) SEARCH_HOURS="$2"; shift 2 ;;
            --target) TARGET_DEVICE="$2"; shift 2 ;;
            --out-dir) OUT_DIR="$2"; shift 2 ;;
            --verbose) VERBOSE=true; shift ;;
            --no-clean) CLEANUP=false; shift ;;
            --local) LOCAL_MODE=true; shift ;;
            --help) help; exit 0 ;;
            *) echo "ERROR: Unknown option $1"; exit 1 ;;
        esac
    done
}

function resolve_binary_name() {
    local app_id="$1"
    # Search for app_id -> executable mapping in tizen-manifest.xml files
    while IFS= read -r manifest; do
        local manifest_appid
        manifest_appid=$(sed -n 's/.*appid="\([^"]*\)".*/\1/p' "$manifest" | head -1)
        if [ "$manifest_appid" = "$app_id" ]; then
            sed -n 's/.*exec="\([^"]*\)".*/\1/p' "$manifest" | head -1
            return
        fi
    done < <(find examples -name "tizen-manifest.xml" 2>/dev/null)
    # Fallback: use app_id as the binary name (common for unit tests)
    echo "$app_id"
}

function resolve_binary_path() {
    local target_dir="$1"
    local binary="$2"
    # Search in root and tests/ subfolder
    if [[ -f "$target_dir/$binary" ]]; then echo "$target_dir/$binary"
    elif [[ -f "$target_dir/tests/$binary" ]]; then echo "$target_dir/tests/$binary"
    fi
}

# ============================================================================
# Extraction & Analysis
# ============================================================================

function extract_coredump() {
    local archive_path="$1"
    local dest_dir="$2"
    if [[ "$archive_path" == *.zip ]]; then
        unzip -qo "$archive_path" -d "$dest_dir"
    fi
    # Tizen crash-manager tars are often truncated in QEMU. --ignore-zeros is vital.
    local tar_file
    tar_file=$(find "$dest_dir" -name "*.coredump.tar" 2>/dev/null | head -n 1)
    if [ -n "$tar_file" ]; then
        if [ "$VERBOSE" = true ]; then echo "Extracting $tar_file with --ignore-zeros"; fi
        tar --ignore-zeros -xf "$tar_file" -C "$dest_dir" 2>/dev/null || true
    fi
}

function find_coredump_file() {
    local dir="$1"
    local f
    # 1. Prefer non-empty files explicitly named .coredump (raw or extracted)
    f=$(find "$dir" -type f -name "*.coredump" -size +0 2>/dev/null | head -1)
    [ -n "$f" ] && echo "$f" && return
    # 2. Check file type as fallback, also ensuring size > 0
    mapfile -t all < <(find "$dir" -type f -size +0 2>/dev/null)
    for f in "${all[@]}"; do
        if file "$f" | grep -q 'core file'; then echo "$f"; return; fi
    done
    echo ""
}

function run_gdb_analysis() {
    local target_dir="$1"
    local binary_name="$2"
    local coredump="$3"
    local crash_name="$4"

    local binary_path
    binary_path=$(resolve_binary_path "$target_dir" "$binary_name")
    if [ -z "$binary_path" ]; then
        echo "WARNING: Could not find binary '$binary_name' in $target_dir"
        return
    fi

    local coredump_size=""
    if [ -f "$coredump" ]; then
        coredump_size=$(du -sh "$coredump" 2>/dev/null | cut -f1)
    fi

    echo "--------------------------------------------------------------------------------"
    echo "ANALYZING CRASH: $crash_name"
    echo "BINARY:          $binary_path"
    echo "COREDUMP:        $coredump ($coredump_size)"
    echo "--------------------------------------------------------------------------------"

    local gdb_bin="gdb-multiarch"
    local sysroot=""
    if [ -d "$target_dir/system_libs" ]; then
        sysroot="$target_dir/system_libs"
    fi

    if [ -n "$TIZEN_SDK_ROOT" ]; then
        if [[ "$target_dir" == *"arm64"* || "$target_dir" == *"aarch64"* ]]; then
            gdb_bin="$TIZEN_SDK_ROOT/tools/aarch64-linux-gnu-gcc-14.2/bin/aarch64-linux-gnu-gdb"
            if [ -z "$sysroot" ]; then
                sysroot="$TIZEN_SDK_ROOT/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device64.core"
            fi
        else
            gdb_bin="$TIZEN_SDK_ROOT/tools/arm-linux-gnueabi-gcc-14.2/bin/arm-linux-gnueabi-gdb"
            if [ -z "$sysroot" ]; then
                sysroot="$TIZEN_SDK_ROOT/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device.core"
            fi
        fi
    fi

    local gdb_args=("--batch" "-ex" "set auto-load safe-path /")
    if [ -d "$sysroot" ]; then
        gdb_args+=("-ex" "set sysroot $sysroot" "-ex" "set solib-absolute-prefix $sysroot")
        # Create a native host symlink to map /mnt/chip inside the sysroot if using system_libs
        if [ "$sysroot" = "$target_dir/system_libs" ]; then
            mkdir -p "$sysroot/mnt"
            ln -sf "../../" "$sysroot/mnt/chip" 2>/dev/null || true
        fi
    fi

    # Map Tizen device paths to host paths for GDB file-backed mappings
    gdb_args+=("-ex" "set substitute-path /mnt/chip/ $target_dir/")
    if [ -d "$sysroot" ]; then
        if [[ "$target_dir" == *"arm64"* || "$target_dir" == *"aarch64"* ]]; then
            gdb_args+=("-ex" "set substitute-path /usr/lib/ $sysroot/lib64/")
            gdb_args+=("-ex" "set substitute-path /lib/ $sysroot/lib64/")
        else
            gdb_args+=("-ex" "set substitute-path /usr/lib/ $sysroot/lib/")
            gdb_args+=("-ex" "set substitute-path /lib/ $sysroot/lib/")
        fi
    fi

    # Set solib-search-path to locate shared libraries by their filenames
    local solib_paths=("$target_dir" "$target_dir/tests")
    if [ -d "$sysroot" ]; then
        if [[ "$target_dir" == *"arm64"* || "$target_dir" == *"aarch64"* ]]; then
            solib_paths+=("$sysroot/lib64" "$sysroot/usr/lib64")
            if [ -n "$TIZEN_SDK_ROOT" ]; then
                solib_paths+=("$TIZEN_SDK_ROOT/tools/aarch64-linux-gnu-gcc-14.2/aarch64-tizen-linux-gnu/lib64")
            fi
        else
            solib_paths+=("$sysroot/lib" "$sysroot/usr/lib")
            if [ -n "$TIZEN_SDK_ROOT" ]; then
                solib_paths+=("$TIZEN_SDK_ROOT/tools/arm-linux-gnueabi-gcc-14.2/arm-tizen-linux-gnueabi/lib")
            fi
        fi
    fi
    local solib_path_str
    solib_path_str=$(IFS=:; echo "${solib_paths[*]}")
    gdb_args+=("-ex" "set solib-search-path $solib_path_str")

    gdb_args+=(
        "-ex" "file $binary_path"
        "-ex" "core-file $coredump"
        "-ex" "thread apply all bt full"
    )

    if [ "$VERBOSE" = true ]; then echo "Running: $gdb_bin ${gdb_args[*]}"; fi
    "$gdb_bin" "${gdb_args[@]}"
}

# ============================================================================
# Main Logic
# ============================================================================

parse_arguments "$@"

# Auto-detect local mode if no SDB device is reachable
if [ "$LOCAL_MODE" = false ]; then
    if ! command -v sdb >/dev/null 2>&1 || [ "$(sdb devices 2>/dev/null | grep -cv 'List')" -eq 0 ]; then
        echo "No SDB device found, switching to local mode."
        LOCAL_MODE=true
    fi
fi

if [ "$LOCAL_MODE" = true ]; then
    found_any=false
    for target in out/tizen-*; do
        [ -d "$target/dump" ] || continue
        # Find primary zips and raw coredumps (limit search depth to avoid nested files)
        mapfile -t files < <(find "$target/dump" -mindepth 1 -maxdepth 2 -type f \( -name "*.zip" -o -name "*.coredump" \) 2>/dev/null)
        for f in "${files[@]}"; do
            found_any=true
            base_dir=$(dirname "$f")
            fname=$(basename "$f")
            # Extract application ID (part before the first underscore)
            app_id="${fname%%_*}"
            binary=$(resolve_binary_name "$app_id")

            coredump=""
            if [[ "$fname" == *.zip ]]; then
                # Extract zip to its own isolated subdirectory
                extract_dir="${f%.zip}"
                mkdir -p "$extract_dir"
                unzip -qo "$f" -d "$extract_dir"

                # Extract the nested .tar file inside that same directory
                tar_file=$(find "$extract_dir" -name "*.coredump.tar" 2>/dev/null | head -n 1)
                if [ -n "$tar_file" ]; then
                    tar --ignore-zeros -xf "$tar_file" -C "$extract_dir" 2>/dev/null || true
                fi

                # Find the non-empty coredump file inside that directory
                coredump=$(find_coredump_file "$extract_dir")
            else
                coredump="$f"
            fi

            if [ -n "$coredump" ] && [ -s "$coredump" ]; then
                run_gdb_analysis "$target" "$binary" "$coredump" "$fname"
            fi
        done
    done
    [ "$found_any" = false ] && echo "No crash dumps found in out/tizen-*/dump/"
    exit 0
fi

# SDB remote mode implementation (simplified placeholder)
echo "SDB remote mode is currently limited. Please use --local for CI analysis."
exit 1
