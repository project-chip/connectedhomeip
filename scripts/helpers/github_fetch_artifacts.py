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

import github
import logging
import subprocess


def fetchMasterMergeCommitSHA():
  return subprocess.run(
      'git merge-base --fork-point master'.split(),
      stdout=subprocess.PIPE,
      stderr=subprocess.STDOUT,
  ).stdout.decode('utf8').split()[0]


def fetchArtifactsForJob(jobName, githubToken, githubRepo, downloadDir):
    masterCommitSHA = fetchMasterMergeCommitSHA()

    logging.info('Master merge commit: "%s"' % masterCommitSHA)

    api = github.Github(githubToken)
    repo = api.get_repo(githubRepo)

    commit = repo.get_commit(masterCommitSHA)

    logging.info('COMMIT: %r' % commit)

    logging.error('NOT YET IMPLEMENTED')
    pass