#!/usr/bin/env python3
# Copyright (c) 2025 Project CHIP Authors
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

"""
Generate UriConfig.h from UriConfig.h.in template.
Uses configurable parameters passed from GN build system.
"""

import sys
import os


def main():
    if len(sys.argv) != 6:
        print("Usage: gen_uri_config.py <input> <output> <version> <have_wprintf> <have_reallocarray>", file=sys.stderr)
        print("  have_wprintf: 1 to enable, 0 to disable", file=sys.stderr)
        print("  have_reallocarray: 1 to enable, 0 to disable", file=sys.stderr)
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]
    version = sys.argv[3]
    have_wprintf = sys.argv[4] == "1"
    have_reallocarray = sys.argv[5] == "1"

    # Print build arguments
    print("Generating UriConfig.h with arguments:")
    print(f"  Input template: {input_file}")
    print(f"  Output file: {output_file}")
    print(f"  Version: {version}")
    print(f"  HAVE_WPRINTF: {have_wprintf}")
    print(f"  HAVE_REALLOCARRAY: {have_reallocarray}")

    # Read template
    with open(input_file, 'r') as f:
        content = f.read()

    # Replace version
    content = content.replace('@PROJECT_VERSION@', version)

    # Handle feature defines based on configuration
    if have_wprintf:
        content = content.replace('#cmakedefine HAVE_WPRINTF', '#  define HAVE_WPRINTF 1')
    else:
        content = content.replace('#cmakedefine HAVE_WPRINTF', '/* #undef HAVE_WPRINTF */')

    if have_reallocarray:
        content = content.replace('#cmakedefine HAVE_REALLOCARRAY', '#  define HAVE_REALLOCARRAY 1')
    else:
        content = content.replace('#cmakedefine HAVE_REALLOCARRAY', '/* #undef HAVE_REALLOCARRAY */')

    # Write output
    os.makedirs(os.path.dirname(output_file), exist_ok=True)
    with open(output_file, 'w') as f:
        f.write(content)

    print(f"Generated {output_file} successfully")


if __name__ == '__main__':
    main()
