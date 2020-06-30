#!/usr/bin/env python3

#
# Copyright (c) 2020 Project CHIP Authors
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
import attr
import coloredlogs
import logging

from circleci.api import Api


@attr.s
class CompletedBuild:
  """Represents basic build information"""
  job_name = attr.ib(kw_only=True)
  build_num = attr.ib(kw_only=True)


@attr.s
class ArtifactInfo:
  """Represents basic artifact information"""
  path = attr.ib(kw_only=True)  # pretty path
  url = attr.ib(kw_only=True)  # download URL


class ApiHandler:
  """Wrapper around circleci API calls, specific for artifact access."""

  def __init__(self, token, user='project-chip', project='connectedhomeip'):
    self.api = Api(token)
    self.user = user
    self.project = project
    self.vcs_type = 'github'

  def fetchCompletedBuilds(self, branch='master'):
    """Gets a snapshot of the latest successful builds in the specified branch."""
    for build in self.api.get_project_build_summary(
        self.user, self.project, branch=branch, status_filter='successful'):
      logging.debug('Fetched summary: %r', build)
      yield CompletedBuild(
          job_name=build['workflows']['job_name'], build_num=build['build_num'])

  def fetchLatestBuildNumber(self, job_name, branch='master'):
    """Fetch the latest build number for the specified job name."""
    logging.info('Searching for the latest job "%s"', job_name)

    for build in self.fetchCompletedBuilds(branch):
      logging.info('Found a completed build %r', build)

      if build.job_name == job_name:
        logging.info('Found the latest job with id %d', build.build_num)
        return build.build_num

    logging.error('No usable latest build was found.')
    raise Exception('No build found for job "%s"' % job_name)

  def getArtifacts(self, build_num):
    """Fetch artifacts for the specified build number."""
    for artifact in self.api.get_artifacts(self.user, self.project, build_num,
                                           self.vcs_type):
      logging.debug('Fetched artifact info: %r', artifact)
      yield ArtifactInfo(path=artifact['path'], url=artifact['url'])


def fetchArtifactsForJob(token, job_name, download_dir):
  """Download all job artifacts in to the specified directory."""
  # actual processing: downloads all the given artifacts
  handler = ApiHandler(token)
  for artifact in handler.getArtifacts(
      handler.fetchLatestBuildNumber(job_name)):
    logging.info('Downloading artifact %r...', artifact)
    handler.api.download_artifact(artifact.url, download_dir)
    logging.info('Download complete')


def main():
  """Main task if executed standalone."""
  parser = argparse.ArgumentParser(description='Fetch master build artifacts.')
  parser.add_argument('--token', type=str, help='API token to use')
  parser.add_argument('--job', type=str, help='What job to search for')
  parser.add_argument(
      '--download-dir',
      type=str,
      default='.',
      help='Where to download the artifacts')
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

  if not args.token or not args.job:
    logging.error(
        'Required arguments missing. Please specify at least job and token.')
    return

  fetchArtifactsForJob(args.token, args.job, args.download_dir)


if __name__ == '__main__':
  # execute only if run as a script
  main()
