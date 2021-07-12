#!/bin/bash

#
# Copyright (c) 2021 Project CHIP Authors
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

CHIP_ROOT=$(readlink -f ${BASH_SOURCE%/scripts/tools/zap/run_zaptool.sh})
ZAP_ROOT="$CHIP_ROOT/third_party/zap/repo"
[[ -n "$1" ]] && ZAP_ARGS=(-i "$(readlink -f "$1")") || ZAP_ARGS=()


cd $CHIP_ROOT \
	&& git submodule update --init third_party/zap/repo \
	|| exit 1

cd $ZAP_ROOT
if ! npm list installed-check &> /dev/null 
then
	npm install installed-check \
		|| exit 1
fi

if ! ./node_modules/.bin/installed-check -c &> /dev/null 
then
	npm install \
		|| exit 1
fi


cd "$ZAP_ROOT"
node src-script/zap-start.js --logToStdout \
	--gen "$CHIP_ROOT/src/app/zap-templates/app-templates.json" \
	--zcl "$CHIP_ROOT/src/app/zap-templates/zcl/zcl.json" \
	"${ZAP_ARGS[@]}"
