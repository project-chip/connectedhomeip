#!/bin/bash

CHIP_ROOT=$(readlink -f "${BASH_SOURCE%/scripts/tools/zap/run_zaptool.sh}")
ZAP_ROOT="$CHIP_ROOT/third_party/zap/repo"

(
	cd "$CHIP_ROOT"
	git submodule update --init third_party/zap/repo

	cd "$ZAP_ROOT"
	if ! npm list installed-check > /dev/null
	then
		npm install installed-check
	fi

	if ! ./node_modules/.bin/installed-check -c 2> /dev/null
       	then
		npm install
	fi
)

[[ -n "$1" ]] && ZAP_ARGS=(-i "$(readlink -f "$1")") || ZAP_ARGS=()

cd "$ZAP_ROOT"
node src-script/zap-start.js --logToStdout \
    --gen "$CHIP_ROOT/src/app/zap-templates/app-templates.json" \
    --zcl "$CHIP_ROOT/src/app/zap-templates/zcl/zcl.json" \
  "${ZAP_ARGS[@]}"
