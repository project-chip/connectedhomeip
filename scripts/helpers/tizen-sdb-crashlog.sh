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
VERBOSE=false
SYSROOT_PATH=""

# ============================================================================
# Utility Functions
# ============================================================================

function help() {
    cat <<EOF
Usage: $0 [options]

Analyze core dumps from Tizen devices via SDB (crash-manager format).

Options:
    --target SDB_ID   - SDB identifier (e.g. 192.168.0.1:26101)
    --sysroot PATH    - Path to system libraries sysroot
    --verbose         - Print GDB command for manual execution
    --help            - Print help

Examples:
    $0                                    # Analyze crashes on connected device
    $0 --target 192.168.0.1:26101         # Analyze crashes on specific device
    $0 --sysroot /path/to/sysroot         # Use custom sysroot
EOF
}

function parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            --target) TARGET_DEVICE="$2"; shift 2 ;;
            --sysroot) SYSROOT_PATH="$2"; shift 2 ;;
            --verbose) VERBOSE=true; shift ;;
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
    # Fallback: use app_id as the binary name
    echo "$app_id"
}

function resolve_binary_path() {
    local target_dir="$1"
    local binary="$2"
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
    local tar_file
    tar_file=$(find "$dest_dir" -name "*.coredump.tar" 2>/dev/null | head -n 1)
    if [ -n "$tar_file" ]; then
        tar --ignore-zeros -xf "$tar_file" -C "$dest_dir" 2>/dev/null || true
    fi
}

function find_coredump_file() {
    local dir="$1"
    local f
    f=$(find "$dir" -type f -name "*.coredump" -size +0 2>/dev/null | head -1)
    [ -n "$f" ] && echo "$f" && return
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
    
    local gdb_bin="gdb-multiarch"
    local sysroot=""
    
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

    local solib_paths=("$target_dir" "$target_dir/tests")
    if [ -d "$sysroot" ]; then
        if [[ "$target_dir" == *"arm64"* || "$target_dir" == *"aarch64"* ]]; then
            solib_paths+=("$sysroot/lib64" "$sysroot/usr/lib64")
        else
            solib_paths+=("$sysroot/lib" "$sysroot/usr/lib")
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
# Main Logic - SDB remote mode (crash-manager zip files)
# ============================================================================

parse_arguments "$@"

# Check if SDB is available
if ! command -v sdb >/dev/null 2>&1; then
    echo "ERROR: sdb command not found. Please install Tizen Studio."
    exit 1
fi

# Set target device
if [ -n "$TARGET_DEVICE" ]; then
    SDB_CMD="sdb -s $TARGET_DEVICE"
else
    SDB_CMD="sdb"
fi

# Check device connection
DEVICE_COUNT=$($SDB_CMD devices 2>/dev/null | grep -cv 'List' || echo 0)
if [ "$DEVICE_COUNT" -eq 0 ]; then
    echo "ERROR: No SDB device found."
    echo "Connect a device or specify --target <device_id>"
    exit 1
fi

echo "Connected to $DEVICE_COUNT device(s)"
echo ""

# Create temporary directory for crash dumps
TEMP_DIR=$(mktemp -d -t tizen-crash-XXXXXX)
trap 'rm -rf "$TEMP_DIR"' EXIT

echo "Pulling crash dumps from device..."
echo ""

# Pull crash files from device
if ! $SDB_CMD pull "$CRASH_REMOTE_DIR" "$TEMP_DIR/" 2>/dev/null; then
    echo "WARNING: Could not pull crash files from device."
    echo "The device may not have any crash dumps."
    exit 0
fi

found_any=false

# Process all zip files
for zip_file in $(find "$TEMP_DIR" -name "*.zip" 2>/dev/null); do
    found_any=true
    fname=$(basename "$zip_file")
    app_id="${fname%%_*}"
    binary=$(resolve_binary_name "$app_id")
    
    extract_dir="${zip_file%.zip}"
    mkdir -p "$extract_dir"
    
    echo "Processing: $fname"
    unzip -qo "$zip_file" -d "$extract_dir"
    
    tar_file=$(find "$extract_dir" -name "*.coredump.tar" 2>/dev/null | head -n 1)
    if [ -n "$tar_file" ]; then
        tar --ignore-zeros -xf "$tar_file" -C "$extract_dir" 2>/dev/null || true
    fi
    
    coredump=$(find_coredump_file "$extract_dir")
    
    if [ -n "$coredump" ] && [ -s "$coredump" ]; then
        run_gdb_analysis "$extract_dir" "$binary" "$coredump" "$fname"
    else
        echo "WARNING: No valid coredump found in $fname"
    fi
    
    echo ""
done

if [ "$found_any" = false ]; then
    echo "No crash dumps found on device."
fi
