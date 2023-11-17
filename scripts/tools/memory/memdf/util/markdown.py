#
# Copyright (c) 2021 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
"""Markdown utilities."""


def read_hierified(f):
    """Read a markdown table in ‘hierified’ format."""

    line = f.readline()
    header = tuple((s.strip() for s in line.split('|')[1:-1]))

    _ = f.readline()  # The line under the header.

    rows = []
    for line in f:
        line = line.strip()
        if not line:
            break
        row = []
        columns = line.split('|')
        for i in range(0, len(header)):
            column = columns[i + 1].strip()
            if not column:
                column = rows[-1][i] if rows else '(blank)'
            row.append(column)
        rows.append(tuple(row))

    return (header, rows)
