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
import coloredlogs
import logging
import github

def main():
  """Main task if executed standalone."""
  parser = argparse.ArgumentParser(description='Fetch master build artifacts.')
  parser.add_argument(
      '--github-api-token',
      type=str,
      help='Github API token to upload the report as a comment')
  parser.add_argument(
      '--github-repository', type=str, help='Repository to use for PR comments')
  parser.add_argument(
      '--release-tag', type=str, help='Release tag to upload asset to')
  parser.add_argument(
      '--asset-path', type=str, help='What asset to upload')
  parser.add_argument(
      '--asset-name', type=str, help='How to name the asset when uploading')
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
    logging.error('Required arguments missing: github api token is required')
    return

  api = github.Github(args.github_api_token)
  repo = api.get_repo(args.github_repository)

  logging.info('Connected to github repository')

  release = repo.get_release(args.release_tag)
  logging.info('Release "%s" found. Uploading asset' % release.title)

  logging.info('Uploading %s as %s' % (args.asset_path, args.asset_name))
  release.upload_asset(args.asset_path, name = args.asset_name)
  logging.info('Asset upload complete')
  

if __name__ == '__main__':
  # execute only if run as a script
  main()