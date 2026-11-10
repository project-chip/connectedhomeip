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
    function emit(line) { print line; fflush() }

    # Sanitizer reports run from the header line to the SUMMARY line.
    # Only the first few are printed in full.
    report_lines > 0 {
        if (reports <= 10 && report_lines <= 200) { emit($0) }
        report_lines++
        if ($0 ~ /^SUMMARY: / || report_lines > 1000) { report_lines = 0 }
        next
    }

    # GN errors run up to a line of underscores. A missing submodule
    # produces one per file, so only the first few are printed in full.
    gn_lines > 0 {
        if ($0 ~ /^(___|\+ |[a-z-]+: )/ || gn_lines > 12) {
            gn_lines = 0
        } else {
            if (gn_errors <= 3) { emit($0) }
            gn_lines++
            next
        }
    }
    /^==[0-9]+==ERROR: |^WARNING: ThreadSanitizer: / {
        reports++
        report_lines = 1
        emit(reports <= 10 ? $0 : $0 " (report omitted)")
        next
    }

    # Indented lines after a summary header (failing tests, failed build
    # commands), or source context after a compiler error.
    indented && /^\t/ { emit($0); next }
    { indented = 0 }
    context > 0 { emit($0); context--; next }

    /^(Testing failed|Failing tests|The following build commands failed):$/ {
        emit($0); indented = 1; next
    }

    # path:line: error: -[Class test] : message           (XCTest)
    # path:line:col: error: message, then source and caret (compilers)
    /^[^ \t:]+:[0-9]+: (fatal )?error: / { emit($0); next }
    /^[^ \t:]+:[0-9]+:[0-9]+: (fatal )?error: / { emit($0); context = 2; next }
    /^[^ \t:]+:[0-9]+:[0-9]+: runtime error: / { emit($0); next }
    /^ERROR at / {
        gn_errors++
        gn_lines = 1
        if (gn_errors <= 3) { emit($0) }
        next
    }

    /^(ld|clang|clang\+\+|swift-frontend|xcodebuild|ninja): (fatal )?error/ { emit($0); next }
    /^FAILED: |^ninja: build stopped/ { emit($0); next }

    /^Test Suite '.*' started at / { emit($0); next }
    /^Test Suite '.*' failed at / { emit($0); executed = 1; next }
    /^Test Suite '(All tests|Selected tests|MatterTests\.xctest)' passed at / { emit($0); executed = 1; next }
    executed && /^\t Executed / { emit($0); executed = 0; next }
    /^Test Case '.*' failed / { emit($0); next }
    /^Restarting after unexpected exit/ { emit($0); next }
    /^Process [0-9]+: [1-9][0-9]* leaks? for / { emit($0); next }

    /^\*\* .* \*\*$|^\([0-9]+ failures?\)$/ { emit($0); next }

    END {
        if (gn_errors > 3) { emit("(" gn_errors - 3 " more GN errors omitted)") }
        if (reports > 10) { emit("(" reports - 10 " sanitizer reports omitted)") }
    }
EOF
