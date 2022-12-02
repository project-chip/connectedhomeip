#
#   Copyright (c) 2022 Project CHIP Authors
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
import sys
import subprocess

"""
 Simple script to fetch some information on the current git repo.
 This script is used by gn in a exec_script to format a sotfware version
 string based on the branch, commit SHA and state.
 The gn exec_script retrieves the output from the stdout.
"""


def getGitCommitSHA() -> str:
    return subprocess.check_output(['git', 'rev-parse', 'HEAD']).decode('ascii').strip()


def getGitShortCommitSHA() -> str:
    return subprocess.check_output(['git', 'describe', '--always', '--dirty']).decode('ascii').strip()


def getGitCurrentBranchName() -> str:
    return subprocess.check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD']).decode('ascii').strip()


def main():
    # <Matter Version>-<branch_name>-<Short_SHA>[-dirty]
    print("v1.0-" + getGitCurrentBranchName() + "-" + getGitShortCommitSHA())
    sys.exit(0)


if __name__ == "__main__":
    main()
