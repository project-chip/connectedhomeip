#!/usr/bin/env python3

#
# Copyright (c) 2021 Project CHIP Authors
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

import argparse
import logging
import os
import tarfile

import coloredlogs
import github


class BundleBuilder:

    def __init__(self, outputName, outputPrefix, workingDirectory):
        self.outputName = outputName + '.tar.xz'
        self.outputPrefix = outputPrefix
        self.workingDirectory = workingDirectory

        logging.info('Creating bundle "%s":', self.outputName)

        self.output = tarfile.open(self.outputName, 'w:xz')

    def appendFile(self, name):
        """Appends the specified file in the working directory to the bundle."""
        logging.info('   Appending %s to the bundle', name)

        current_directory = os.path.realpath(os.curdir)
        try:
            os.chdir(self.workingDirectory)
            self.output.add(name, os.path.join(self.outputPrefix, name))
        finally:
            os.chdir(current_directory)

    def close(self):
        """Closes the bundle and returns the file name of the bundle."""
        logging.info('   Bundle creation complete.')
        self.output.close()
        return self.outputName


def main():
    """Main task if executed standalone."""
    parser = argparse.ArgumentParser(
        description='Uploads an asset bundle file to a github release .')
    parser.add_argument(
        '--github-api-token',
        type=str,
        help='Github API token to upload the report as a comment')
    parser.add_argument(
        '--github-repository', type=str, help='Repository to use for PR comments')
    parser.add_argument(
        '--release-tag', type=str, help='Release tag to upload asset to')
    parser.add_argument(
        '--bundle-files',
        type=str,
        help='A file containing what assets to include')
    parser.add_argument(
        '--working-directory',
        type=str,
        help='What directory to use as the current directory for uploading')
    parser.add_argument(
        '--bundle-name', type=str, help='Prefix to use in the archive file')
    parser.add_argument(
        '--log-level',
        default=logging.INFO,
        type=lambda x: getattr(logging, x),
        help='Configure the logging level.')
    args = parser.parse_args()

    # Ensures somewhat pretty logging of what is going on
    logging.basicConfig(
        level=args.log_level,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    coloredlogs.install()

    if not args.github_api_token:
        logging.error(
            'Required arguments missing: github api token is required')
        return

    bundle = BundleBuilder(args.bundle_name, args.bundle_name,
                           args.working_directory)

    with open(args.bundle_files, 'rt') as bundleInputs:
        for fileName in bundleInputs.readlines():
            bundle.appendFile(fileName.strip())

    assetPath = bundle.close()

    api = github.Github(args.github_api_token)
    repo = api.get_repo(args.github_repository)

    logging.info('Connected to github repository')

    release = repo.get_release(args.release_tag)
    logging.info('Release "%s" found.' % args.release_tag)

    logging.info('Uploading %s', assetPath)
    release.upload_asset(assetPath)
    logging.info('Asset upload complete')


if __name__ == '__main__':
    # execute only if run as a script
    main()
