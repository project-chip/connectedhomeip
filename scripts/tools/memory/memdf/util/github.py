#
# Copyright (c) 2022 Project CHIP Authors
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
#
"""Utility wrapper for GitHub operations."""

import itertools
import logging
import os
import subprocess
from typing import Iterable, Mapping, Optional

import dateutil  # type: ignore
import dateutil.parser  # type: ignore
import ghapi.all  # type: ignore
from memdf import Config, ConfigDescription


def postprocess_config(config: Config, _key: str, _info: Mapping) -> None:
    """Postprocess --github-repository."""
    if config['github.repository']:
        owner, repo = config.get('github.repository').split('/', 1)
        config.put('github.owner', owner)
        config.put('github.repo', repo)
        if not config['github.token']:
            config['github.token'] = os.environ.get('GITHUB_TOKEN')
            if not config['github.token']:
                logging.error('Missing --github-token')


CONFIG: ConfigDescription = {
    Config.group_def('github'): {
        'title': 'github options',
    },
    'github.token': {
        'help': 'Github API token, or "SKIP" to suppress connecting to github',
        'metavar': 'TOKEN',
        'default': '',
        'argparse': {
            'alias': ['--github-api-token', '--token'],
        },
    },
    'github.repository': {
        'help': 'Github repostiory',
        'metavar': 'OWNER/REPO',
        'default': '',
        'argparse': {
            'alias': ['--repo'],
        },
        'postprocess': postprocess_config,
    },
    'github.dryrun-comment': {
        'help': "Don't actually post comments",
        'default': False,
    },
    'github.keep': {
        'help': "Don't remove PR artifacts",
        'default': False,
        'argparse': {
            'alias': ['--keep'],
        },
    },
    'github.limit-artifact-pages': {
        'help': 'Examine no more than COUNT pages of artifacts',
        'metavar': 'COUNT',
        'default': 0,
        'argparse': {
            'type': int,
        },
    },
}


class Gh:
    """Utility wrapper for GitHub operations."""

    def __init__(self, config: Config):
        self.config = config
        self.ghapi: Optional[ghapi.all.GhApi] = None
        self.deleted_artifacts: set[int] = set()

        owner = config['github.owner']
        repo = config['github.repo']
        token = config['github.token']
        if owner and repo and token and token != 'SKIP':
            self.ghapi = ghapi.all.GhApi(owner=owner, repo=repo, token=token)

    def __bool__(self):
        return self.ghapi is not None

    def get_comments_for_pr(self, pr: int):
        """Iterate PR comments."""
        assert self.ghapi
        try:
            return itertools.chain.from_iterable(
                ghapi.all.paged(self.ghapi.issues.list_comments, pr))
        except Exception as e:
            logging.error('Failed to get comments for PR #%d: %s', pr, e)
            return []

    def get_commits_for_pr(self, pr: int):
        """Iterate PR commits."""
        assert self.ghapi
        try:
            return itertools.chain.from_iterable(
                ghapi.all.paged(self.ghapi.pulls.list_commits, pr))
        except Exception as e:
            logging.error('Failed to get commits for PR #%d: %s', pr, e)
            return []

    def get_artifacts(self, page_limit: int = -1, per_page: int = -1):
        """Iterate artifact descriptions."""
        if page_limit < 0:
            page_limit = self.config['github.limit-artifact-pages']
        if per_page < 0:
            per_page = self.config['github.artifacts-per-page'] or 100

        assert self.ghapi
        try:
            page = 0
            for i in ghapi.all.paged(
                    self.ghapi.actions.list_artifacts_for_repo,
                    per_page=per_page):
                if not i.artifacts:
                    break
                for a in i.artifacts:
                    yield a
                page += 1
                logging.debug('ASP: artifact page %d of %d', page, page_limit)
                if page_limit and page >= page_limit:
                    break
        except Exception as e:
            logging.error('Failed to get artifact list: %s', e)

    def get_size_artifacts(self,
                           page_limit: int = -1,
                           per_page: int = -1,
                           label: str = ''):
        """Iterate size artifact descriptions."""
        for a in self.get_artifacts(page_limit, per_page):
            # Size artifacts have names of the form:
            #   Size,{group},{pr},{commit_hash},{parent_hash}[,{event}]
            # This information is added to the attribute record from GitHub.
            if a.name.startswith('Size,') and a.name.count(',') >= 4:
                _, group, pr, commit, parent, *etc = a.name.split(',')
                if label and group != label:
                    continue
                a.group = group
                a.commit = commit
                a.parent = parent
                a.pr = pr
                a.created_at = dateutil.parser.isoparse(a.created_at)
                # Old artifact names don't include the event.
                if etc:
                    event = etc[0]
                else:
                    event = 'push' if pr == '0' else 'pull_request'
                a.event = event
                yield a

    def download_artifact(self, artifact_id: int):
        """Download a GitHub artifact, returning a binary zip object."""
        logging.debug('Downloading artifact %d', artifact_id)
        try:
            assert self.ghapi

            # It seems like github artifact download is at least partially broken
            # (see https://github.com/project-chip/connectedhomeip/issues/32656)
            #
            # This makes `self.ghapi.actions.download_artifact` not work
            #
            # Oddly enough downloading via CURL seems ok
            owner = self.config['github.owner']
            repo = self.config['github.repo']
            token = self.config['github.token']

            download_url = f"https://api.github.com/repos/{owner}/{repo}/actions/artifacts/{artifact_id}/zip"

            # Follow https://docs.github.com/en/rest/actions/artifacts?apiVersion=2022-11-28#download-an-artifact
            return subprocess.check_output(
                [
                    'curl',
                    '-L',
                    '-H', 'Accept: application/vnd.github+json',
                    '-H', f'Authorization: Bearer {token}',
                    '-H', 'X-GitHub-Api-Version: 2022-11-28',
                    '--output', '-',
                    download_url
                ]
            )
        except Exception as e:
            logging.error('Failed to download artifact %d: %s', artifact_id, e)
        return None

    def delete_artifact(self, artifact_id: int) -> bool:
        """Delete a GitHub artifact."""
        if not artifact_id or artifact_id in self.deleted_artifacts:
            return True
        self.deleted_artifacts.add(artifact_id)

        if self.config['github.keep']:
            logging.info('Suppressed deleting artifact %d', artifact_id)
            return False

        try:
            assert self.ghapi
            logging.info('Deleting artifact %d', artifact_id)
            self.ghapi.actions.delete_artifact(artifact_id)
            return True
        except Exception as e:
            # During manual testing we sometimes lose the race against CI.
            logging.error('Failed to delete artifact %d: %s', artifact_id, e)
        return False

    def delete_artifacts(self, artifacts: Iterable[int]):
        for artifact_id in artifacts:
            self.delete_artifact(artifact_id)

    def create_comment(self, issue_id: int, text: str) -> bool:
        """Create a GitHub comment."""
        if self.config['github.dryrun-comment']:
            logging.info('Suppressed creating comment on #%d', issue_id)
            logging.debug('%s', text)
            return False

        assert self.ghapi
        logging.info('Creating comment on #%d', issue_id)
        try:
            self.ghapi.issues.create_comment(issue_id, text)
            return True
        except Exception as e:
            logging.error('Failed to created comment on #%d: %s', issue_id, e)
        return False

    def update_comment(self, comment_id: int, text: str) -> bool:
        """Update a GitHub comment."""
        if self.config['github.dryrun-comment']:
            logging.info('Suppressed updating comment #%d', comment_id)
            logging.debug('%s', text)
            return False

        logging.info('Updating comment #%d', comment_id)
        try:
            assert self.ghapi
            self.ghapi.issues.update_comment(comment_id, text)
            return True
        except Exception as e:
            logging.error('Failed to update comment %d: %s', comment_id, e)
        return False
