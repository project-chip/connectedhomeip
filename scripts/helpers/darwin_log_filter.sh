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

# Prints the parts of `xcodebuild test` output that are needed to see why a
# build or test run failed: test suite progress, test failures, build errors,
# sanitizer reports and the final xcodebuild summary. The full output of a
# Darwin framework test run is millions of lines, which is too much for the
# GitHub Actions log viewer.
#
# Usage:
#   xcodebuild test ... 2>&1 | tee darwin-tests.log | darwin_log_filter.sh
#   darwin_log_filter.sh < darwin-tests.log

if [[ $# -ne 0 ]]; then
    echo "Usage: $0 < xcodebuild-output" >&2
    exit 2
fi

# Test output is not always valid UTF-8.
export LC_ALL=C
exec awk -f /dev/fd/3 3<<'EOF'
    # Each rule below matches a header line, prints it and starts a block
    # of up to N following lines. A block ends at the next header, at its
    # end pattern (exclusive) or after N lines, so it never hides a header.
    function emit(line) { print line; fflush() }
    function block_start(lines, end, show) { block_lines = lines; block_end = end; block_show = show }

    # Only the first few sanitizer reports are printed in full.
    /^==[0-9]+==ERROR: |^WARNING: ThreadSanitizer: / {
        reports++
        emit(reports <= 10 ? $0 : $0 " (report omitted)")
        block_start(200, "", reports <= 10)
        next
    }

    # A missing submodule produces one GN error per file, so only the first
    # few are printed in full.
    /^ERROR at / {
        gn_errors++
        if (gn_errors <= 3) { emit($0) }
        block_start(12, "^(___|\\+ )", gn_errors <= 3)
        next
    }

    # path:line:col: error: message, followed by source line and caret
    /^[^ \t:]+:[0-9]+:[0-9]+: (fatal )?error: / { emit($0); block_start(2, "", 1); next }

    # Followed by indented lines (failing tests, failed build commands, or
    # the "Executed N tests" line of a test suite).
    /^(Testing failed|Failing tests|The following build commands failed):$/ ||
    /^Test Suite '.*' failed at / ||
    /^Test Suite '(All tests|Selected tests|MatterTests\.xctest)' passed at / {
        emit($0); block_start(100, "^([^\t]|$)", 1); next
    }

    /^[^ \t:]+:[0-9]+: (fatal )?error: / ||          # XCTest: path:line: error: -[Class test] : message
    /^[^ \t:]+:[0-9]+:[0-9]+: runtime error: / ||    # UBSan
    /^SUMMARY: / ||
    /^(ld|clang|clang\+\+|swift-frontend|xcodebuild|ninja): (fatal )?error/ ||
    /^FAILED: |^ninja: build stopped/ ||
    /^Test Suite '.*' started at / ||
    /^Test Case '.*' failed / ||
    /^Restarting after unexpected exit/ ||
    /^Process [0-9]+: [1-9][0-9]* leaks? for / ||
    /^\*\* .* \*\*$|^\([0-9]+ failures?\)$/ {
        emit($0); block_start(0); next
    }

    block_lines > 0 {
        if (block_end != "" && $0 ~ block_end) {
            block_lines = 0
        } else {
            if (block_show) { emit($0) }
            block_lines--
        }
    }

    END {
        if (gn_errors > 3) { emit("(" gn_errors - 3 " more GN errors omitted)") }
        if (reports > 10) { emit("(" reports - 10 " sanitizer reports omitted)") }
    }
EOF
