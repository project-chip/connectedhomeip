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

# Publish TC-CNET-4.11 results from the Cirque WiFiSwitchNetworkTest log to the
# GitHub Actions job summary and stdout (same intent as tests.yaml "Print test run
# summaries" for dedicated CNET steps).

set -euo pipefail

LOG_DIR="${1:-${LOG_DIR:-/tmp/cirque_test_output}}"
LOG_FILE="${LOG_DIR}/WiFiSwitchNetworkTest.log"

emit() {
    printf '%s\n' "$*"
    if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
        printf '%s\n' "$*" >>"$GITHUB_STEP_SUMMARY"
    fi
}

emit_block() {
    if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
        printf '%s' "$1" >>"$GITHUB_STEP_SUMMARY"
    fi
    printf '%s' "$1"
}

emit "## TC-CNET-4.11 (Cirque / WiFiSwitchNetworkTest)"
emit ""
emit "Dual simulated access points (\`WIFI_1ST_AP\` / \`WIFI_2ND_AP\`) via \`mac80211_hwsim\` and Cirque \`mac80211_ap_image\` — not the BLE-WiFi mock runner."
emit ""

if [[ ! -f "$LOG_FILE" ]]; then
    emit "**Log not found:** \`${LOG_FILE}\`"
    emit ""
    emit "The Cirque job may have failed before \`WiFiSwitchNetworkTest\` ran. See the full artifact under \`cirque_log-*\`."
    exit 0
fi

emit "**Log:** \`${LOG_FILE}\`"
emit ""

class_summary="$(grep -F 'Summary for test class TC_CNET_4_11:' "$LOG_FILE" | tail -1 || true)"
run_summary="$(grep -F 'Test results:' "$LOG_FILE" | tail -1 || true)"
commission_summary="$(grep -F 'Summary for test class CommissionDeviceTest:' "$LOG_FILE" | tail -1 || true)"
cnet_result="$(grep -E '\[Test\] test_TC_CNET_4_11 (PASS|FAIL)' "$LOG_FILE" | tail -1 || true)"

if [[ -n "$commission_summary" ]]; then
    emit "- \`${commission_summary#*"[MatterTest] "}\`"
fi
if [[ -n "$cnet_result" ]]; then
    if grep -q ' FAIL$' <<<"$cnet_result"; then
        emit "- **\`${cnet_result#*"[MatterTest] "}\`**"
    else
        emit "- \`${cnet_result#*"[MatterTest] "}\`"
    fi
fi
if [[ -n "$class_summary" ]]; then
    emit "- \`${class_summary#*"[MatterTest] "}\`"
fi
if [[ -n "$run_summary" ]]; then
    emit "- \`${run_summary}\`"
fi

emit ""
emit "### Certification steps (TC-CNET-4.11)"
emit ""
emit '| Step | Description |'
emit '| ---: | --- |'

while IFS= read -r line; do
    step_num="$(sed -n 's/.*Test Step \([0-9][0-9]*\) :.*/\1/p' <<<"$line")"
    [[ -z "$step_num" ]] && continue
    desc="$(sed -n 's/.*Test Step [0-9][0-9]* : //p' <<<"$line" | head -c 120)"
    emit "| ${step_num} | ${desc} |"
done < <(grep -E 'INFO \*\*\*\*\* Test Step [0-9]+ :' "$LOG_FILE" | grep -v 'INFO 		' | sort -t' ' -k8 -n | uniq)

emit ""

if grep -qE '(\[Test\] test_TC_CNET_4_11 FAIL|Failed [1-9])' "$LOG_FILE"; then
    emit "### Failure excerpts"
    emit ""
    emit '```text'
    emit_block "$(grep -E 'FAIL|AssertionError|asserts\.|change_networks:.*failed|TC_CNET_4_11 failed' "$LOG_FILE" | tail -20 || true)"
    emit '```'
    emit ""
fi

emit "### Full log"
emit ""
emit "Download the \`cirque_log-*\` artifact → \`WiFiSwitchNetworkTest.log\` and \`WiFiSwitchNetworkTest/device_logs/\`."
emit ""

echo ""
echo "========================================================================"
echo "  CIRQUE TC-CNET-4.11 SUMMARY"
echo "========================================================================"
[[ -n "$class_summary" ]] && echo "  ${class_summary#*"[MatterTest] "}"
[[ -n "$run_summary" ]] && echo "  ${run_summary}"
[[ -n "$cnet_result" ]] && echo "  ${cnet_result#*"[MatterTest] "}"
echo "========================================================================"
