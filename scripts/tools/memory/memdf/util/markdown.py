#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
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
