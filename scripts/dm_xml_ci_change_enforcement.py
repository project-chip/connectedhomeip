#!/usr/bin/env python3

# Copyright (c) 2024 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import os
import subprocess
import sys

import click


@click.command()
@click.argument('dir', nargs=1, type=click.Path(exists=True))
def check_dm_directory(dir):
    clusters = os.path.join(dir, 'clusters')
    device_types = os.path.join(dir, 'device_types')
    if not os.path.isdir(clusters) or not os.path.isdir(device_types):
        print(f"Invalid data model directory {dir}")
        sys.exit(1)

    # We are operating in a VM, and although there is a checkout, it is working in a scratch directory
    # where the ownership is different than the runner.
    # Adding an exception for this directory so that git can function properly.
    subprocess.run("git config --global --add safe.directory '*'", shell=True)

    def check_dir(dir):
        cmd = f'git diff HEAD^..HEAD --name-only -- {dir}'
        output = subprocess.check_output(cmd, shell=True).decode()
        if output and 'spec_sha' not in output and 'scraper_version' not in output:
            print(f'Data model directory {dir} had changes to the following files without a corresponding update to the spec SHA')
            print(output)
            print("Note that the data_model directory files are automatically updated by a spec scraper and should not be manually updated.")
            return 1
        return 0

    def check_dm_scraper_script_updates(dir):
        # Run the scraper script, but don't actually scrape from the spec right now
        # because we don't have a spec checkout. This will ensure the script is
        # working, and that any script-applied updates are applied correctly.
        # TODO: If we can get this running against a checkout of the spec at the
        # stated spec revision, that would be MUCH better and we could remove
        # the check_dir check, which is not as robust.
        scripts_dir = os.path.dirname(os.path.realpath(__file__))
        generate_file = os.path.join(scripts_dir, 'spec_xml', 'generate_spec_xml.py')
        # we're not using the scraper or spec, so it doesn't matter here
        cmd_list = ['python3', generate_file, '--scraper', 'x', '--spec-root', 'x',
                    '--output-dir', dir, '--include-in-progress', 'None', '--skip-scrape']
        subprocess.run(cmd_list, check=True)
        cmd_list = ['git', 'diff', 'HEAD', '--name-only', '--', dir]
        output = subprocess.check_output(cmd_list).decode()
        if output:
            print(f"Data model directory {dir} files do not match DM generation script output")
            print(output)
            print("Note that the data_model directory files are automatically updated by a spec scraper and should not be manually updated.")
            return 1
        return 0

    ret_check_dir = check_dir(dir)
    ret_check_scraper = check_dm_scraper_script_updates(dir)
    sys.exit(ret_check_dir+ret_check_scraper)


if __name__ == '__main__':
    check_dm_directory()
