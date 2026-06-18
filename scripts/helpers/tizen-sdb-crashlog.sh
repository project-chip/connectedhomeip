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
# Tizen SDB Crashlog Analyzer
# ============================================================================
#
# This script analyzes crash dumps from physical Tizen devices connected via
# SDB (Smart Development Bridge). It pulls crash-manager archives (zip format)
# from the device, extracts coredumps, and runs GDB analysis.
#
# Features:
#   - Interactive or automatic SDB device selection
#   - Time-based crash filtering (--hours)
#   - Interactive crash log selection with time-ago display
#   - Automatic or interactive build target matching
#   - Architecture-aware GDB setup
#
# For analyzing raw core dumps from QEMU tests, use
# scripts/helpers/tizen-crashlog.sh instead.
#
# ============================================================================

set -e

# Configuration
CRASH_REMOTE_DIR="/opt/usr/share/crash/dump"
LOCAL_TMP_DIR="out/tizen-crashes-tmp"
SEARCH_HOURS=24
CLEANUP=true
VERBOSE=false
SELECT_LAST=false
SYSROOT_PATH=""
# Target device time (set during fetch_recent_crashes, used for time-ago display)
TARGET_NOW=0

# ============================================================================
# Utility Functions
# ============================================================================

function help() {
    cat <<EOF
Usage: $0 [options]

Analyze core dumps from Tizen devices via SDB (crash-manager format).

Options:
    --target SDB_ID   - SDB identifier (e.g. 192.168.0.1:26101)
                        When omitted and multiple devices are connected,
                        an interactive selection is shown.
    --hours NUM       - Filter crashes from the last NUM hours (default: 24)
    --out-dir DIR     - Build output directory (e.g. out/tizen-arm64-light-no-thread).
                        When set, only crashes matching binaries in this directory
                        are shown and build target selection is skipped.
    --sysroot PATH    - Path to system libraries sysroot (overrides auto-detection)
    --last            - Automatically select the most recent crash log
    --verbose         - Print GDB command for manual execution
    --no-clean        - Keep temporary files (pulled crash archives) after analysis
    --help            - Print help

Examples:
    $0                                              # Interactive: select device & crash
    $0 --last                                       # Analyze most recent crash
    $0 --target 192.168.0.1:26101                   # Use specific SDB device
    $0 --out-dir out/tizen-arm64-light-no-thread    # Match crashes to specific build
    $0 --hours 1                                    # Show crashes from last hour only
    $0 --sysroot /path/to/sysroot                   # Use custom sysroot
    $0 --no-clean                                   # Keep temp files for inspection
EOF
}

function parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            --target) TARGET_DEVICE="$2"; shift 2 ;;
            --hours) SEARCH_HOURS="$2"; shift 2 ;;
            --out-dir) OUT_DIR="$2"; shift 2 ;;
            --sysroot) SYSROOT_PATH="$2"; shift 2 ;;
            --last) SELECT_LAST=true; shift ;;
            --verbose) VERBOSE=true; shift ;;
            --no-clean) CLEANUP=false; shift ;;
            --help) help; exit 0 ;;
            *) echo "ERROR: Unknown option $1"; help; exit 1 ;;
        esac
    done
}

function verbose_log() {
    if [ "$VERBOSE" = true ]; then
        echo "+ $*"
    fi
}

# ============================================================================
# SDB Setup & Device Selection
# ============================================================================

function setup_sdb() {
    if ! command -v sdb >/dev/null 2>&1; then
        echo "ERROR: 'sdb' command not found. Please install Tizen SDK or add sdb to your PATH."
        exit 1
    fi

    # If target device is specified, use it directly
    if [ -n "$TARGET_DEVICE" ]; then
        SDB_CMD=("sdb" "-s" "$TARGET_DEVICE")
        echo "Connecting to target device $TARGET_DEVICE via SDB..."
        "${SDB_CMD[@]}" root on
        return
    fi

    # Discover connected devices
    local device_lines
    mapfile -t device_lines < <(sdb devices 2>/dev/null | grep -v 'List of devices attached' | grep -v '^$')

    local device_count=${#device_lines[@]}
    if [ "$device_count" -eq 0 ]; then
        echo "ERROR: No SDB device found."
        echo "Connect a device or specify --target <device_id>"
        exit 1
    fi

    # Single device: use it automatically
    if [ "$device_count" -eq 1 ]; then
        local device_id
        device_id=$(echo "${device_lines[0]}" | awk '{print $1}')
        SDB_CMD=("sdb" "-s" "$device_id")
        echo "Connected to device: $device_id"
        "${SDB_CMD[@]}" root on
        return
    fi

    # Multiple devices: interactive selection
    echo ""
    echo "Multiple SDB devices connected:"
    echo "------------------------------------------------------------"
    for i in "${!device_lines[@]}"; do
        echo "[$i] ${device_lines[$i]}"
    done
    echo "------------------------------------------------------------"

    while true; do
        read -r -p "Select a device [0-$(( ${#device_lines[@]} - 1 ))]: " choice
        if [[ "$choice" =~ ^[0-9]+$ ]] && [ "$choice" -lt "${#device_lines[@]}" ]; then
            local device_id
            device_id=$(echo "${device_lines[$choice]}" | awk '{print $1}')
            SDB_CMD=("sdb" "-s" "$device_id")
            break
        fi
        echo "Invalid selection. Please try again."
    done

    echo "Connecting to selected device via SDB..."
    "${SDB_CMD[@]}" root on
}

# ============================================================================
# Binary Resolution
# ============================================================================

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
    # Search in root and tests/ subfolder
    if [[ -f "$target_dir/$binary" ]]; then echo "$target_dir/$binary"
    elif [[ -f "$target_dir/tests/$binary" ]]; then echo "$target_dir/tests/$binary"
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
    if [ -z "$TARGET_NOW" ]; then
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
        [ -n "$remote_zip" ] || continue
        [[ "$remote_zip" == *"No such file"* ]] && continue

        local filename timestamp_str formatted_time file_epoch
        filename=$(basename "$remote_zip")
        # Extract timestamp from filename format: APPID_PID_YYYYMMDDHHMMSS.zip
        timestamp_str=$(echo "$filename" | grep -oE '[0-9]{14}' || true)

        if [ -n "$timestamp_str" ]; then
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
        [ -n "$pair" ] || continue
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
        read -r -p "Select a crash log to analyze [0-$(( ${#VALID_CRASHES[@]} - 1 ))]: " choice
        if [[ "$choice" =~ ^[0-9]+$ ]] && [ "$choice" -lt ${#VALID_CRASHES[@]} ]; then
            SELECTED_ZIP="${VALID_CRASHES[$choice]}"
            break
        fi
        echo "Invalid selection. Please try again."
    done
}

# ============================================================================
# System Library Pulling from Device
# ============================================================================

function pull_system_libs() {
    local target_dir="$1"
    local sysroot_dir="$target_dir/system_libs"

    # Already pulled? Skip
    if [ -d "$sysroot_dir/usr/lib64" ] || [ -d "$sysroot_dir/usr/lib" ]; then
        echo "System libraries already present in $sysroot_dir"
        echo "$sysroot_dir"
        return
    fi

    echo "Pulling system libraries from device (this may take a moment)..."

    if [[ "$target_dir" == *"arm64"* || "$target_dir" == *"aarch64"* ]]; then
        mkdir -p "$sysroot_dir/usr/lib64" "$sysroot_dir/lib64"
        # Pull essential system libraries for 64-bit Tizen
        "${SDB_CMD[@]}" pull /usr/lib64 "$sysroot_dir/usr/lib64" 2>/dev/null || true
        "${SDB_CMD[@]}" pull /lib64 "$sysroot_dir/lib64" 2>/dev/null || true
    else
        mkdir -p "$sysroot_dir/usr/lib" "$sysroot_dir/lib"
        # Pull essential system libraries for 32-bit Tizen
        "${SDB_CMD[@]}" pull /usr/lib "$sysroot_dir/usr/lib" 2>/dev/null || true
        "${SDB_CMD[@]}" pull /lib "$sysroot_dir/lib" 2>/dev/null || true
    fi

    # Create a symlink so GDB can resolve /mnt/chip/ paths through the sysroot
    mkdir -p "$sysroot_dir/mnt"
    ln -sf "../../" "$sysroot_dir/mnt/chip" 2>/dev/null || true

    echo "$sysroot_dir"
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
    local dir="$1"
    # First try: find by .coredump extension
    local f
    f=$(find "$dir" -type f -name "*.coredump" -size +0 2>/dev/null | head -1)
    if [ -n "$f" ]; then
        echo "$f"
        return
    fi
    # Second try: detect by file magic
    mapfile -t all_files < <(find "$dir" -type f -size +0 2>/dev/null)
    for f in "${all_files[@]}"; do
        if file "$f" | grep -q 'core file'; then
            echo "$f"
            return
        fi
    done
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
        # Check for binary in root and tests/ subfolder
        local binary_path
        binary_path=$(resolve_binary_path "$target" "$binary")
        [ -n "$binary_path" ] || continue

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
        read -r -p "Select a build target [0-$(( ${#targets[@]} - 1 ))]: " choice
        if [[ "$choice" =~ ^[0-9]+$ ]] && [ "$choice" -lt ${#targets[@]} ]; then
            SELECTED_TARGET="${targets[$choice]}"
            break
        fi
        echo "Invalid selection. Please try again."
    done
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
            # Only use SDK sysroot if it actually exists and contains libs
            if [ -d "$TIZEN_SDK_ROOT/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device64.core/usr/lib64" ] || \
               [ -d "$TIZEN_SDK_ROOT/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device64.core/lib64" ]; then
                sysroot="$TIZEN_SDK_ROOT/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device64.core"
            fi
        else
            gdb_bin="$TIZEN_SDK_ROOT/tools/arm-linux-gnueabi-gcc-14.2/bin/arm-linux-gnueabi-gdb"
            if [ -d "$TIZEN_SDK_ROOT/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device.core/usr/lib" ] || \
               [ -d "$TIZEN_SDK_ROOT/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device.core/lib" ]; then
                sysroot="$TIZEN_SDK_ROOT/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device.core"
            fi
        fi
    fi

    if [ -n "$sysroot" ]; then
        echo "SYSROOT:         $sysroot"
    else
        echo "SYSROOT:         (none)"
    fi
    echo "--------------------------------------------------------------------------------"

    local gdb_args=("--batch" "-ex" "set auto-load safe-path /")
    if [ -d "$sysroot" ]; then
        gdb_args+=("-ex" "set sysroot $sysroot" "-ex" "set solib-absolute-prefix $sysroot")
    fi

    # Map Tizen device paths to host paths for GDB file-backed mappings.
    # On a Tizen device:
    #   - Matter apps are installed at /opt/usr/globalapps/<appid>/bin/<binary>
    #   - System libraries are in /usr/lib64/ and /lib64/ (64-bit) or /usr/lib/ and /lib/ (32-bit)
    #   - QEMU test paths use /mnt/chip/
    gdb_args+=("-ex" "set substitute-path /mnt/chip/ $target_dir/")
    # Map the installed app binary path to local build directory
    gdb_args+=("-ex" "set substitute-path /opt/usr/globalapps/ $target_dir/")
    if [ -d "$sysroot" ]; then
        if [[ "$target_dir" == *"arm64"* || "$target_dir" == *"aarch64"* ]]; then
            gdb_args+=("-ex" "set substitute-path /usr/lib64/ $sysroot/usr/lib64/")
            gdb_args+=("-ex" "set substitute-path /usr/lib/ $sysroot/usr/lib64/")
            gdb_args+=("-ex" "set substitute-path /lib64/ $sysroot/lib64/")
            gdb_args+=("-ex" "set substitute-path /lib/ $sysroot/lib64/")
        else
            gdb_args+=("-ex" "set substitute-path /usr/lib/ $sysroot/usr/lib/")
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

setup_sdb

if [ -n "$OUT_DIR" ]; then
    if [ ! -d "$OUT_DIR" ]; then
        echo "ERROR: --out-dir directory does not exist: $OUT_DIR"
        exit 1
    fi
    echo "Using build output directory: $OUT_DIR"
fi

fetch_recent_crashes

if [ -n "$OUT_DIR" ]; then
    filter_crashes_for_out_dir "$OUT_DIR"
else
    filter_matter_crashes
fi

select_crash_log

crash_filename=$(basename "$SELECTED_ZIP")
app_id="${crash_filename%%_*}"
binary=$(resolve_binary_name "$app_id")

pull_and_extract_crash "$SELECTED_ZIP"

coredump=$(find_coredump "$LOCAL_TMP_DIR")
if [ -z "$coredump" ] || [ ! -f "$coredump" ]; then
    echo "ERROR: Could not find a valid core dump file inside the pulled archive."
    if [ "$CLEANUP" = true ]; then rm -rf "$LOCAL_TMP_DIR"; fi
    exit 1
fi

is_core_64=$(detect_core_is_64bit "$coredump")

if [ -n "$OUT_DIR" ]; then
    binary_path=$(resolve_binary_path "$OUT_DIR" "$binary")
    if [ -z "$binary_path" ]; then
        echo "ERROR: Binary '$binary' not found in $OUT_DIR"
        if [ "$CLEANUP" = true ]; then rm -rf "$LOCAL_TMP_DIR"; fi
        exit 1
    fi
    SELECTED_TARGET="$OUT_DIR"
else
    mapfile -t matching_targets < <(find_matching_targets "$binary" "$is_core_64")
    select_build_target "$binary" "${matching_targets[@]}"
fi

# Ensure system libraries are available for GDB analysis.
# If no sysroot is explicitly set and the build target doesn't have system_libs
# and the SDK sysroot is missing/incomplete, pull libraries from the device.
if [ -z "$SYSROOT_PATH" ] && [ ! -d "$SELECTED_TARGET/system_libs" ]; then
    local_sdk_sysroot=""
    if [[ "$SELECTED_TARGET" == *"arm64"* || "$SELECTED_TARGET" == *"aarch64"* ]]; then
        local_sdk_sysroot="$TIZEN_SDK_ROOT/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device64.core"
    else
        local_sdk_sysroot="$TIZEN_SDK_ROOT/platforms/tizen-10.0/tizen/rootstraps/tizen-10.0-device.core"
    fi
    # Check if SDK sysroot has actual library content
    if [ ! -d "$local_sdk_sysroot/usr/lib64" ] && [ ! -d "$local_sdk_sysroot/usr/lib" ] && \
       [ ! -d "$local_sdk_sysroot/lib64" ] && [ ! -d "$local_sdk_sysroot/lib" ]; then
        pull_system_libs "$SELECTED_TARGET"
    fi
fi

run_gdb_analysis "$SELECTED_TARGET" "$binary" "$coredump" "$crash_filename"

if [ "$CLEANUP" = true ]; then
    rm -rf "$LOCAL_TMP_DIR"
else
    echo "Temporary files preserved in: $LOCAL_TMP_DIR"
fi
