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
LOCAL_SYSROOT="out/tizen-crash-sysroot"
SEARCH_HOURS=1

# ============================================================================
# Utility Functions
# ============================================================================

function help() {
    cat <<EOF
Usage:
$0 [--help] [--hours NUM] [--target SDB_ID]

Options:
    --hours NUM     - Filter crashes from the last NUM hours (default: 1)
    --target SDB_ID - SDB identifier (e.g. 192.168.0.118:26101)
    --help          - Print help
EOF
}

function parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            --hours)
                SEARCH_HOURS="$2"
                shift 2
                ;;
            --target)
                TARGET_DEVICE="$2"
                shift 2
                ;;
            --help)
                help
                exit 0
                ;;
            *)
                echo "ERROR: Unknown option $1"
                help
                exit 1
                ;;
        esac
    done
}

function setup_sdb() {
    SDB_CMD=("sdb")
    if [ -n "$TARGET_DEVICE" ]; then
        SDB_CMD=("sdb" "-s" "$TARGET_DEVICE")
    fi

    echo "Connecting to target device via SDB..."
    "${SDB_CMD[@]}" root on
}

# ============================================================================
# Crash Log Discovery
# ============================================================================

function fetch_recent_crashes() {
    local target_now cutoff_time

    target_now=$("${SDB_CMD[@]}" shell "date +%s" | tr -d '\r' | grep -E '^[0-9]+$')
    if [ -z "$target_now" ]; then
        echo "ERROR: Failed to retrieve current time from target device."
        exit 1
    fi

    cutoff_time=$((target_now - SEARCH_HOURS * 3600))

    echo "Fetching crash logs from the last $SEARCH_HOURS hour(s)..."
    local remote_files
    remote_files=$("${SDB_CMD[@]}" shell "ls $CRASH_REMOTE_DIR/*.zip 2>/dev/null" | tr -d '\r')

    VALID_CRASHES=()
    for remote_zip in $remote_files; do
        [ -n "$remote_zip" ] || continue
        [[ "$remote_zip" == *"No such file"* ]] && continue

        local filename timestamp_str formatted_time file_epoch
        filename=$(basename "$remote_zip")
        # Extract timestamp from filename format: APPID_PID_YYYYMMDDHHMMSS.zip
        timestamp_str=$(echo "$filename" | grep -oE '[0-9]{14}')

        if [ -n "$timestamp_str" ]; then
            # Convert YYYYMMDDHHMMSS to Unix epoch
            formatted_time="${timestamp_str:0:4}-${timestamp_str:4:2}-${timestamp_str:6:2} ${timestamp_str:8:2}:${timestamp_str:10:2}:${timestamp_str:12:2}"
            file_epoch=$(date -d "$formatted_time" +%s 2>/dev/null || echo 0)

            if [ "$file_epoch" -ge "$cutoff_time" ]; then
                VALID_CRASHES+=("$remote_zip")
            fi
        fi
    done
}

function select_crash_log() {
    if [ ${#VALID_CRASHES[@]} -eq 0 ]; then
        echo "No crash logs found within the specified time window."
        exit 0
    fi

    if [ ${#VALID_CRASHES[@]} -eq 1 ]; then
        SELECTED_ZIP="${VALID_CRASHES[0]}"
        echo "Found exactly one crash log: $(basename "$SELECTED_ZIP")"
        return
    fi

    echo ""
    echo "Available crash logs from the last $SEARCH_HOURS hour(s):"
    echo "------------------------------------------------------------"
    for i in "${!VALID_CRASHES[@]}"; do
        echo "[$i] $(basename "${VALID_CRASHES[$i]}")"
    done
    echo "------------------------------------------------------------"

    while true; do
        read -r -p "Select a crash log to analyze [0-$(( ${#VALID_CRASHES[@]} - 1 ))]: " choice
        if [[ "$choice" =~ ^[0-9]+$ ]] && [ "$choice" -lt ${#VALID_CRASHES[@]} ]; then
            SELECTED_ZIP="${VALID_CRASHES[$choice]}"
            break
        fi
        echo "Invalid selection. Please try again."
    done
}

# ============================================================================
# Binary Resolution
# ============================================================================

function resolve_binary_name() {
    local app_id="$1"

    # Lookup app_id -> exec in tizen-manifest.xml files
    for manifest in examples/*/tizen/tizen-manifest.xml; do
        [ -f "$manifest" ] || continue
        local manifest_appid
        manifest_appid=$(sed -n 's/.*appid="\([^"]*\)".*/\1/p' "$manifest" | head -1)
        if [ "$manifest_appid" = "$app_id" ]; then
            sed -n 's/.*exec="\([^"]*\)".*/\1/p' "$manifest" | head -1
            return
        fi
    done

    # Fallback: use app_id as binary name
    echo "$app_id"
}

# ============================================================================
# Crash Archive Extraction
# ============================================================================

function pull_and_extract_crash() {
    local remote_zip="$1"
    local filename
    filename=$(basename "$remote_zip")

    mkdir -p "$LOCAL_TMP_DIR"

    local local_zip="$LOCAL_TMP_DIR/$filename"
    echo "Pulling $filename from device..."
    "${SDB_CMD[@]}" pull "$remote_zip" "$local_zip"

    echo "Extracting crash archive..."
    unzip -qo "$local_zip" -d "$LOCAL_TMP_DIR"

    # Handle inner tar structure if present inside the Tizen zip layout
    local archive
    mapfile -t nested_tars < <(find "$LOCAL_TMP_DIR" -type f \( -name "*.tar" -o -name "*.tar.gz" \) 2>/dev/null)
    for archive in "${nested_tars[@]}"; do
        if file "$archive" | grep -qE 'tar archive|gzip compressed'; then
            tar -xf "$archive" -C "$LOCAL_TMP_DIR"
        fi
    done
}

function find_coredump() {
    local coredump=""

    # Find by file signature first (most reliable)
    local f
    mapfile -t all_files < <(find "$LOCAL_TMP_DIR" -type f)
    for f in "${all_files[@]}"; do
        if file "$f" | grep -q 'core file'; then
            echo "$f"
            return
        fi
    done

    # Fallback: find by extension
    coredump=$(find "$LOCAL_TMP_DIR" -type f -name "*.coredump" | head -n 1)
    if [ -n "$coredump" ]; then
        echo "$coredump"
        return
    fi

    echo ""
}

# ============================================================================
# Architecture Detection
# ============================================================================

function detect_core_is_64bit() {
    local coredump="$1"
    local core_info
    core_info=$(file "$coredump")

    if [[ "$core_info" == *"aarch64"* ]] || [[ "$core_info" == *"64-bit"* ]]; then
        echo "true"
    else
        echo "false"
    fi
}

# ============================================================================
# Build Target Matching
# ============================================================================

function find_matching_targets() {
    local binary="$1"
    local is_core_64="$2"

    local matching=()
    local target
    for target in out/tizen-*; do
        [ -d "$target" ] || continue
        [ -f "$target/$binary" ] || continue

        # Enforce architecture alignment between core file and build target
        if [ "$is_core_64" = true ]; then
            [[ "$target" != *"arm64"* ]] && [[ "$target" != *"aarch64"* ]] && continue
        else
            [[ "$target" == *"arm64"* ]] && continue
            [[ "$target" == *"aarch64"* ]] && continue
        fi

        matching+=("$target")
    done

    printf '%s\n' "${matching[@]}"
}

function select_build_target() {
    local binary="$1"
    shift
    local targets=("$@")

    if [ ${#targets[@]} -eq 0 ]; then
        echo "ERROR: Could not find a matching local unstripped binary for '$binary' with correct architecture inside out/tizen-* directories."
        rm -rf "$LOCAL_TMP_DIR"
        exit 1
    fi

    if [ ${#targets[@]} -eq 1 ]; then
        echo "Found matching build target: ${targets[0]}"
        SELECTED_TARGET="${targets[0]}"
        return
    fi

    echo ""
    echo "Multiple build targets contain binary '$binary' with matching architecture:"
    echo "------------------------------------------------------------"
    for i in "${!targets[@]}"; do
        echo "[$i] ${targets[$i]}"
    done
    echo "------------------------------------------------------------"

    while true; do
        read -r -p "Select a build target [0-$(( ${#targets[@]} - 1 ))]: " choice
        if [[ "$choice" =~ ^[0-9]+$ ]] && [ "$choice" -lt ${#targets[@]} ]; then
            SELECTED_TARGET="${targets[$choice]}"
            break
        fi
        echo "Invalid selection. Please try again."
    done
}

# ============================================================================
# GDB Setup
# ============================================================================

function get_gdb_binary() {
    local target_path="$1"

    if [[ "$target_path" == *"arm64"* ]] || [[ "$target_path" == *"aarch64"* ]]; then
        echo "$TIZEN_SDK_ROOT/tools/aarch64-linux-gnu-gcc-14.2/bin/aarch64-linux-gnu-gdb"
    else
        echo "$TIZEN_SDK_ROOT/tools/arm-linux-gnueabi-gcc-14.2/bin/arm-linux-gnueabi-gdb"
    fi
}

function set_sysroot_env() {
    local target_path="$1"

    if [[ "$target_path" == *"arm64"* ]] || [[ "$target_path" == *"aarch64"* ]]; then
        export TIZEN_SDK_SYSROOT="$TIZEN_SDK_ROOT/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device64.core"
    else
        export TIZEN_SDK_SYSROOT="$TIZEN_SDK_ROOT/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device.core"
    fi
}

# ============================================================================
# Local Sysroot Builder
# ============================================================================

function build_local_sysroot() {
    local coredump="$1"

    echo "Building local sysroot from device shared libraries..."

    # Extract mapped file paths from the coredump's NT_FILE notes
    # readelf -n output format: "  0xADDR  0xADDR  0xOFFSET  /path/to/file"
    local core_libs
    mapfile -t core_libs < <(readelf -n "$coredump" 2>/dev/null | grep -oE '/[^ ]+' | sort -u)

    if [ ${#core_libs[@]} -eq 0 ]; then
        echo "WARNING: Could not parse coredump file mappings. Backtrace may be incomplete."
        echo "Consider pulling /usr/lib64 from the device manually into a sysroot."
        SYSROOT_TO_USE="$TIZEN_SDK_SYSROOT"
        return
    fi

    local pulled_count=0 cached_count=0
    local lib_path local_lib
    for lib_path in "${core_libs[@]}"; do
        # Skip non-library paths
        [[ "$lib_path" == /dev/* ]] && continue
        [[ "$lib_path" == /proc/* ]] && continue
        [[ "$lib_path" == /sys/* ]] && continue

        local_lib="$LOCAL_SYSROOT$lib_path"
        if [ -f "$local_lib" ]; then
            cached_count=$((cached_count + 1))
            continue
        fi

        mkdir -p "$(dirname "$local_lib")"
        if "${SDB_CMD[@]}" pull "$lib_path" "$local_lib" 2>/dev/null; then
            pulled_count=$((pulled_count + 1))
        fi
    done

    echo "Sysroot: $pulled_count libraries pulled, $cached_count cached from previous runs."
    SYSROOT_TO_USE="$LOCAL_SYSROOT"
}

# ============================================================================
# GDB Analysis
# ============================================================================

function run_gdb_analysis() {
    local gdb_bin="$1"
    local sysroot="$2"
    local binary_path="$3"
    local coredump="$4"
    local crash_filename="$5"

    echo "----------------------------------------------------------------------------------------------------"
    echo "ANALYZING CRASH: $crash_filename"
    echo "TARGET BINARY:  $binary_path"
    echo "REAL COREDUMP:  $coredump"
    echo "USING GDB:      $gdb_bin"
    echo "SYSROOT:        $sysroot"
    echo "----------------------------------------------------------------------------------------------------"

    "$gdb_bin" --batch \
        -ex "set auto-load safe-path /" \
        -ex "set sysroot $sysroot" \
        -ex "set solib-absolute-prefix $sysroot" \
        -ex "file $binary_path" \
        -ex "core-file $coredump" \
        -ex "thread apply all bt full"
}

# ============================================================================
# Main
# ============================================================================

parse_arguments "$@"
setup_sdb

fetch_recent_crashes
select_crash_log

# Resolve crash app_id to binary name
crash_filename=$(basename "$SELECTED_ZIP")
app_id="${crash_filename%%_*}"
binary=$(resolve_binary_name "$app_id")

pull_and_extract_crash "$SELECTED_ZIP"

coredump=$(find_coredump)
if [ -z "$coredump" ] || [ ! -f "$coredump" ]; then
    echo "ERROR: Could not find a valid core dump file inside the pulled archive."
    rm -rf "$LOCAL_TMP_DIR"
    exit 1
fi

is_core_64=$(detect_core_is_64bit "$coredump")

mapfile -t matching_targets < <(find_matching_targets "$binary" "$is_core_64")
select_build_target "$binary" "${matching_targets[@]}"

gdb_bin=$(get_gdb_binary "$SELECTED_TARGET")
set_sysroot_env "$SELECTED_TARGET"

build_local_sysroot "$coredump"

run_gdb_analysis "$gdb_bin" "$SYSROOT_TO_USE" "$SELECTED_TARGET/$binary" "$coredump" "$crash_filename"

# Cleanup temporary files (keep sysroot cached for future runs)
rm -rf "$LOCAL_TMP_DIR"
