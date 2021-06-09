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
import github
import logging
import traceback
import sys

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
      '--pr',
      type=int,
      help='The PR number of the pull request')
  parser.add_argument(
      '--title',
      type=str,
      help='Title of the comment')
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
    logging.error('Required arguments missing: github api token is required.')
    return

  try:
    logging.info('Uploading report to "%s", PR %d', args.github_repository, args.pr)
    logging.info('Title "%s"', args.title)

    rawText = sys.stdin.read()
    logging.info('Content "%s"', rawText)

    api = github.Github(args.github_api_token)
    repo = api.get_repo(args.github_repository)
    pull = repo.get_pull(args.pr)

    # Remove old comment
    for comment in pull.get_issue_comments():
      if not comment.user.login == 'github-actions[bot]':
        continue
      if not comment.body.startswith(args.title):
        continue
      logging.info('Removing obsolete comment with heading "%s"', (args.title))
      comment.delete()

    # Post new comment
    pull.create_issue_comment("""{title} from {baseSha}

  <details>
    <summary>Full output</summary>

  ```
  {content}
  ```

  </details>
  """.format(title=args.title, content=rawText, baseSha=pull.base.sha))

  except Exception as e:
    tb = traceback.format_exc()
    logging.warning('Failed to post comment: %s', tb)



if __name__ == '__main__':
  # execute only if run as a script
  main()
