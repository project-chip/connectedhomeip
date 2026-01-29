#!/bin/bash
#
# Copyright (c) 2025 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -euo pipefail

readonly RED='\033[0;31m'
readonly NC='\033[0m' # No Color

main() {
    local choice

    echo 'This will remove the following temporary files used by Matter and Thread:'
    echo '  `tmp`'
    echo '  `/tmp/chip_*`'
    echo '  `/var/lib/thread/*`'
    printf "${RED}Are you sure you want to continue? (y/N): ${NC}"

    read -r -n 1 choice

    echo ""

    if [[ "$choice" =~ ^[Yy]$ ]]; then
        echo "Removing temporary files.."
    else
        exit 1
    fi

    sudo rm -rvf tmp
    sudo rm -rvf /tmp/chip_*
    sudo rm -rvf /var/lib/thread/*

    if ip addr show dev lo | grep -q 'fe80::'; then
        echo 'loopback interface already has link-local address'
    else
        sudo ip addr add fe80::47/64 dev lo
        echo 'add link-local address to loopback interface for it to act as infra link'
    fi

    pkill -USR1 -f ot-rcp || true
    pkill -f chip-lighting-app || true

    local path=$PATH
    path=third_party/openthread/repo/build/simulation/examples/apps/cli:$path
    path=third_party/openthread/repo/build/simulation/examples/apps/ncp:$path
    path=third_party/ot-br-posix/repo/build/otbr/src/agent:$path
    path=third_party/ot-br-posix/repo/build/otbr/third_party/openthread/repo/src/posix:$path

    sudo sed -i 's/disable-user-service-publishing=yes/disable-user-service-publishing=no/g' /etc/avahi/avahi-daemon.conf
    sudo sed -i 's/publish-aaaa-on-ipv4=no/publish-aaaa-on-ipv4=yes/g' /etc/avahi/avahi-daemon.conf
    sudo systemctl restart avahi-daemon

    echo 'Setting sticky bit to run otbr-agent with root privileges..'
    sudo chown root third_party/ot-br-posix/repo/build/otbr/src/agent/otbr-agent
    sudo chmod +s third_party/ot-br-posix/repo/build/otbr/src/agent/otbr-agent
    sudo chown root third_party/ot-br-posix/repo/build/otbr/third_party/openthread/repo/src/posix/ot-ctl
    sudo chmod +s third_party/ot-br-posix/repo/build/otbr/third_party/openthread/repo/src/posix/ot-ctl
    echo 'Done'
    PATH=$path OT_RCP=$(command -v ot-rcp) "$@"
}

main "$@"
