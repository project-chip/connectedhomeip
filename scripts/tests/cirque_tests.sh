#!/bin/bash

#
# Copyright (c) 2020 Project CHIP Authors
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

set -x -e

SOURCE="${BASH_SOURCE[0]}"
SOURCE_DIR="$(cd -P "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)"
REPO_DIR="$SOURCE_DIR/../../"

function flask_clean() {
  flask_pid=`ps aux | grep "[f]lask" | awk '{print $2}' | sort -k2 -rn`
  if [ ! -z "${flask_pid}" ]; then
    for pid in "${flask_pid}"; do
      kill -9 $pid >/dev/null 2>&1
    done
  fi
}

function socat_clean() {
  socat_pid=`ps aux | grep "[s]ocat" | awk '{print $2}'`
  if [ ! -z "${socat_pid}" ]; then
    for pid in "${socat_pid}"; do
      kill -9 $pid >/dev/null 2>&1
    done
  fi
}

function virtual_thread_clean() {
  vthread_pid=`ps aux | grep "[o]t-ncp-ftd" | awk '{print $2}'`
  if [ ! -z "${vthread_pid}" ]; then
    for pid in "${vthread_pid}"; do
      kill -9 $pid >/dev/null 2>&1
    done
  fi
}

__cirquetest_start_flask() {
    cd $REPO_DIR/third_party/cirque
    FLASK_APP='cirque/restservice/service.py' \
    PATH="${PATH}":"${OPENTHREAD_DIR}"/output/x86_64-unknown-linux-gnu/bin/ \
    python3 -m flask run
}

__cirquetest_clean_flask() {
    flask_clean
    socat_clean
    virtual_thread_clean
}

cirquetest_bootstrap() {
    cd $REPO_DIR/third_party/cirque
    sudo apt-get install -y bazel socat psmisc tigervnc-standalone-server tigervnc-viewer python3-pip python3-venv python3-setuptools
    pip3 install pycodestyle==2.5.0 wheel
    git fetch origin 2dbd3fe2215faaec0ba3dd33b155ff1e793abbab && git checkout FETCH_HEAD
    make NO_GRPC=1 install -j
    ./dependency_modules.sh
    python3 -m venv venv
    source venv/bin/activate
    pip3 install wheel
    pip3 install -r requirements.txt
    deactivate

    echo "This is CirqueTest::Bootstrap"
    echo "Source    $SOURCE"
    echo "SourceDir $SOURCE_DIR"
    echo "RepoDir   $REPO_DIR"
}

cirquetest_test() {
    # Start Cirque flash server
    __cirquetest_start_flask &
    cd $REPO_DIR/src/test_driver
    ./do_on-off-cluster.py
}

cirquetest_help() {
    echo "This is CirqueTest::Help"
}

subcommand="$1"
shift

case $subcommand in
    "" | "help")
        cirquetest_help $@
        ;;
    *)
        cirquetest_${subcommand} $@
        if [ $? = 127 ]; then
            echo "Unknown command: $?" >&2
            exit $?
        fi
        ;;
esac
