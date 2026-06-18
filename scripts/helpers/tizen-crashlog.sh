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

# ============================================================================
# Tizen QEMU Crashlog Analyzer
# ============================================================================
#
# This script is designed to work with Tizen QEMU test runners
# (src/test_driver/tizen/chip_tests/runner.sh and
#  src/test_driver/tizen/integration_tests/lighting-app/runner.sh).
#
# The runners set core_pattern to "/mnt/chip/dump/core.%e.%p.%t" which creates
# raw core dumps instantly on the shared filesystem. This script analyzes
# those raw core dumps from out/tizen-*/dump/ directories.
#
# For analyzing crash-manager dumps from physical Tizen devices (zip format),
# use scripts/helpers/tizen-sdb-crashlog.sh instead.
#
# ============================================================================

set -e

# Configuration
VERBOSE=false
SYSROOT_PATH=""
LAST_ONLY=false

# ============================================================================
# Utility Functions
# ============================================================================

function help() {
    cat <<EOF
Usage: $0 [options]

Analyze raw core dumps from Tizen QEMU tests.

Options:
    --sysroot PATH    - Path to system libraries sysroot (default: \$TIZEN_SDK_ROOT/platforms/...)
    --verbose         - Print GDB command for manual execution
    --last            - Analyze only the most recent coredump (default: all)
    --help            - Print help

Examples:
    $0                                              # Analyze all coredumps
    $0 --last                                       # Analyze most recent coredump
    $0 --sysroot /path/to/sysroot                   # Use custom sysroot
    $0 --verbose                                    # Show GDB command
EOF
}

function parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            --verbose)
                VERBOSE=true
                shift
                ;;
            --sysroot)
                SYSROOT_PATH="$2"
                shift 2
                ;;
            --last)
                LAST_ONLY=true
                shift
                ;;
            --help)
                help
                exit 0
                ;;
            *)
                echo "ERROR: Unknown option $1"
                exit 1
                ;;
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
    if [[ -f "$target_dir/$binary" ]]; then
        echo "$target_dir/$binary"
    elif [[ -f "$target_dir/tests/$binary" ]]; then
        echo "$target_dir/tests/$binary"
    else
        # Linux %e in core_pattern truncates the executable name to 15 characters.
        # If no exact match, try prefix matching to find the full binary name.
        local found
        found=$(find "$target_dir" "$target_dir/tests" -maxdepth 1 -type f -name "$binary*" -executable 2>/dev/null | head -1)
        if [ -n "$found" ]; then
            echo "$found"
        fi
    fi
}

# ============================================================================
# GDB Analysis
# ============================================================================

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

    local gdb_bin="gdb-multiarch"
    local sysroot=""

    # Use provided sysroot path if available
    if [ -n "$SYSROOT_PATH" ]; then
        sysroot="$SYSROOT_PATH"
    elif [ -d "$target_dir/system_libs" ]; then
        sysroot="$target_dir/system_libs"
    elif [ -n "$TIZEN_SDK_ROOT" ]; then
        if [[ "$target_dir" == *"arm64"* || "$target_dir" == *"aarch64"* ]]; then
            gdb_bin="$TIZEN_SDK_ROOT/tools/aarch64-linux-gnu-gcc-14.2/bin/aarch64-linux-gnu-gdb"
            sysroot="$TIZEN_SDK_ROOT/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device64.core"
        else
            gdb_bin="$TIZEN_SDK_ROOT/tools/arm-linux-gnueabi-gcc-14.2/bin/arm-linux-gnueabi-gdb"
            sysroot="$TIZEN_SDK_ROOT/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device.core"
        fi
    fi

    if [ -n "$sysroot" ]; then
        echo "SYSROOT:         $sysroot"
    fi
    echo "--------------------------------------------------------------------------------"

    local gdb_args=("--batch" "-ex" "set auto-load safe-path /")
    if [ -d "$sysroot" ]; then
        gdb_args+=("-ex" "set sysroot $sysroot" "-ex" "set solib-absolute-prefix $sysroot")
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
    solib_path_str=$(
        IFS=:
        echo "${solib_paths[*]}"
    )
    gdb_args+=("-ex" "set solib-search-path $solib_path_str")

    gdb_args+=(
        "-ex" "file $binary_path"
        "-ex" "core-file $coredump"
        "-ex" "thread apply all bt full"
    )

    if [ "$VERBOSE" = true ]; then
        echo ""
        echo "To run GDB manually, execute:"
        echo "$gdb_bin \\"
        for arg in "${gdb_args[@]}"; do
            echo "    '$arg' \\"
        done
        echo ""
    fi
    "$gdb_bin" "${gdb_args[@]}"
}

# ============================================================================
# Main Logic - Local mode only (raw coredumps from QEMU)
# ============================================================================

parse_arguments "$@"

echo "Analyzing raw core dumps from out/tizen-*/dump/"
echo ""

found_any=false
declare -a ALL_FILES=()

# Collect all raw coredump files from all targets
for target in out/tizen-*; do
    [ -d "$target" ] || continue
    while IFS= read -r -d '' f; do
        ALL_FILES+=("$f:$target")
    done < <(find "$target/dump" -maxdepth 1 -type f -name "core.*" -size +0 -print0 2>/dev/null)
done

# If --last is specified, sort by modification time and take only the most recent
if [ "$LAST_ONLY" = true ] && [ ${#ALL_FILES[@]} -gt 0 ]; then
    LATEST_ENTRY=$(printf '%s\n' "${ALL_FILES[@]}" | while read -r entry; do
        file_path="${entry%%:*}"
        mtime=$(stat -c %Y "$file_path" 2>/dev/null || echo 0)
        printf '%s\t%s\n' "$mtime" "$entry"
    done | sort -rn | head -n 1 | cut -f2-)
    ALL_FILES=("$LATEST_ENTRY")
    LATEST_FILE="${LATEST_ENTRY%%:*}"
    echo "Analyzing most recent coredump: $LATEST_FILE"
    echo ""
fi

for entry in "${ALL_FILES[@]}"; do
    found_any=true
    f="${entry%%:*}"
    target_dir="${entry##*:}"
    fname=$(basename "$f")

    # Extract binary name from core filename (core.NAME.PID.TIMESTAMP)
    # Format: core.TestServer.12345.1781607184
    binary=$(echo "$fname" | cut -d'.' -f2)
    if [ -z "$binary" ]; then
        binary="unknown"
    fi

    if [ -f "$f" ] && [ -s "$f" ]; then
        run_gdb_analysis "$target_dir" "$binary" "$f" "$fname"
    fi
done

if [ "$found_any" = false ]; then
    echo "No raw core dumps found in out/tizen-*/dump/"
    echo ""
    echo "Raw core dumps should be created with pattern: /mnt/chip/dump/core.%e.%p.%t"
    echo "Make sure the test runner set the core_pattern before running tests."
fi
