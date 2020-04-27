#!/bin/sh

#
#    Copyright (c) 2020 Project CHIP Authors. All Rights Reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

#
#    Description:
#      This file is the script for Travis CI hosted, distributed continuous
#      integration 'before_install' trigger of the 'install' step.
#

die() {
  echo " *** ERROR: " "${*}"
  exit 1
}

# Package build machine OS-specific configuration and setup

case "$TRAVIS_OS_NAME" in

  linux)
    # By default, Travis CI does not have IPv6 enabled on
    # Linux. Ensure that IPv6 is enabled since CHIP, and its unit
    # and functional tests, depend on working IPv6 support.

    sudo sysctl -w net.ipv6.conf.all.disable_ipv6=0

    # If Docker is present, then enable IPv6 configuration.

    if command -v docker; then
      sudo service docker stop

      sudo touch /etc/docker/daemon.json

      sudo echo '{ "ipv6" : true }' >/etc/docker/daemon.json

      sudo service docker start
    fi

    ;;

esac
