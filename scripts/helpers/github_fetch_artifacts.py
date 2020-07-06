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
import io
import logging
import requests
import subprocess
import zipfile

# Artifacts can be fetched using:
#   curl -H "Accept: application/vnd.github.v3+json" https://api.github.com/repos/project-chip/connectedhomeip/actions/artifacts


class ArtifactInfo(github.GithubObject.NonCompletableGithubObject):

  def _initAttributes(self):
    pass

  def _useAttributes(self, attr):
    if 'id' in attr:
      self.id = self._makeIntAttribute(attr['id']).value
    if 'node_id' in attr:
      self.node_id = self._makeStringAttribute(attr['node_id']).value
    if 'name' in attr:
      self.name = self._makeStringAttribute(attr['name']).value
    if 'size_in_bytes' in attr:
      self.size_in_bytes = self._makeIntAttribute(attr['size_in_bytes']).value
    if 'url' in attr:
      self.url = self._makeStringAttribute(attr['url']).value
    if 'archive_download_url' in attr:
      self.archive_download_url = self._makeStringAttribute(attr['archive_download_url']).value
    if 'expired' in attr:
      self.expired = self._makeBoolAttribute(attr['expired']).value
    if 'created_at' in attr:
      self.created_at = self._makeDatetimeAttribute(attr['created_at']).value
    if 'updated_at' in attr:
      self.expires_at = self._makeDatetimeAttribute(attr['updated_at']).value

  def downloadBlob(self):
    url = self.archive_download_url
    logging.info('Fetching: %r' % url)

    headers, _ = self._requester.requestBlobAndCheck('GET', url)

    if headers['status'] != '302 Found':
        raise Exception('Expected a redirect during blob download.')

    response = requests.get(headers['location'])
    response.raise_for_status()

    return response.content

class ArtifactFetcher(github.GithubObject.NonCompletableGithubObject):

  def __init__(self, repo):
    self.url = repo.url + '/actions/artifacts'
    self._requester = repo._requester

  def get_artifacts(self):
    return github.PaginatedList.PaginatedList(
        ArtifactInfo,
        self._requester,
        self.url,
        None,
        headers={'Accept': 'application/vnd.github.v3+json'},
        list_item='artifacts',
    )


def fetchMasterMergeCommitSHA(forkPointRef, revStr):
  if revStr:
    result = subprocess.run(
        ['git', 'rev-parse', revStr],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    ).stdout.decode('utf8')

    logging.info('Parsed revision %r base result: %r' % (revStr, result))
  else:
    logging.info('Finding merge point from %s' % forkPointRef)
    result = subprocess.run(
        ('git merge-base --fork-point %s' % forkPointRef).split(),
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    ).stdout.decode('utf8')

    logging.info('Merge base result: %r' % result)

  return result.split()[0]


def fetchArtifactsForJob(jobName, githubToken, githubRepo, downloadDir, forkPointRef, compareRev):
  masterMergeSha = fetchMasterMergeCommitSHA(forkPointRef, compareRev)

  logging.info('Master merge commit: "%s"', masterMergeSha)

  api = github.Github(githubToken)
  repo = api.get_repo(githubRepo)

  masterArtifactName = '%s-%s' % (jobName, masterMergeSha)
  logging.info('Searching for artifact: %s' % masterArtifactName)

  artifact = None
  fetcher = ArtifactFetcher(repo)
  for idx, a in enumerate(fetcher.get_artifacts()):
    logging.debug('%d: Found artifact: %s from %r', idx, a.name, a.created_at)

    if a.name == masterArtifactName:
        artifact = a
        break

  if not artifact:
    logging.error('Artifact not found')
    return

  zipFile = zipfile.ZipFile(io.BytesIO(artifact.downloadBlob()), 'r')

  logging.info('Extracting zip file to %r' % downloadDir)
  zipFile.extractall(downloadDir)
