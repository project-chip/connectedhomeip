#!/usr/bin/env python3
#
# Copyright (c) 2026 Project CHIP Authors
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
"""Unit tests for GitHub artifact discovery in gh_report and memdf.util.github."""

import io
import json
import pathlib
import sys
import unittest
import zipfile
from types import SimpleNamespace
from unittest import mock

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import gh_report  # noqa: E402
import memdf.report  # noqa: E402
import memdf.util.config  # noqa: E402
import memdf.util.github  # noqa: E402
import memdf.util.sqlite  # noqa: E402
from memdf import Config  # noqa: E402
from memdf.util.github import Gh  # noqa: E402


def _make_config(**overrides) -> Config:
    config = Config().init({
        **memdf.util.config.CONFIG,
        **memdf.util.github.CONFIG,
        **memdf.util.sqlite.CONFIG,
        **memdf.report.OUTPUT_CONFIG,
        **gh_report.GITHUB_CONFIG,
        **gh_report.DB_CONFIG,
        **gh_report.REPORT_CONFIG,
    })
    config.put('github.owner', 'project-chip')
    config.put('github.repo', 'connectedhomeip')
    config.put('github.token', 'SKIP')
    config.put('github.retries', 1)
    config.put('report.pr', True)
    config.put('args.inputs', [])
    for k, v in overrides.items():
        config.put(k, v)
    return config


def _make_size_zip(commit_hash: str, parent_hash: str, pr: int, event: str,
                   flash_size: int) -> bytes:
    payload = {
        'platform': 'esp32',
        'config': 'devkitc',
        'target': 'lock-app',
        'time': 1700000000,
        'input': 'lock-app.elf',
        'event': event,
        'hash': commit_hash,
        'parent': parent_hash,
        'pr': pr,
        'by': 'section',
        'frames': {
            'section': [{
                'section': '.text',
                'size': flash_size
            }],
            'region': [{
                'region': 'FLASH',
                'size': flash_size
            }],
        },
    }
    buf = io.BytesIO()
    with zipfile.ZipFile(buf, 'w') as zf:
        zf.writestr('esp32-devkitc-lock-app-sizes.json', json.dumps(payload))
    return buf.getvalue()


class TestGithubArtifactDiscovery(unittest.TestCase):
    """Tests for resilient artifact listing and targeted commit lookups."""

    def test_fallback_to_workflow_runs_when_repo_artifact_list_fails(self):
        """When list_artifacts_for_repo fails (e.g. 502), fallback to workflow runs."""
        config = _make_config(**{'github.limit-artifact-pages': 2})
        gh = Gh(config)
        gh.ghapi = mock.MagicMock()
        gh._size_workflow_paths = {'.github/workflows/examples-esp32.yaml'}

        gh.ghapi.actions.list_artifacts_for_repo.side_effect = RuntimeError(
            'HTTP 502 Bad Gateway')

        run_ignored_schedule = SimpleNamespace(
            id=10,
            event='schedule',
            path='.github/workflows/bloat_check.yaml',
        )
        run_ignored_workflow = SimpleNamespace(
            id=11,
            event='pull_request',
            path='.github/workflows/tests.yaml',
        )
        run_esp32 = SimpleNamespace(
            id=12,
            event='pull_request',
            path='.github/workflows/examples-esp32.yaml',
        )
        gh.ghapi.actions.list_workflow_runs_for_repo.return_value = (
            SimpleNamespace(workflow_runs=[
                run_ignored_schedule,
                run_ignored_workflow,
                run_esp32,
            ]))

        artifact_pr = SimpleNamespace(
            id=501,
            name='Size,ESP32-Examples,42,aaaa1111,bbbb0000,pull_request',
            created_at='2026-10-15T10:00:00Z',
            expired=False,
        )
        gh.ghapi.actions.list_workflow_run_artifacts.return_value = (
            SimpleNamespace(artifacts=[artifact_pr]))

        found = list(gh.get_size_artifacts())
        self.assertEqual(len(found), 1)
        self.assertEqual(found[0].id, 501)
        self.assertEqual(found[0].group, 'ESP32-Examples')
        self.assertEqual(found[0].commit, 'aaaa1111')
        self.assertEqual(found[0].parent, 'bbbb0000')
        gh.ghapi.actions.list_workflow_run_artifacts.assert_called_once_with(
            run_id=12, per_page=100, page=1)

    def test_fetch_missing_parent_artifact_by_commit_sha(self):
        """Missing parent artifacts are fetched directly via head_sha workflow runs."""
        config = _make_config()
        szc = gh_report.SizeContext(config)
        self.addCleanup(szc.db.close)
        szc.gh.ghapi = mock.MagicMock()
        szc.gh._size_workflow_paths = {'.github/workflows/examples-esp32.yaml'}

        pr_artifact = SimpleNamespace(
            id=1001,
            name='Size,ESP32-Examples,99,pr_sha_1234,base_sha_5678,pull_request',
            created_at='2026-10-15T11:00:00Z',
            expired=False,
        )
        parent_artifact = SimpleNamespace(
            id=1002,
            name='Size,ESP32-Examples,0,base_sha_5678,older_sha_0000,push',
            created_at='2026-10-10T08:00:00Z',
            expired=False,
        )

        # Repo-wide artifact listing only returns the recent PR artifact; the
        # parent artifact is older and not in the initial listing.
        szc.gh.ghapi.actions.list_artifacts_for_repo.return_value = (
            SimpleNamespace(artifacts=[pr_artifact]))

        parent_run = SimpleNamespace(
            id=77,
            event='push',
            path='.github/workflows/examples-esp32.yaml',
        )
        szc.gh.ghapi.actions.list_workflow_runs_for_repo.return_value = (
            SimpleNamespace(workflow_runs=[parent_run]))
        szc.gh.ghapi.actions.list_workflow_run_artifacts.return_value = (
            SimpleNamespace(artifacts=[parent_artifact]))

        zip_blobs = {
            1001:
                _make_size_zip('pr_sha_1234', 'base_sha_5678', 99,
                               'pull_request', 10200),
            1002:
                _make_size_zip('base_sha_5678', 'older_sha_0000', 0, 'push',
                               10000),
        }
        szc.gh.download_artifact = mock.MagicMock(
            side_effect=lambda aid: zip_blobs.get(aid))

        szc.read_inputs()
        dfs = szc.report_matching_commits()

        self.assertIn('99,base_sha_5678,pr_sha_1234', dfs)
        df = dfs['99,base_sha_5678,pr_sha_1234']
        self.assertEqual(int(df.iloc[0]['change']), 200)
        szc.gh.ghapi.actions.list_workflow_runs_for_repo.assert_called_once_with(
            status='completed',
            per_page=100,
            page=1,
            head_sha='base_sha_5678',
        )

    def test_limit_pr_direct_commit_lookup(self):
        """When --github-limit-pr is set and missing from scan, lookup PR head and base directly."""
        config = _make_config(**{'github.limit-pr': 99})
        szc = gh_report.SizeContext(config)
        self.addCleanup(szc.db.close)
        szc.gh.ghapi = mock.MagicMock()
        szc.gh._size_workflow_paths = {'.github/workflows/examples-esp32.yaml'}
        szc.get_newest_commit = mock.MagicMock(return_value='pr_sha_1234')

        # Initial repo scan returns nothing.
        szc.gh.ghapi.actions.list_artifacts_for_repo.return_value = (
            SimpleNamespace(artifacts=[]))

        pr_artifact = SimpleNamespace(
            id=3001,
            name='Size,ESP32-Examples,99,pr_sha_1234,base_sha_5678,pull_request',
            created_at='2026-10-15T11:00:00Z',
            expired=False,
        )
        parent_artifact = SimpleNamespace(
            id=3002,
            name='Size,ESP32-Examples,0,base_sha_5678,older_sha_0000,push',
            created_at='2026-10-10T08:00:00Z',
            expired=False,
        )

        def fake_list_runs(**kwargs):
            if kwargs.get('head_sha') == 'pr_sha_1234':
                return SimpleNamespace(workflow_runs=[
                    SimpleNamespace(
                        id=88,
                        event='pull_request',
                        path='.github/workflows/examples-esp32.yaml',
                    )
                ])
            if kwargs.get('head_sha') == 'base_sha_5678':
                return SimpleNamespace(workflow_runs=[
                    SimpleNamespace(
                        id=77,
                        event='push',
                        path='.github/workflows/examples-esp32.yaml',
                    )
                ])
            return SimpleNamespace(workflow_runs=[])

        def fake_list_run_artifacts(run_id, **kwargs):
            if run_id == 88:
                return SimpleNamespace(artifacts=[pr_artifact])
            if run_id == 77:
                return SimpleNamespace(artifacts=[parent_artifact])
            return SimpleNamespace(artifacts=[])

        szc.gh.ghapi.actions.list_workflow_runs_for_repo.side_effect = (
            fake_list_runs)
        szc.gh.ghapi.actions.list_workflow_run_artifacts.side_effect = (
            fake_list_run_artifacts)

        zip_blobs = {
            3001:
                _make_size_zip('pr_sha_1234', 'base_sha_5678', 99,
                               'pull_request', 9900),
            3002:
                _make_size_zip('base_sha_5678', 'older_sha_0000', 0, 'push',
                               10000),
        }
        szc.gh.download_artifact = mock.MagicMock(
            side_effect=lambda aid: zip_blobs.get(aid))

        szc.read_inputs()
        dfs = szc.report_matching_commits()

        self.assertIn('99,base_sha_5678,pr_sha_1234', dfs)
        df = dfs['99,base_sha_5678,pr_sha_1234']
        self.assertEqual(int(df.iloc[0]['change']), -100)

    def test_parent_fetch_recording_new_group_does_not_crash(self):
        """Parent lookup may encounter artifacts for new groups while the
        required-artifact scan iterates the (group, commit) map."""
        config = _make_config()
        szc = gh_report.SizeContext(config)
        self.addCleanup(szc.db.close)
        szc.gh.ghapi = mock.MagicMock()
        szc.gh._size_workflow_paths = {'.github/workflows/examples-esp32.yaml'}

        child_artifact = SimpleNamespace(
            id=4001,
            name='Size,ESP32-Examples,99,pr_sha_1234,base_sha_5678,pull_request',
            created_at='2026-10-15T11:00:00Z',
            expired=False,
        )
        szc.gh.ghapi.actions.list_artifacts_for_repo.return_value = (
            SimpleNamespace(artifacts=[child_artifact]))

        parent_run = SimpleNamespace(
            id=77,
            event='push',
            path='.github/workflows/examples-esp32.yaml',
        )
        szc.gh.ghapi.actions.list_workflow_runs_for_repo.return_value = (
            SimpleNamespace(workflow_runs=[parent_run]))
        # The parent's runs carry artifacts for two groups, one of which was
        # not seen in the initial scan.
        parent_artifacts = [
            SimpleNamespace(
                id=4002,
                name='Size,ESP32-Examples,0,base_sha_5678,older_sha_0000,push',
                created_at='2026-10-10T08:00:00Z',
                expired=False,
            ),
            SimpleNamespace(
                id=4003,
                name='Size,NXP-Examples,0,base_sha_5678,older_sha_0000,push',
                created_at='2026-10-10T08:00:00Z',
                expired=False,
            ),
        ]
        szc.gh.ghapi.actions.list_workflow_run_artifacts.return_value = (
            SimpleNamespace(artifacts=parent_artifacts))

        zip_blobs = {
            4001:
                _make_size_zip('pr_sha_1234', 'base_sha_5678', 99,
                               'pull_request', 10200),
            4002:
                _make_size_zip('base_sha_5678', 'older_sha_0000', 0, 'push',
                               10000),
        }
        szc.gh.download_artifact = mock.MagicMock(
            side_effect=lambda aid: zip_blobs.get(aid))

        szc.read_inputs()
        dfs = szc.report_matching_commits()
        self.assertIn('99,base_sha_5678,pr_sha_1234', dfs)
        self.assertEqual(
            int(dfs['99,base_sha_5678,pr_sha_1234'].iloc[0]['change']), 200)

    def test_fallback_scan_window_bounds_runs_listing(self):
        """With --github-scan-hours, fallback run scans pass a created filter."""
        config = _make_config(**{'github.scan-hours': 6})
        gh = Gh(config)
        gh.ghapi = mock.MagicMock()
        gh.ghapi.actions.list_artifacts_for_repo.side_effect = RuntimeError(
            'HTTP 502 Bad Gateway')
        gh.ghapi.actions.list_workflow_runs_for_repo.return_value = (
            SimpleNamespace(workflow_runs=[]))

        self.assertEqual(list(gh.get_size_artifacts()), [])
        calls = gh.ghapi.actions.list_workflow_runs_for_repo.call_args_list
        self.assertEqual(len(calls), 1)
        self.assertRegex(calls[0].kwargs['created'],
                         r'^>\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z$')

    def test_rate_limit_errors_are_not_retried(self):
        """Retry stops immediately (rather than wasting quota) on exhausted rate limits."""

        class _FakeRateLimitError(RuntimeError):
            pass

        error = _FakeRateLimitError('API rate limit exhausted')
        error.status_code = 403
        error.response = SimpleNamespace(
            headers={'x-ratelimit-remaining': '0'})

        config = _make_config(**{'github.retries': 5})
        gh = Gh(config)
        fn = mock.MagicMock(side_effect=error)
        with self.assertRaises(_FakeRateLimitError):
            gh._call_with_retry(fn)
        self.assertEqual(fn.call_count, 1)

    def test_scan_created_after_window(self):
        """The scan window produces a GitHub created filter, or is empty."""
        gh = Gh(_make_config())
        self.assertEqual(gh._scan_created_after(), '')
        gh = Gh(_make_config(**{'github.scan-hours': 6}))
        self.assertRegex(gh._scan_created_after(),
                         r'^>\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z$')

    def test_expired_artifacts_ignored_and_newest_kept(self):
        """Expired artifacts are skipped and newer artifacts are not overwritten by older ones."""
        expired = SimpleNamespace(
            id=2001,
            name='Size,ESP32-Examples,42,aaaa,bbbb,pull_request',
            created_at='2026-10-15T12:00:00Z',
            expired=True,
        )
        self.assertIsNone(Gh.parse_size_artifact(expired))

        newer = Gh.parse_size_artifact(
            SimpleNamespace(
                id=2002,
                name='Size,ESP32-Examples,42,aaaa,bbbb,pull_request',
                created_at='2026-10-15T12:00:00Z',
                expired=False,
            ))
        older = Gh.parse_size_artifact(
            SimpleNamespace(
                id=2003,
                name='Size,ESP32-Examples,42,aaaa,bbbb,pull_request',
                created_at='2026-10-15T09:00:00Z',
                expired=False,
            ))
        self.assertIsNotNone(newer)
        self.assertIsNotNone(older)

        size_artifacts: dict = {}
        gh_report.SizeContext._record_size_artifact(size_artifacts, newer)
        gh_report.SizeContext._record_size_artifact(size_artifacts, older)
        self.assertEqual(size_artifacts['ESP32-Examples']['aaaa'].id, 2002)


if __name__ == '__main__':
    unittest.main()
