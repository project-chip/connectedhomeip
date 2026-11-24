#!/usr/bin/env bash
#
# Copyright (c) 2026 Project CHIP Authors
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

# Prints test totals from an xcodebuild result bundle, in the style of the
# XCTest "Executed N tests" line. The totals XCTest itself prints at the end of
# a run only cover the last test process, which is wrong when a test case
# relaunches the process (shouldRelaunchBeforeRunningTest) or after a crash.
#
# Counts are of tests, not of failure records as in the XCTest line.
#
# Usage: darwin_test_summary.sh TestResults.xcresult

set -eo pipefail

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 TestResults.xcresult" >&2
    exit 2
fi

test_seconds=$(xcrun xcresulttool get test-results tests --path "$1" |
    jq '[.. | objects | select(.nodeType == "Test Case") | .durationInSeconds // 0] | add // 0')

xcrun xcresulttool get test-results summary --path "$1" | jq -r --argjson test_seconds "$test_seconds" '
    def n($count; $noun): "\($count) \($noun)\(if $count == 1 then "" else "s" end)";
    "Summary: Executed \(n(.totalTestCount; "test")), with \(n(.skippedTests; "test")) skipped"
    + " and \(n(.failedTests; "test")) failed"
    + (if .expectedFailures > 0 then " (\(n(.expectedFailures; "expected failure")))" else "" end)
    + " in \(n($test_seconds | round; "second"))"
'
