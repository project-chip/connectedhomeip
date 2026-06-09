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
SELECT_LAST=false
# Target device time (set during fetch_recent_crashes, used for time-ago display)
TARGET_NOW=0

# ============================================================================
# Utility Functions
# ============================================================================

function help() {
    cat <<EOF
Usage:
$0 [--help] [--hours NUM] [--target SDB_ID] [--out-dir DIR] [--last] [--verbose] [--no-clean]

Options:
    --hours NUM     - Filter crashes from the last NUM hours (default: 24)
    --target SDB_ID - SDB identifier (e.g. 192.168.0.118:26101)
    --out-dir DIR   - Build output directory (e.g. out/tizen-arm64-light-no-thread).
                      When set, only crashes matching binaries in this directory
                      are shown and build target selection is skipped.
    --last          - Automatically select the most recent crash log
    --verbose       - Print commands before execution (especially GDB invocation)
    --no-clean      - Keep temporary files (pulled crash archives) after analysis
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
            --out-dir)
                OUT_DIR="$2"
                shift 2
                ;;
            --verbose)
                VERBOSE=true
                shift
                ;;
            --no-clean)
                CLEANUP=false
                shift
                ;;
            --last)
                SELECT_LAST=true
                shift
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
    if ! command -v sdb >/dev/null 2>&1; then
        echo "ERROR: 'sdb' command not found. Please install Tizen SDK or add sdb to your PATH."
        exit 1
    fi

    SDB_CMD=("sdb")
    if [ "$TARGET_DEVICE" != "" ]; then
        SDB_CMD=("sdb" "-s" "$TARGET_DEVICE")
    fi

    echo "Connecting to target device via SDB..."
    "${SDB_CMD[@]}" root on
}

function verbose_log() {
    if [ "$VERBOSE" = true ]; then
        echo "+ $*"
    fi
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

function list_binaries_in_out_dir() {
    local out_dir="$1"
    local f
    for f in "$out_dir"/*; do
        [ -f "$f" ] || continue
        local basename
        basename=$(basename "$f")
        [[ "$basename" == *.txt ]] && continue
        [[ "$basename" == *.json ]] && continue
        [[ "$basename" == *.zip ]] && continue
        [[ "$basename" == *.log ]] && continue
        if file "$f" | grep -q 'executable'; then
            echo "$basename"
        fi
    done
}

# ============================================================================
# Crash Log Discovery
# ============================================================================

function format_time_ago() {
    local epoch="$1"
    local diff=$((TARGET_NOW - epoch))
    if [ "$diff" -lt 0 ]; then
        diff=0
    fi
    local hours=$((diff / 3600))
    local minutes=$(((diff % 3600) / 60))
    printf "%dh%02dm ago" "$hours" "$minutes"
}

function fetch_recent_crashes() {
    # Get device time as both Unix epoch and formatted string to compute timezone offset
    TARGET_NOW=$("${SDB_CMD[@]}" shell "date +%s" | tr -d '\r' | grep -E '^[0-9]+$' || true)
    if [ "$TARGET_NOW" = "" ]; then
        echo "ERROR: Failed to retrieve current time from target device."
        exit 1
    fi

    local device_datetime
    device_datetime=$("${SDB_CMD[@]}" shell "date '+%Y-%m-%d %H:%M:%S'" | tr -d '\r')
    # Compute offset: device epoch vs host's interpretation of device's local time
    local host_epoch=0
    if date --version >/dev/null 2>&1; then
        host_epoch=$(date -d "$device_datetime" +%s 2>/dev/null || echo 0)
    else
        host_epoch=$(date -j -f "%Y-%m-%d %H:%M:%S" "$device_datetime" +%s 2>/dev/null || echo 0)
    fi
    local tz_offset=$((TARGET_NOW - host_epoch))

    local cutoff_time=$((TARGET_NOW - SEARCH_HOURS * 3600))

    echo "Fetching crash logs from the last $SEARCH_HOURS hour(s)..."
    local remote_files_raw
    remote_files_raw=$("${SDB_CMD[@]}" shell "ls $CRASH_REMOTE_DIR/*.zip 2>/dev/null" | tr -d '\r')

    # Split into proper array for word-safe iteration
    local remote_files=()
    readarray -t remote_files <<<"$remote_files_raw"

    # Collect crashes as "EPOCH:PATH" pairs for sorting
    local crash_pairs=()
    for remote_zip in "${remote_files[@]}"; do
        [ "$remote_zip" != "" ] || continue
        [[ "$remote_zip" == *"No such file"* ]] && continue

        local filename timestamp_str formatted_time file_epoch
        filename=$(basename "$remote_zip")
        # Extract timestamp from filename format: APPID_PID_YYYYMMDDHHMMSS.zip
        timestamp_str=$(echo "$filename" | grep -oE '[0-9]{14}' || true)

        if [ "$timestamp_str" != "" ]; then
            formatted_time="${timestamp_str:0:4}-${timestamp_str:4:2}-${timestamp_str:6:2} ${timestamp_str:8:2}:${timestamp_str:10:2}:${timestamp_str:12:2}"
            # Parse on host (host timezone) then apply device timezone offset
            file_epoch=$(date -d "$formatted_time" +%s 2>/dev/null || echo 0)
            file_epoch=$((file_epoch + tz_offset))

            if [ "$file_epoch" -ge "$cutoff_time" ]; then
                crash_pairs+=("$file_epoch:$remote_zip")
            fi
        fi
    done

    # Sort newest first (descending by epoch)
    mapfile -t sorted_pairs < <(printf '%s\n' "${crash_pairs[@]}" | sort -t: -k1,1nr)

    VALID_CRASHES=()
    CRASH_EPOCHS=()
    local pair
    for pair in "${sorted_pairs[@]}"; do
        [ "$pair" != "" ] || continue
        CRASH_EPOCHS+=("${pair%%:*}")
        VALID_CRASHES+=("${pair#*:}")
    done
}

function filter_crashes_for_out_dir() {
    local out_dir="$1"

    mapfile -t available_binaries < <(list_binaries_in_out_dir "$out_dir")
    if [ ${#available_binaries[@]} -eq 0 ]; then
        echo "WARNING: No ELF executables found in $out_dir"
        return
    fi

    local filtered_crashes=()
    local filtered_epochs=()
    for i in "${!VALID_CRASHES[@]}"; do
        local remote_zip="${VALID_CRASHES[$i]}"
        local filename app_id binary
        filename=$(basename "$remote_zip")
        app_id="${filename%%_*}"
        binary=$(resolve_binary_name "$app_id")

        local b
        for b in "${available_binaries[@]}"; do
            if [ "$b" = "$binary" ]; then
                filtered_crashes+=("$remote_zip")
                filtered_epochs+=("${CRASH_EPOCHS[$i]}")
                break
            fi
        done
    done

    VALID_CRASHES=("${filtered_crashes[@]}")
    CRASH_EPOCHS=("${filtered_epochs[@]}")
}

function filter_matter_crashes() {
    # Remove crashes that don't correspond to any known Matter binary
    # in the out/tizen-* build directories
    local filtered_crashes=()
    local filtered_epochs=()

    for i in "${!VALID_CRASHES[@]}"; do
        local remote_zip="${VALID_CRASHES[$i]}"
        local filename app_id binary found
        filename=$(basename "$remote_zip")
        app_id="${filename%%_*}"
        binary=$(resolve_binary_name "$app_id")

        found=false
        local target
        for target in out/tizen-*/"$binary"; do
            if [ -f "$target" ]; then
                found=true
                break
            fi
        done

        if "$found"; then
            filtered_crashes+=("$remote_zip")
            filtered_epochs+=("${CRASH_EPOCHS[$i]}")
        fi
    done

    VALID_CRASHES=("${filtered_crashes[@]}")
    CRASH_EPOCHS=("${filtered_epochs[@]}")
}

function select_crash_log() {
    if [ ${#VALID_CRASHES[@]} -eq 0 ]; then
        echo "No crash logs found within the specified time window."
        exit 0
    fi

    if [ ${#VALID_CRASHES[@]} -eq 1 ] || [ "$SELECT_LAST" = true ]; then
        SELECTED_ZIP="${VALID_CRASHES[0]}"
        local time_ago
        time_ago=$(format_time_ago "${CRASH_EPOCHS[0]}")
        echo "Selected crash log: $(basename "$SELECTED_ZIP") ($time_ago)"
        return
    fi

    echo ""
    echo "Available crash logs from the last $SEARCH_HOURS hour(s):"
    echo "------------------------------------------------------------"
    for i in "${!VALID_CRASHES[@]}"; do
        local time_ago
        time_ago=$(format_time_ago "${CRASH_EPOCHS[$i]}")
        echo "[$i] $(basename "${VALID_CRASHES[$i]}")  ($time_ago)"
    done
    echo "------------------------------------------------------------"

    while true; do
        read -r -p "Select a crash log to analyze [0-$((${#VALID_CRASHES[@]} - 1))]: " choice
        if [[ "$choice" =~ ^[0-9]+$ ]] && [ "$choice" -lt ${#VALID_CRASHES[@]} ]; then
            SELECTED_ZIP="${VALID_CRASHES[$choice]}"
            break
        fi
        echo "Invalid selection. Please try again."
    done
}

# ============================================================================
# Crash Archive Extraction
# ============================================================================

function pull_and_extract_crash() {
    local remote_zip="$1"
    local filename
    filename=$(basename "$remote_zip")

    # Clean stale tmp files from previous runs to avoid coredump confusion
    rm -rf "$LOCAL_TMP_DIR"
    mkdir -p "$LOCAL_TMP_DIR"

    local local_zip="$LOCAL_TMP_DIR/$filename"
    echo "Pulling $filename from device..."
    "${SDB_CMD[@]}" pull "$remote_zip" "$local_zip"

    echo "Extracting crash archive..."
    unzip -qo "$local_zip" -d "$LOCAL_TMP_DIR"

    local archive
    mapfile -t nested_tars < <(find "$LOCAL_TMP_DIR" -type f \( -name "*.tar" -o -name "*.tar.gz" \) 2>/dev/null)
    for archive in "${nested_tars[@]}"; do
        if file "$archive" | grep -qE 'tar archive|gzip compressed'; then
            tar -xf "$archive" -C "$LOCAL_TMP_DIR"
        fi
    done
}

function find_coredump() {
    local f
    mapfile -t all_files < <(find "$LOCAL_TMP_DIR" -type f)
    for f in "${all_files[@]}"; do
        if file "$f" | grep -q 'core file'; then
            echo "$f"
            return
        fi
    done

    local coredump
    coredump=$(find "$LOCAL_TMP_DIR" -type f -name "*.coredump" | head -n 1)
    if [ "$coredump" != "" ]; then
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
        if [ "$CLEANUP" = true ]; then rm -rf "$LOCAL_TMP_DIR"; fi
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
        read -r -p "Select a build target [0-$((${#targets[@]} - 1))]: " choice
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

function get_sdk_sysroot() {
    local target_path="$1"

    if [[ "$target_path" == *"arm64"* ]] || [[ "$target_path" == *"aarch64"* ]]; then
        echo "$TIZEN_SDK_ROOT/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device64.core"
    else
        echo "$TIZEN_SDK_ROOT/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device.core"
    fi
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

    # Build the GDB command line with proper quoting for copy-paste
    local gdb_args=(
        "--batch"
        "-ex" "set auto-load safe-path /"
        "-ex" "set sysroot $sysroot"
        "-ex" "set solib-absolute-prefix $sysroot"
        "-ex" "file $binary_path"
        "-ex" "core-file $coredump"
        "-ex" "thread apply all bt full"
    )

    verbose_log "$gdb_bin ${gdb_args[*]}"

    "$gdb_bin" "${gdb_args[@]}"
}

# ============================================================================
# Main
# ============================================================================

parse_arguments "$@"
setup_sdb

if [ "$OUT_DIR" != "" ]; then
    if [ ! -d "$OUT_DIR" ]; then
        echo "ERROR: --out-dir directory does not exist: $OUT_DIR"
        exit 1
    fi
    echo "Using build output directory: $OUT_DIR"
fi

fetch_recent_crashes

if [ "$OUT_DIR" != "" ]; then
    filter_crashes_for_out_dir "$OUT_DIR"
else
    filter_matter_crashes
fi

select_crash_log

crash_filename=$(basename "$SELECTED_ZIP")
app_id="${crash_filename%%_*}"
binary=$(resolve_binary_name "$app_id")

pull_and_extract_crash "$SELECTED_ZIP"

coredump=$(find_coredump)
if [ "$coredump" = "" ] || [ ! -f "$coredump" ]; then
    echo "ERROR: Could not find a valid core dump file inside the pulled archive."
    "$CLEANUP" && rm -rf "$LOCAL_TMP_DIR"
    exit 1
fi

is_core_64=$(detect_core_is_64bit "$coredump")

if [ "$OUT_DIR" != "" ]; then
    if [ ! -f "$OUT_DIR/$binary" ]; then
        echo "ERROR: Binary '$binary' not found in $OUT_DIR"
        "$CLEANUP" && rm -rf "$LOCAL_TMP_DIR"
        exit 1
    fi
    SELECTED_TARGET="$OUT_DIR"
else
    mapfile -t matching_targets < <(find_matching_targets "$binary" "$is_core_64")
    select_build_target "$binary" "${matching_targets[@]}"
fi

gdb_bin=$(get_gdb_binary "$SELECTED_TARGET")
sysroot=$(get_sdk_sysroot "$SELECTED_TARGET")

run_gdb_analysis "$gdb_bin" "$sysroot" "$SELECTED_TARGET/$binary" "$coredump" "$crash_filename"

if "$CLEANUP"; then
    rm -rf "$LOCAL_TMP_DIR"
else
    echo "Temporary files preserved in: $LOCAL_TMP_DIR"
fi
