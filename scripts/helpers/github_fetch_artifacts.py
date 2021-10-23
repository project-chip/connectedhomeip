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
import requests

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
            self.size_in_bytes = self._makeIntAttribute(
                attr['size_in_bytes']).value
        if 'url' in attr:
            self.url = self._makeStringAttribute(attr['url']).value
        if 'archive_download_url' in attr:
            self.archive_download_url = self._makeStringAttribute(
                attr['archive_download_url']).value
        if 'expired' in attr:
            self.expired = self._makeBoolAttribute(attr['expired']).value
        if 'created_at' in attr:
            self.created_at = self._makeDatetimeAttribute(
                attr['created_at']).value
        if 'updated_at' in attr:
            self.expires_at = self._makeDatetimeAttribute(
                attr['updated_at']).value

    def downloadBlob(self):
        url = self.archive_download_url
        logging.info('Fetching: %r' % url)

        status, headers, _ = self._requester.requestBlob('GET', url)

        if status != 302:
            raise Exception(
                'Expected a redirect during blob download but got status %d, headers %r.' % (status, headers))

        response = requests.get(headers['location'])
        response.raise_for_status()

        return response.content

    def delete(self):
        """Delete this artifact."""
        logging.warning('DELETING artifact ' + self.url)
        self._requester.requestJsonAndCheck('DELETE', self.url)


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


def getAllArtifacts(githubToken, githubRepo):
    """Get all artifacts visible in the given repo."""
    api = github.Github(githubToken)
    repo = api.get_repo(githubRepo)

    return ArtifactFetcher(repo).get_artifacts()
