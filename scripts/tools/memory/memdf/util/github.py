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
import pathlib
import subprocess
import time
from collections.abc import Iterable, Iterator, Mapping
from datetime import UTC, datetime, timedelta
from typing import Any

import dateutil  # type: ignore
import dateutil.parser  # type: ignore
import ghapi.all  # type: ignore
from memdf import Config, ConfigDescription

log = logging.getLogger(__name__)


def postprocess_config(config: Config, _key: str, _info: Mapping) -> None:
    """Postprocess --github-repository."""
    if config['github.repository']:
        owner, repo = config.get('github.repository').split('/', 1)
        config.put('github.owner', owner)
        config.put('github.repo', repo)
        if not config['github.token']:
            config['github.token'] = os.environ.get('GITHUB_TOKEN')
            if not config['github.token']:
                log.error("Missing --github-token")


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
    'github.artifacts-per-page': {
        'help': 'Request COUNT artifacts per page',
        'metavar': 'COUNT',
        'default': 100,
        'argparse': {
            'type': int,
        },
    },
    'github.retries': {
        'help': 'Retry transient GitHub API errors up to COUNT times',
        'metavar': 'COUNT',
        'default': 3,
        'argparse': {
            'type': int,
        },
    },
    'github.scan-hours': {
        'help': 'On fallback scans, examine workflow runs created in the last'
        ' COUNT hours',
        'metavar': 'COUNT',
        'default': 0,
        'argparse': {
            'type': int,
        },
    },
}


def _sync_paged(*args, **kwargs):
    """Synchronous pagination helper compatible with ghapi v1 and v2+."""
    paged_fn = getattr(ghapi.all, 'sync_paged', getattr(ghapi.all, 'paged'))
    return paged_fn(*args, **kwargs)


def _discover_size_workflow_paths(
        workflows_dir: pathlib.Path = pathlib.Path('.github/workflows')
) -> set[str]:
    """Return workflow paths in .github/workflows that upload size reports."""
    paths: set[str] = set()
    if not workflows_dir.is_dir():
        return paths
    for pattern in ('*.yaml', '*.yml'):
        for wf in workflows_dir.glob(pattern):
            try:
                content = wf.read_text(encoding='utf-8', errors='ignore')
            except OSError:
                continue
            if 'upload-size-reports' in content or 'gh_sizes.py' in content:
                paths.add(f'.github/workflows/{wf.name}')
    return paths


class Gh:
    """Utility wrapper for GitHub operations."""

    def __init__(self, config: Config):
        self.config = config
        self.ghapi: ghapi.all.GhApi | None = None
        self.deleted_artifacts: set[int] = set()
        self._size_workflow_paths: set[str] = _discover_size_workflow_paths()

        owner = config['github.owner']
        repo = config['github.repo']
        token = config['github.token']
        if owner and repo and token and token != 'SKIP':
            self.ghapi = ghapi.all.GhApi(owner=owner, repo=repo, token=token, sync=True)

    def __bool__(self):
        return self.ghapi is not None

    @staticmethod
    def _is_rate_limit_error(e: Exception) -> bool:
        """Return True if the exception indicates an exhausted API rate limit."""
        status = getattr(e, 'status_code', None)
        if status not in (403, 429):
            return False
        response = getattr(e, 'response', None)
        if response is None:
            return False
        return response.headers.get('x-ratelimit-remaining') == '0'

    def _call_with_retry(self, fn, *args, **kwargs) -> Any:
        """Call a GitHub API function with retries on transient errors.

        Rate limit errors are not retried; retrying cannot succeed until the
        limit window resets, and meanwhile it would only consume more of the
        remaining quota once it does.
        """
        retries = self.config.get('github.retries') or 1
        delay = 0.5
        for attempt in range(max(1, retries)):
            try:
                return fn(*args, **kwargs)
            except Exception as e:
                if self._is_rate_limit_error(e):
                    log.error("GitHub API rate limit exhausted: %r", e)
                    raise
                if attempt + 1 >= max(1, retries):
                    raise
                log.warning("Transient GitHub API error (attempt %d/%d): %r",
                            attempt + 1, retries, e)
                time.sleep(delay)
                delay *= 2
        return None

    def _is_candidate_workflow_run(self, run: Any) -> bool:
        """Return True if a workflow run can produce size artifacts."""
        event = getattr(run, 'event', None)
        if event and event not in ('pull_request', 'push'):
            return False
        path = getattr(run, 'path', None)
        if path and self._size_workflow_paths:
            return path in self._size_workflow_paths
        return True

    def get_comments_for_pr(self, pr: int):
        """Iterate PR comments."""
        assert self.ghapi
        try:
            return itertools.chain.from_iterable(
                _sync_paged(self.ghapi.issues.list_comments, pr))
        except Exception as e:
            log.exception("Failed to get comments for PR #%d: %r", pr, e)
            return []

    def get_commits_for_pr(self, pr: int):
        """Iterate PR commits."""
        assert self.ghapi
        try:
            return itertools.chain.from_iterable(
                _sync_paged(self.ghapi.pulls.list_commits, pr))
        except Exception as e:
            log.exception("Failed to get commits for PR #%d: %r", pr, e)
            return []

    def get_artifacts_for_run(self,
                              run_id: int,
                              per_page: int = 100) -> Iterator[Any]:
        """Iterate artifacts for a single workflow run."""
        assert self.ghapi
        page = 1
        while True:
            try:
                res = self._call_with_retry(
                    self.ghapi.actions.list_workflow_run_artifacts,
                    run_id=run_id,
                    per_page=per_page,
                    page=page)
            except Exception as e:
                log.warning("Failed to list artifacts for run %d: %r", run_id, e)
                break
            artifacts = getattr(res, 'artifacts', None)
            if not artifacts:
                break
            yield from artifacts
            if len(artifacts) < per_page:
                break
            page += 1

    def get_artifacts_from_workflow_runs(self,
                                         page_limit: int = -1,
                                         per_page: int = -1,
                                         head_sha: str = '',
                                         created_after: str = '') -> Iterator[Any]:
        """Iterate artifacts by querying completed workflow runs.

        When `created_after` is a GitHub repository filter date expression
        (like '>2024-01-01T00:00:00Z'), only runs created after that time are
        examined. This bounds the cost of a scan; without it, scanning runs on
        a large repository consumes many API calls per run examined and always
        restarts from the newest runs, preventing progress through a backlog.
        """
        if page_limit < 0:
            page_limit = self.config['github.limit-artifact-pages'] or 0
        if per_page < 0:
            per_page = self.config['github.artifacts-per-page'] or 100

        assert self.ghapi
        page = 1
        while not page_limit or page <= page_limit:
            kwargs: dict[str, Any] = {
                'status': 'completed',
                'per_page': per_page,
                'page': page,
            }
            if head_sha:
                kwargs['head_sha'] = head_sha
            if created_after:
                kwargs['created'] = created_after
            try:
                res = self._call_with_retry(
                    self.ghapi.actions.list_workflow_runs_for_repo, **kwargs)
            except Exception as e:
                log.exception("Failed to list workflow runs (page %d): %r",
                              page, e)
                break

            runs = getattr(res, 'workflow_runs', None)
            if not runs:
                break

            for run in runs:
                if not self._is_candidate_workflow_run(run):
                    continue
                yield from self.get_artifacts_for_run(run.id, per_page=per_page)

            log.debug("ASP: workflow run page %d of %d", page, page_limit)
            if len(runs) < per_page:
                break
            page += 1

    def _scan_hours(self) -> int:
        """Return the fallback scan time window in hours (0 for no limit)."""
        return self.config.get('github.scan-hours') or 0

    def _scan_created_after(self, now: datetime | None = None) -> str:
        """Return a GitHub `created` filter for the fallback scan window."""
        hours = self._scan_hours()
        if hours <= 0:
            return ''
        if now is None:
            now = datetime.now(UTC)
        return '>' + (now - timedelta(hours=hours)).strftime(
            '%Y-%m-%dT%H:%M:%SZ')

    def get_artifacts(self,
                      page_limit: int = -1,
                      per_page: int = -1) -> Iterator[Any]:
        """Iterate artifact descriptions, falling back to workflow runs on failure."""
        if page_limit < 0:
            page_limit = self.config['github.limit-artifact-pages'] or 0
        if per_page < 0:
            per_page = self.config['github.artifacts-per-page'] or 100

        assert self.ghapi
        seen_ids: set[int] = set()
        page = 1
        current_per_page = per_page
        repo_list_failed = False

        while not page_limit or page <= page_limit:
            try:
                res = self._call_with_retry(
                    self.ghapi.actions.list_artifacts_for_repo,
                    per_page=current_per_page,
                    page=page)
            except Exception as e:
                if current_per_page > 30 and page == 1:
                    current_per_page = 30
                    log.warning(
                        "Retrying list_artifacts_for_repo with per_page=%d after error: %r",
                        current_per_page, e)
                    continue
                log.warning(
                    "Failed to list repo artifacts on page %d (%r); falling back to workflow runs",
                    page, e)
                repo_list_failed = True
                break

            artifacts = getattr(res, 'artifacts', None)
            if not artifacts:
                break

            for a in artifacts:
                aid = getattr(a, 'id', None)
                if aid is not None:
                    if aid in seen_ids:
                        continue
                    seen_ids.add(aid)
                yield a

            log.debug("ASP: artifact page %d of %d", page, page_limit)
            if len(artifacts) < current_per_page:
                break
            page += 1

        if repo_list_failed:
            remaining_pages = max(1, page_limit - page + 1) if page_limit else 0
            for a in self.get_artifacts_from_workflow_runs(
                    page_limit=remaining_pages,
                    per_page=per_page,
                    created_after=self._scan_created_after()):
                aid = getattr(a, 'id', None)
                if aid is not None:
                    if aid in seen_ids:
                        continue
                    seen_ids.add(aid)
                yield a

    @staticmethod
    def parse_size_artifact(a: Any, label: str = '') -> Any | None:
        """Parse size artifact metadata from an artifact description."""
        if getattr(a, 'expired', False):
            return None
        name = getattr(a, 'name', '')
        # Size artifacts have names of the form:
        #   Size,{group},{pr},{commit_hash},{parent_hash}[,{event}]
        # This information is added to the attribute record from GitHub.
        if not (name.startswith('Size,') and name.count(',') >= 4):
            return None
        _, group, pr, commit, parent, *etc = name.split(',')
        if label and group != label:
            return None
        a.group = group
        a.commit = commit
        a.parent = parent
        a.pr = pr
        if isinstance(a.created_at, str):
            a.created_at = dateutil.parser.isoparse(a.created_at)
        # Old artifact names don't include the event.
        if etc:
            event = etc[0]
        else:
            event = 'push' if pr == '0' else 'pull_request'
        a.event = event
        return a

    def get_size_artifacts(self,
                           page_limit: int = -1,
                           per_page: int = -1,
                           label: str = '') -> Iterator[Any]:
        """Iterate size artifact descriptions."""
        for a in self.get_artifacts(page_limit, per_page):
            parsed = self.parse_size_artifact(a, label=label)
            if parsed is not None:
                yield parsed

    def get_size_artifacts_for_commit(self,
                                      commit_sha: str,
                                      label: str = '') -> Iterator[Any]:
        """Iterate size artifact descriptions for a specific commit SHA."""
        if not commit_sha:
            return
        seen_ids: set[int] = set()
        for a in self.get_artifacts_from_workflow_runs(head_sha=commit_sha):
            aid = getattr(a, 'id', None)
            if aid is not None:
                if aid in seen_ids:
                    continue
                seen_ids.add(aid)
            parsed = self.parse_size_artifact(a, label=label)
            if parsed is not None and parsed.commit == commit_sha:
                yield parsed

    def download_artifact(self, artifact_id: int):
        """Download a GitHub artifact, returning a binary zip object."""
        log.debug("Downloading artifact %d", artifact_id)
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
            log.exception("Failed to download artifact %d: %r", artifact_id, e)
        return None

    def delete_artifact(self, artifact_id: int) -> bool:
        """Delete a GitHub artifact."""
        if not artifact_id or artifact_id in self.deleted_artifacts:
            return True
        self.deleted_artifacts.add(artifact_id)

        if self.config['github.keep']:
            log.info("Suppressed deleting artifact %d", artifact_id)
            return False

        try:
            assert self.ghapi
            log.info("Deleting artifact %d", artifact_id)
            self.ghapi.actions.delete_artifact(artifact_id)
            return True
        except Exception as e:
            # During manual testing we sometimes lose the race against CI.
            log.exception("Failed to delete artifact %d: %r", artifact_id, e)
        return False

    def delete_artifacts(self, artifacts: Iterable[int]):
        for artifact_id in artifacts:
            self.delete_artifact(artifact_id)

    def create_comment(self, issue_id: int, text: str) -> bool:
        """Create a GitHub comment."""
        if self.config['github.dryrun-comment']:
            log.info("Suppressed creating comment on issue #%d", issue_id)
            log.debug(text)
            return False

        assert self.ghapi
        log.info("Creating comment on issue #%d", issue_id)
        try:
            self.ghapi.issues.create_comment(issue_id, text)
            return True
        except Exception as e:
            log.exception("Failed to created comment on issue #%d: %r", issue_id, e)
        return False

    def update_comment(self, comment_id: int, text: str) -> bool:
        """Update a GitHub comment."""
        if self.config['github.dryrun-comment']:
            log.info("Suppressed updating comment #%d", comment_id)
            log.debug(text)
            return False

        log.info("Updating comment #%d", comment_id)
        try:
            assert self.ghapi
            self.ghapi.issues.update_comment(comment_id, text)
            return True
        except Exception as e:
            log.exception("Failed to update comment %d: %r", comment_id, e)
        return False
