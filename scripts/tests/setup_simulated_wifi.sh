#!/usr/bin/env bash
#
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
#

# Load mac80211_hwsim on the host kernel before Cirque starts.
#
# Cirque only modprobes the module and then moves phy devices into containers.
# GitHub-hosted runners use an Azure kernel that ships cfg80211 and mac80211 in
# linux-modules-extra and does not package mac80211_hwsim. The out-of-tree
# module has to be built from the Ubuntu source package that matches `uname -r`,
# because that tree carries the ABI patch the packaged mac80211 was built with.
# A vanilla kernel.org copy of the same version does not, and fails to load.

set -euo pipefail

RADIOS="${1:-16}"
export DEBIAN_FRONTEND=noninteractive

if [[ ! "${RADIOS}" =~ ^[1-9][0-9]*$ ]]; then
    echo "ERROR: radio count must be a positive integer, got: ${RADIOS}" >&2
    exit 1
fi

as_root() {
    if [[ "$(id -u)" -eq 0 ]]; then
        "$@"
    elif command -v sudo >/dev/null 2>&1; then
        sudo "$@"
    else
        echo "ERROR: root or passwordless sudo is required" >&2
        exit 1
    fi
}

apt_install() {
    as_root env DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends "$@"
}

require_wlan() {
    local -a wlan_devs
    mapfile -t wlan_devs < <(find /sys/class/net -maxdepth 1 -name 'wlan*' -printf '%f\n' | sort)
    echo "wlan devices (${#wlan_devs[@]}): ${wlan_devs[*]:-none}"
    if ((${#wlan_devs[@]} < RADIOS)); then
        echo "ERROR: mac80211_hwsim produced ${#wlan_devs[@]} wlan device(s); need ${RADIOS}." >&2
        exit 1
    fi
}

load_wireless_deps() {
    if modinfo libarc4 >/dev/null 2>&1; then
        as_root modprobe libarc4
    fi
    as_root modprobe cfg80211
    as_root modprobe mac80211
}

load_packaged_hwsim() {
    if ! lsmod | grep -q '^mac80211_hwsim '; then
        load_wireless_deps
        as_root modprobe mac80211_hwsim "radios=${RADIOS}"
    fi
    sleep 2
    require_wlan
}

enable_ubuntu_deb_src() {
    local candidate file line src
    local -a files=() to_add=() existing_lines=()
    shopt -s nullglob
    if [[ -f /etc/apt/sources.list ]]; then
        files+=(/etc/apt/sources.list)
    fi
    for candidate in /etc/apt/sources.list.d/*.sources /etc/apt/sources.list.d/*.list; do
        if grep -qE 'archive\.ubuntu\.com|security\.ubuntu\.com' "${candidate}"; then
            files+=("${candidate}")
        fi
    done
    shopt -u nullglob
    if ((${#files[@]} == 0)); then
        echo "ERROR: no Ubuntu apt source list found; cannot download the kernel source." >&2
        exit 1
    fi

    for file in "${files[@]}"; do
        case "${file}" in
            *.sources)
                if grep -q '^Types: deb[[:space:]]*$' "${file}"; then
                    as_root sed -i 's/^Types: deb[[:space:]]*$/Types: deb deb-src/' "${file}"
                fi
                ;;
            *)
                to_add=()
                mapfile -t existing_lines < "${file}"
                for line in "${existing_lines[@]}"; do
                    [[ "${line}" =~ ^deb[[:space:]] ]] || continue
                    src="deb-src${line#deb}"
                    if ! printf '%s\n' "${existing_lines[@]}" | grep -qxF "${src}"; then
                        to_add+=("${src}")
                    fi
                done
                if ((${#to_add[@]} > 0)); then
                    printf '%s\n' "${to_add[@]}" | as_root tee -a "${file}" >/dev/null
                fi
                ;;
        esac
    done
    echo "Enabled deb-src for: ${files[*]}"
}

install_kernel_compiler() {
    local kbuild cc_text cc_pkg
    kbuild="/lib/modules/$(uname -r)/build"
    if [[ ! -d "${kbuild}" ]]; then
        echo "ERROR: kernel headers are not installed at ${kbuild}" >&2
        exit 1
    fi
    cc_pkg="gcc"
    if [[ -f "${kbuild}/.config" ]]; then
        cc_text="$(sed -n 's/^CONFIG_CC_VERSION_TEXT="//p' "${kbuild}/.config" | head -1)"
        cc_text="${cc_text%%\"*}"
        cc_pkg="$(printf '%s\n' "${cc_text}" | grep -o 'gcc-[0-9][0-9]*' | head -1 || true)"
        cc_pkg="${cc_pkg:-gcc}"
    fi
    if ! command -v "${cc_pkg}" >/dev/null 2>&1; then
        echo "Installing kernel compiler ${cc_pkg}"
        apt_install "${cc_pkg}"
    fi
    echo "Kernel compiler: ${cc_pkg}"
}

ubuntu_source_identity() {
    local show source_field
    show="$(apt-cache show "linux-headers-$(uname -r)" | awk 'BEGIN { RS = ""; } { print; exit }')"
    if [[ -z "${show}" ]]; then
        echo "ERROR: apt has no linux-headers-$(uname -r) package" >&2
        exit 1
    fi
    source_field="$(printf '%s\n' "${show}" | sed -n 's/^Source: //p' | head -1)"
    SOURCE_PKG="${source_field%% *}"
    SOURCE_VER="$(printf '%s\n' "${source_field}" | sed -n 's/.*(\([^)]*\)).*/\1/p')"
    if [[ -z "${SOURCE_VER}" ]]; then
        SOURCE_VER="$(printf '%s\n' "${show}" | sed -n 's/^Version: //p' | head -1)"
    fi
    if [[ -z "${SOURCE_PKG}" || -z "${SOURCE_VER}" ]]; then
        echo "ERROR: could not resolve the Ubuntu source package for linux-headers-$(uname -r)" >&2
        printf '%s\n' "${show}" >&2
        exit 1
    fi
}

build_hwsim_from_ubuntu_source() {
    local root src_dir mod_dir header ko
    local -a hwsim_sources
    enable_ubuntu_deb_src
    as_root env DEBIAN_FRONTEND=noninteractive apt-get update
    ubuntu_source_identity
    echo "Building mac80211_hwsim from ${SOURCE_PKG}=${SOURCE_VER} for $(uname -r)"
    if ! apt-cache showsrc "${SOURCE_PKG}" | grep -Fxq "Version: ${SOURCE_VER}"; then
        echo "ERROR: ${SOURCE_PKG} ${SOURCE_VER} is not in the Ubuntu source index." >&2
        apt-cache showsrc "${SOURCE_PKG}" | sed -n 's/^Version: //p' | head -20 >&2 || true
        exit 1
    fi

    apt_install build-essential bc dpkg-dev ca-certificates xz-utils "linux-headers-$(uname -r)"
    install_kernel_compiler

    root="$(mktemp -d)"
    # shellcheck disable=SC2064
    trap "rm -rf ${root@Q}" EXIT
    src_dir="${root}/src"
    mod_dir="${root}/mod"
    mkdir -p "${src_dir}" "${mod_dir}"

    (
        cd "${src_dir}"
        apt-get source --only-source "${SOURCE_PKG}=${SOURCE_VER}"
    )

    mapfile -d '' hwsim_sources < <(find "${src_dir}" -path '*/drivers/net/wireless/virtual/mac80211_hwsim.c' -print0)
    if ((${#hwsim_sources[@]} != 1)); then
        echo "ERROR: expected one mac80211_hwsim.c in ${SOURCE_PKG} ${SOURCE_VER}, found ${#hwsim_sources[@]}" >&2
        exit 1
    fi
    cp "${hwsim_sources[0]}" "${mod_dir}/mac80211_hwsim.c"
    header="${hwsim_sources[0]%.c}.h"
    if [[ -f "${header}" ]]; then
        cp "${header}" "${mod_dir}/"
    fi
    cat > "${mod_dir}/Makefile" << 'EOF'
obj-m := mac80211_hwsim.o
EOF

    make -C "/lib/modules/$(uname -r)/build" "M=${mod_dir}" modules
    ko="${mod_dir}/mac80211_hwsim.ko"
    if [[ ! -f "${ko}" ]]; then
        echo "ERROR: build did not produce ${ko}" >&2
        exit 1
    fi
    modinfo "${ko}"
    load_wireless_deps
    as_root insmod "${ko}" "radios=${RADIOS}"
    sleep 2
    require_wlan
}

echo "=== Setting up mac80211_hwsim (radios=${RADIOS}, kernel=$(uname -r)) ==="

if modinfo mac80211_hwsim >/dev/null 2>&1; then
    echo "mac80211_hwsim is already packaged for this kernel."
    load_packaged_hwsim
    exit 0
fi

echo "mac80211_hwsim is not packaged. Installing linux-modules-extra-$(uname -r)..."
as_root env DEBIAN_FRONTEND=noninteractive apt-get update
apt_install "linux-modules-extra-$(uname -r)"

if modinfo mac80211_hwsim >/dev/null 2>&1; then
    echo "mac80211_hwsim came from linux-modules-extra."
    load_packaged_hwsim
    exit 0
fi

echo "linux-modules-extra does not ship mac80211_hwsim. Building it from the matching Ubuntu source."
build_hwsim_from_ubuntu_source
echo "mac80211_hwsim is loaded."
