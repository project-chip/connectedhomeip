#!/usr/bin/env python3
#
# Copyright (c) 2026 Project CHIP Authors
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

import json
import os
import sys
import tempfile
import unittest
from unittest.mock import MagicMock, patch

# Ensure the parent directory is in the path so we can import platform_merge_bot
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

# isort: split

# pylint: disable=wrong-import-position
from platform_merge_bot import ELIGIBILITY_COMMENT_MARKER, PlatformMergeBot  # noqa: E402


class TestPlatformMergeBot(unittest.TestCase):
    """Unit tests for the platform merge bot."""

    def setUp(self) -> None:
        """Sets up the test case by creating a temporary config file and mocking the Github API."""
        # Create a temporary config file for testing
        self.config_content = """
nxp:
  maintainers:
    - doru91
    - nxpdev
  paths:
    - "src/platform/nxp/**"
    - "examples/**/nxp/**"

esp32:
  maintainers:
    - esp32dev
  paths:
    - "src/platform/ESP32/**"
"""
        with tempfile.NamedTemporaryFile(delete=False, mode="w", suffix=".yaml") as f:
            f.write(self.config_content)
            self.temp_config_name = f.name
        self.addCleanup(os.unlink, self.temp_config_name)

        # Patch Github initialization so we don't hit the network
        self.github_patcher = patch("platform_merge_bot.Github")
        self.mock_github_class = self.github_patcher.start()
        self.addCleanup(self.github_patcher.stop)

        self.mock_github_instance = MagicMock()
        self.mock_github_class.return_value = self.mock_github_instance
        self.mock_repo = MagicMock()
        self.mock_github_instance.get_repo.return_value = self.mock_repo

        # Default mock commit with passing CI
        self.mock_commit = MagicMock()
        self.mock_commit.get_combined_status.return_value.state = "success"
        self.mock_commit.get_combined_status.return_value.total_count = 0
        self.mock_commit.get_combined_status.return_value.statuses = []
        # Default mock 10 successful check suites to pass the minimum checks count guard
        mock_suite = MagicMock()
        mock_suite.status = "completed"
        mock_suite.conclusion = "success"
        self.mock_commit.get_check_suites.return_value = [mock_suite] * 10
        self.mock_repo.get_commit.return_value = self.mock_commit

        # Initialize bot under test
        self.bot = PlatformMergeBot(
            token="dummy_token",
            repo_name="dummy/repo",
            config_path=self.temp_config_name,
            dry_run=False,
        )
        self.bot._bot_username = "platform-merge-bot"
        self.bot._get_unresolved_threads = MagicMock(return_value=[])
        self.bot._is_pullapprove_green = MagicMock(return_value=False)

    def create_mock_file(self, filename: str) -> MagicMock:
        """Creates a mock file object with the given filename."""
        mock_file = MagicMock()
        mock_file.filename = filename
        mock_file.previous_filename = None
        return mock_file

    def create_mock_review(self, username: str, state: str) -> MagicMock:
        """Creates a mock review object with the given user and state."""
        mock_review = MagicMock()
        mock_review.user.login = username
        mock_review.state = state
        return mock_review

    def create_mock_pr(
        self,
        number: int,
        title: str,
        author: str,
        files: list[MagicMock],
        reviews: list[MagicMock],
        comments: list[MagicMock] | None = None,
        draft: bool = False,
    ) -> MagicMock:
        """Creates a mock PullRequest object with the given parameters."""
        mock_pr = MagicMock()
        mock_pr.number = number
        mock_pr.title = title
        mock_pr.user.login = author
        mock_pr.draft = draft
        mock_pr.state = "open"
        mock_pr.mergeable = True
        mock_pr.get_files.return_value = files
        mock_pr.get_reviews.return_value = reviews
        mock_pr.get_issue_comments.return_value = comments or []
        mock_pr.head.sha = "dummy_sha"
        return mock_pr

    def test_config_loading(self) -> None:
        """Tests that the YAML config is parsed correctly into PlatformGroup objects."""
        self.assertEqual(len(self.bot.groups), 2)
        self.assertIn("nxp", self.bot.groups)
        self.assertIn("esp32", self.bot.groups)

        nxp_group = self.bot.groups["nxp"]
        self.assertEqual(nxp_group.maintainers, ["doru91", "nxpdev"])
        self.assertEqual(nxp_group.paths, ["examples/**/nxp/**", "src/platform/nxp/**"])

    def test_file_matching(self) -> None:
        """Tests that file path matching against platform groups functions correctly."""
        nxp_group = self.bot.groups["nxp"]

        self.assertTrue(
            nxp_group.matches_file("src/platform/nxp/SystemTimeSupport.cpp")
        )
        self.assertTrue(
            nxp_group.matches_file("src/platform/nxp/rt/rt1060/SystemTimeSupport.cpp")
        )
        self.assertTrue(
            nxp_group.matches_file("examples/all-clusters-app/nxp/main.cpp")
        )

        self.assertFalse(
            nxp_group.matches_file("src/platform/ESP32/SystemTimeSupport.cpp")
        )
        self.assertFalse(nxp_group.matches_file("src/app/Command.cpp"))

    def test_get_pr_review_states(self) -> None:
        """Tests that the correct active reviewers are identified from the chronological reviews list."""
        reviews = [
            self.create_mock_review("doru91", "COMMENTED"),
            self.create_mock_review("nxpdev", "APPROVED"),
            self.create_mock_review("doru91", "CHANGES_REQUESTED"),
            self.create_mock_review("doru91", "APPROVED"),  # approved later
            self.create_mock_review(
                "nxpdev", "CHANGES_REQUESTED"
            ),  # requested changes later
        ]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", [], reviews)
        approvers, change_requesters = self.bot.get_pr_review_states(mock_pr)

        # doru91 should be in approvers (latest was approved)
        # nxpdev should NOT be in approvers (latest was changes requested)
        self.assertIn("doru91", approvers)
        self.assertNotIn("nxpdev", approvers)
        self.assertIn("nxpdev", change_requesters)

        # test dismissed
        reviews2 = [
            self.create_mock_review("doru91", "APPROVED"),
            self.create_mock_review("doru91", "DISMISSED"),
        ]
        mock_pr2 = self.create_mock_pr(2, "Test PR 2", "author", [], reviews2)
        approvers2, change_requesters2 = self.bot.get_pr_review_states(mock_pr2)
        self.assertNotIn("doru91", approvers2)

    def test_analyze_pr_files_fully_covered(self) -> None:
        """Tests that all changed files are covered by platform groups."""
        files = [
            self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp"),
            self.create_mock_file("examples/all-clusters-app/nxp/main.cpp"),
        ]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, [])
        matched_files, uncovered = self.bot.analyze_pr_files(mock_pr)

        self.assertEqual(uncovered, set())
        self.assertEqual(
            matched_files["nxp"],
            {
                "src/platform/nxp/SystemTimeSupport.cpp",
                "examples/all-clusters-app/nxp/main.cpp",
            },
        )
        self.assertEqual(matched_files["esp32"], set())

    def test_analyze_pr_files_not_fully_covered(self) -> None:
        """Tests that uncovered files are correctly identified."""
        files = [
            self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp"),
            self.create_mock_file("src/app/Command.cpp"),  # Uncovered
        ]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, [])
        matched_files, uncovered = self.bot.analyze_pr_files(mock_pr)

        self.assertEqual(uncovered, {"src/app/Command.cpp"})
        self.assertEqual(
            matched_files["nxp"], {"src/platform/nxp/SystemTimeSupport.cpp"}
        )

    def test_analyze_pr_files_rename_bypass(self) -> None:
        """Tests that previous filenames of renamed files are also checked for coverage."""
        mock_file = self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp")
        mock_file.previous_filename = "src/app/Command.cpp"
        files = [mock_file]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, [])
        _, uncovered = self.bot.analyze_pr_files(mock_pr)

        self.assertIn("src/app/Command.cpp", uncovered)

    def test_check_and_process_pr_draft(self) -> None:
        """Tests that drafts are skipped entirely without checking files."""
        mock_pr = self.create_mock_pr(1, "Test PR", "author", [], [], draft=True)
        self.bot.check_and_process_pr(mock_pr)

        # Verify no files were fetched
        mock_pr.get_files.assert_not_called()

    def test_check_and_process_pr_not_eligible(self) -> None:
        """Tests that PR containing uncovered files is skipped and not merged."""
        files = [
            self.create_mock_file("src/app/Command.cpp"),
        ]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, [])
        self.bot.check_and_process_pr(mock_pr)

        # Should not merge
        mock_pr.merge.assert_not_called()
        mock_pr.create_issue_comment.assert_not_called()

    def test_check_and_process_pr_needs_approval(self) -> None:
        """Tests that PR lacking platform maintainer approvals does not merge and gets an eligibility comment."""
        files = [
            self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp"),
        ]
        # Approver is 'random_user', not a maintainer
        reviews = [
            self.create_mock_review("random_user", "APPROVED"),
        ]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)
        self.bot.check_and_process_pr(mock_pr)

        # Should not merge, but should post eligibility comment
        mock_pr.merge.assert_not_called()
        mock_pr.create_issue_comment.assert_called_once()
        self.assertIn(
            ELIGIBILITY_COMMENT_MARKER,
            mock_pr.create_issue_comment.call_args[0][0],
        )
        self.assertIn("Needs approval", mock_pr.create_issue_comment.call_args[0][0])

    def test_check_and_process_pr_already_commented_updates_if_different(self) -> None:
        """Tests that an existing eligibility comment is edited if the status has changed."""
        files = [
            self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp"),
        ]
        reviews = []
        # Existing eligibility comment with different body
        mock_comment = MagicMock()
        mock_comment.user.login = "platform-merge-bot"
        mock_comment.body = f"{ELIGIBILITY_COMMENT_MARKER}\nOutdated Info..."
        mock_pr = self.create_mock_pr(
            1, "Test PR", "author", files, reviews, comments=[mock_comment]
        )

        self.bot.check_and_process_pr(mock_pr)

        # Should not merge, should not create a new comment, but should edit the existing one
        mock_pr.merge.assert_not_called()
        mock_pr.create_issue_comment.assert_not_called()
        mock_comment.edit.assert_called_once()

    def test_check_and_process_pr_already_commented_no_op_if_identical(self) -> None:
        """Tests that an existing eligibility comment is untouched if the status is identical."""
        files = [
            self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp"),
        ]
        reviews = []
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)

        # First run: posts the eligibility comment
        self.bot.check_and_process_pr(mock_pr)
        mock_pr.create_issue_comment.assert_called_once()
        posted_comment_body = mock_pr.create_issue_comment.call_args[0][0]

        # Set up the PR to return the posted comment in subsequent get_issue_comments() calls
        mock_comment = MagicMock()
        mock_comment.user.login = "platform-merge-bot"
        mock_comment.body = posted_comment_body
        mock_pr.get_issue_comments.return_value = [mock_comment]

        # Reset mocks
        mock_pr.merge.reset_mock()
        mock_pr.create_issue_comment.reset_mock()
        mock_comment.edit.reset_mock()

        # Second run: should be a no-op since status is identical
        self.bot.check_and_process_pr(mock_pr)

        mock_pr.merge.assert_not_called()
        mock_pr.create_issue_comment.assert_not_called()
        mock_comment.edit.assert_not_called()

    def test_check_and_process_pr_fully_approved_merge(self) -> None:
        """Tests that PR is merged and commented when fully approved."""
        files = [
            self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp"),
        ]
        reviews = [
            self.create_mock_review("doru91", "APPROVED"),
        ]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)

        self.bot.check_and_process_pr(mock_pr)

        # Should merge and comment
        self.assertEqual(mock_pr.create_issue_comment.call_count, 1)
        self.assertIn(
            "Platform Maintainers Auto-Merge Executed",
            mock_pr.create_issue_comment.call_args[0][0],
        )
        self.assertIn(
            "approved by @doru91", mock_pr.create_issue_comment.call_args[0][0]
        )

        mock_pr.merge.assert_called_once_with(
            merge_method="squash",
            commit_title="Test PR (Auto-merged by platform-bot)",
            sha="dummy_sha",
        )

    def test_check_and_process_pr_multi_group_approved(self) -> None:
        """Tests that PR is merged when all affected platform groups have approved."""
        files = [
            self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp"),
            self.create_mock_file("src/platform/ESP32/SystemTimeSupport.cpp"),
        ]
        reviews = [
            self.create_mock_review("doru91", "APPROVED"),
            self.create_mock_review("esp32dev", "APPROVED"),
        ]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)

        self.bot.check_and_process_pr(mock_pr)

        # Should merge because both groups are approved
        mock_pr.merge.assert_called_once()
        self.assertIn(
            "approved by @doru91", mock_pr.create_issue_comment.call_args[0][0]
        )
        self.assertIn(
            "approved by @esp32dev", mock_pr.create_issue_comment.call_args[0][0]
        )

    def test_check_and_process_pr_multi_group_missing_one_approval(self) -> None:
        """Tests that PR is not merged and lists pending groups when only some groups approved."""
        files = [
            self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp"),
            self.create_mock_file("src/platform/ESP32/SystemTimeSupport.cpp"),
        ]
        # Only doru91 approved, esp32dev has not
        reviews = [
            self.create_mock_review("doru91", "APPROVED"),
        ]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)

        self.bot.check_and_process_pr(mock_pr)

        # Should NOT merge, should post eligibility indicating esp32 lacks approval
        mock_pr.merge.assert_not_called()
        self.assertIn(
            "- **esp32**: @esp32dev (❌ Needs approval)",
            mock_pr.create_issue_comment.call_args[0][0],
        )
        self.assertIn(
            "- **nxp**: @doru91, @nxpdev (✅ Approved)",
            mock_pr.create_issue_comment.call_args[0][0],
        )

    def test_check_and_process_pr_blocked_by_changes_requested(self) -> None:
        """Tests that any active changes requested review blocks auto-merge."""
        files = [
            self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp"),
        ]
        # doru91 approved, but someone else requested changes
        reviews = [
            self.create_mock_review("doru91", "APPROVED"),
            self.create_mock_review("other_user", "CHANGES_REQUESTED"),
        ]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)

        self.bot.check_and_process_pr(mock_pr)

        # Should NOT merge because of the active changes requested
        mock_pr.merge.assert_not_called()

    def test_get_pr_review_states_comment_does_not_overwrite_approval(self) -> None:
        """Tests that a standard comment review does not overwrite an existing approval review."""
        reviews = [
            self.create_mock_review("doru91", "APPROVED"),
            self.create_mock_review("doru91", "COMMENTED"),
        ]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", [], reviews)
        approvers, change_requesters = self.bot.get_pr_review_states(mock_pr)
        self.assertIn("doru91", approvers)

    def test_get_pr_review_states_comment_does_not_overwrite_changes_requested(
        self,
    ) -> None:
        """Tests that a standard comment review does not overwrite an existing changes requested review."""
        reviews = [
            self.create_mock_review("doru91", "CHANGES_REQUESTED"),
            self.create_mock_review("doru91", "COMMENTED"),
        ]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", [], reviews)
        approvers, change_requesters = self.bot.get_pr_review_states(mock_pr)
        self.assertIn("doru91", change_requesters)

    def test_check_and_process_pr_removes_comment_when_ineligible(self) -> None:
        """Tests that a stale eligibility comment is deleted if the PR is no longer eligible."""
        files = [
            self.create_mock_file(
                "src/app/Command.cpp"
            ),  # Uncovered file makes it ineligible
        ]
        mock_comment = MagicMock()
        mock_comment.user.login = "platform-merge-bot"
        mock_comment.body = f"{ELIGIBILITY_COMMENT_MARKER}\nSome status info..."
        mock_pr = self.create_mock_pr(
            1, "Test PR", "author", files, [], comments=[mock_comment]
        )

        self.bot.check_and_process_pr(mock_pr)

        mock_comment.delete.assert_called_once()
        mock_pr.merge.assert_not_called()

    def test_check_and_process_pr_removes_comment_when_no_changed_files(self) -> None:
        """Tests that a stale eligibility comment is deleted if the PR has no changed files."""
        files = []  # No changed files
        mock_comment = MagicMock()
        mock_comment.user.login = "platform-merge-bot"
        mock_comment.body = f"{ELIGIBILITY_COMMENT_MARKER}\nSome status info..."
        mock_pr = self.create_mock_pr(
            1, "Test PR", "author", files, [], comments=[mock_comment]
        )

        self.bot.check_and_process_pr(mock_pr)

        mock_comment.delete.assert_called_once()
        mock_pr.merge.assert_not_called()

    def test_check_and_process_pr_already_commented_deletes_duplicates(self) -> None:
        """Tests that duplicate eligibility comments are deleted and only one is kept/updated."""
        files = [
            self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp"),
        ]
        reviews = []

        # We have two eligibility comments
        mock_comment1 = MagicMock()
        mock_comment1.user.login = "platform-merge-bot"
        mock_comment1.body = f"{ELIGIBILITY_COMMENT_MARKER}\nOutdated Info..."

        mock_comment2 = MagicMock()
        mock_comment2.user.login = "platform-merge-bot"
        mock_comment2.body = f"{ELIGIBILITY_COMMENT_MARKER}\nDuplicate Info..."

        mock_pr = self.create_mock_pr(
            1,
            "Test PR",
            "author",
            files,
            reviews,
            comments=[mock_comment1, mock_comment2],
        )

        self.bot.check_and_process_pr(mock_pr)

        # The first comment should be edited (updated)
        mock_comment1.edit.assert_called_once()
        # The second comment should be deleted
        mock_comment2.delete.assert_called_once()
        mock_pr.merge.assert_not_called()

    def test_config_loading_invalid_format(self) -> None:
        """Tests that ValueError is raised for various invalid config formats."""
        invalid_contents = [
            # Not a dict
            "[]",
            # Group name not string (YAML allows int keys)
            "123:\n  maintainers:\n    - dev\n  paths:\n    - 'path/**'",
            # Group data not dict
            "group: 'not a dict'",
            # Unrecognized keys
            "group:\n  maintainers:\n    - dev\n  paths:\n    - 'path/**'\n  extra: 1",
            # Maintainers not list
            "group:\n  maintainers: dev\n  paths:\n    - 'path/**'",
            # Paths not list
            "group:\n  maintainers:\n    - dev\n  paths: 'path/**'",
            # Empty maintainers
            "group:\n  maintainers: []\n  paths:\n    - 'path/**'",
            # Empty paths
            "group:\n  maintainers:\n    - dev\n  paths: []",
            # Empty maintainer string
            "group:\n  maintainers:\n    - ''\n  paths:\n    - 'path/**'",
            # Empty path string
            "group:\n  maintainers:\n    - dev\n  paths:\n    - ' '",
        ]

        for content in invalid_contents:
            with tempfile.NamedTemporaryFile(
                delete=False, mode="w", suffix=".yaml"
            ) as f:
                f.write(content)
                temp_name = f.name
            try:
                with self.assertRaises(ValueError):
                    PlatformMergeBot(
                        token="dummy_token",
                        repo_name="dummy/repo",
                        config_path=temp_name,
                        dry_run=False,
                    )
            finally:
                os.unlink(temp_name)

    def test_check_and_process_pr_ci_pending_status_does_not_merge(self) -> None:
        """Tests that a PR is not merged when combined status is pending."""
        files = [self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp")]
        reviews = [self.create_mock_review("doru91", "APPROVED")]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)

        # Mock a pending CI status check
        mock_status = MagicMock()
        mock_status.context = "license/cla"
        mock_status.state = "pending"
        mock_status.description = "Checking CLA..."
        self.mock_commit.get_combined_status.return_value.statuses = [mock_status]

        self.bot.check_and_process_pr(mock_pr)

        mock_pr.merge.assert_not_called()
        # Should still post/update eligibility comment
        mock_pr.create_issue_comment.assert_called_once()

    def test_check_and_process_pr_ci_pullapprove_pending_still_merges(self) -> None:
        """Tests that pullapprove pending status does not block merge."""
        files = [self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp")]
        reviews = [self.create_mock_review("doru91", "APPROVED")]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)

        # Mock pending pullapprove status but passing CLA status
        status_cla = MagicMock()
        status_cla.context = "license/cla"
        status_cla.state = "success"
        status_cla.description = "CLA signed"

        status_pa = MagicMock()
        status_pa.context = "pullapprove"
        status_pa.state = "pending"
        status_pa.description = "Pending approvals"

        self.mock_commit.get_combined_status.return_value.statuses = [
            status_cla,
            status_pa,
        ]

        self.bot.check_and_process_pr(mock_pr)

        mock_pr.merge.assert_called_once()

    def test_check_and_process_pr_ci_failing_check_suite_does_not_merge(self) -> None:
        """Tests that a PR is not merged when check suite fails."""
        files = [self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp")]
        reviews = [self.create_mock_review("doru91", "APPROVED")]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)

        # Mock completed but failed check suite
        mock_suite = MagicMock()
        mock_suite.status = "completed"
        mock_suite.conclusion = "failure"
        mock_success_suite = MagicMock()
        mock_success_suite.status = "completed"
        mock_success_suite.conclusion = "success"
        self.mock_commit.get_check_suites.return_value = [mock_suite] + [mock_success_suite] * 9

        self.bot.check_and_process_pr(mock_pr)

        mock_pr.merge.assert_not_called()
        mock_pr.create_issue_comment.assert_called_once()

    def test_check_and_process_pr_ci_incomplete_check_suite_does_not_merge(self) -> None:
        """Tests that a PR is not merged when check suite is still running."""
        files = [self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp")]
        reviews = [self.create_mock_review("doru91", "APPROVED")]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)

        # Mock in_progress check suite
        mock_suite = MagicMock()
        mock_suite.status = "in_progress"
        mock_suite.conclusion = None
        mock_success_suite = MagicMock()
        mock_success_suite.status = "completed"
        mock_success_suite.conclusion = "success"
        self.mock_commit.get_check_suites.return_value = [mock_suite] + [mock_success_suite] * 9

        self.bot.check_and_process_pr(mock_pr)

        mock_pr.merge.assert_not_called()
        mock_pr.create_issue_comment.assert_called_once()

    def test_check_and_process_pr_ci_passing_non_blocking_check_suite_merges(self) -> None:
        """Tests that neutral/skipped/success check suites allow merge."""
        files = [self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp")]
        reviews = [self.create_mock_review("doru91", "APPROVED")]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)

        # Mock successful, neutral, and skipped check suites
        mock_suite1 = MagicMock()
        mock_suite1.status = "completed"
        mock_suite1.conclusion = "success"

        mock_suite2 = MagicMock()
        mock_suite2.status = "completed"
        mock_suite2.conclusion = "neutral"

        mock_suite3 = MagicMock()
        mock_suite3.status = "completed"
        mock_suite3.conclusion = "skipped"

        self.mock_commit.get_check_suites.return_value = [
            mock_suite1,
            mock_suite2,
            mock_suite3,
        ] + [mock_suite1] * 7

        self.bot.check_and_process_pr(mock_pr)

        mock_pr.merge.assert_called_once()

    def test_check_and_process_pr_pullapprove_green_skips(self) -> None:
        """Tests that a PR is skipped (and comments removed) if pullapprove is green."""
        files = [self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp")]
        reviews = [self.create_mock_review("doru91", "APPROVED")]
        mock_comment = MagicMock()
        mock_comment.user.login = "platform-merge-bot"
        mock_comment.body = "<!-- platform-merge-bot-eligibility-marker -->"
        mock_pr = self.create_mock_pr(
            1, "Test PR", "author", files, reviews, comments=[mock_comment]
        )

        # Mock pullapprove green
        self.bot._is_pullapprove_green.return_value = True

        self.bot.check_and_process_pr(mock_pr)

        mock_pr.merge.assert_not_called()
        # Should delete the eligibility comment
        mock_comment.delete.assert_called_once()
        mock_pr.create_issue_comment.assert_not_called()

    def test_check_and_process_pr_unresolved_comments_does_not_merge(self) -> None:
        """Tests that unresolved review threads block merge and print comment status."""
        files = [self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp")]
        reviews = [self.create_mock_review("doru91", "APPROVED")]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)

        # Mock an unresolved thread
        self.bot._get_unresolved_threads.return_value = [
            {
                "author": "jmartinez-silabs",
                "body_preview": "Maybe we could check...",
                "url": "https://github.com/project-chip/connectedhomeip/pull/1#discussion_r1",
            }
        ]

        self.bot.check_and_process_pr(mock_pr)

        mock_pr.merge.assert_not_called()
        mock_pr.create_issue_comment.assert_called_once()
        comment_body = mock_pr.create_issue_comment.call_args[0][0]
        self.assertIn("Has unresolved review conversations", comment_body)
        self.assertIn("jmartinez-silabs", comment_body)
        self.assertIn("Maybe we could check...", comment_body)

    def test_check_and_process_pr_closed_skips(self) -> None:
        """Tests that a closed PR is skipped."""
        files = [self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp")]
        reviews = [self.create_mock_review("doru91", "APPROVED")]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)
        mock_pr.state = "closed"

        self.bot.check_and_process_pr(mock_pr)

        mock_pr.merge.assert_not_called()
        mock_pr.create_issue_comment.assert_not_called()

    def test_check_and_process_pr_single_pr_dry_run_bypasses_closed(self) -> None:
        """Tests that a closed PR is processed when single_pr_mode and dry_run are active."""
        files = [self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp")]
        reviews = [self.create_mock_review("doru91", "APPROVED")]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)
        mock_pr.state = "closed"

        # Enable single_pr_mode and dry_run
        self.bot.single_pr_mode = True
        self.bot.dry_run = True

        self.bot.check_and_process_pr(mock_pr)

        # Should log and say "Would merge" in dry-run (which means checking passed and didn't return early)
        # Note: merge is not called in dry-run, but mock_pr.merge should not be called either.
        # We can assert that analyze_pr_files was called, proving it bypassed the state check.
        mock_pr.get_files.assert_called_once()

    def test_check_and_process_pr_skip_checks_comments(self) -> None:
        """Tests that COMMENTS skip check skips unresolved comment checking."""
        files = [self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp")]
        reviews = [self.create_mock_review("doru91", "APPROVED")]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)

        # Configure skip check
        from platform_merge_bot import ValidationCheck

        self.bot.skip_checks = {ValidationCheck.COMMENTS}

        # Mock unresolved threads (should be ignored)
        self.bot._get_unresolved_threads.return_value = [
            {
                "author": "jmartinez-silabs",
                "body_preview": "unresolved",
                "url": "https://github.com/project-chip/connectedhomeip/pull/1#discussion_r1",
            }
        ]

        self.bot.check_and_process_pr(mock_pr)

        # Should merge because comments check was skipped
        mock_pr.merge.assert_called_once()

    def test_check_and_process_pr_skip_checks_ci(self) -> None:
        """Tests that CI skip check bypasses failing combined status."""
        files = [self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp")]
        reviews = [self.create_mock_review("doru91", "APPROVED")]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)

        # Configure skip check
        from platform_merge_bot import ValidationCheck

        self.bot.skip_checks = {ValidationCheck.CI}

        # Mock pending status (should be ignored)
        mock_status = MagicMock()
        mock_status.context = "license/cla"
        mock_status.state = "pending"
        self.mock_commit.get_combined_status.return_value.statuses = [
            mock_status
        ]

        self.bot.check_and_process_pr(mock_pr)

        # Should merge because CI check was skipped
        mock_pr.merge.assert_called_once()

    def test_get_pr_review_states_disallows_self_approval(self) -> None:
        """Tests that a PR author cannot approve their own PR."""
        reviews = [
            self.create_mock_review("doru91", "APPROVED"),
        ]
        # PR is authored by "doru91"
        mock_pr = self.create_mock_pr(1, "Test PR", "doru91", [], reviews)
        approvers, change_requesters = self.bot.get_pr_review_states(mock_pr)

        self.assertNotIn("doru91", approvers)

    def test_check_and_process_pr_deleted_author_skips(self) -> None:
        """Tests that a PR from a deleted/ghost author is skipped."""
        files = [self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp")]
        reviews = [self.create_mock_review("doru91", "APPROVED")]
        mock_pr = self.create_mock_pr(1, "Test PR", "ghost", files, reviews)
        # Simulate deleted user account where pr.user is None
        mock_pr.user = None

        self.bot.check_and_process_pr(mock_pr)

        mock_pr.merge.assert_not_called()
        mock_pr.create_issue_comment.assert_not_called()

    @patch("urllib.request.urlopen")
    def test_get_unresolved_threads_pagination_gating(self, mock_urlopen: MagicMock) -> None:
        """Tests that _get_unresolved_threads appends a system blocker when hasNextPage is True."""
        mock_response = MagicMock()
        mock_response.read.return_value = json.dumps({
            "data": {
                "repository": {
                    "pullRequest": {
                        "reviewThreads": {
                            "pageInfo": {
                                "hasNextPage": True
                            },
                            "nodes": []
                        }
                    }
                }
            }
        }).encode("utf-8")
        mock_urlopen.return_value.__enter__.return_value = mock_response

        mock_pr = self.create_mock_pr(1, "Test PR", "author", [], [])
        unresolved = PlatformMergeBot._get_unresolved_threads(self.bot, mock_pr)

        self.assertEqual(len(unresolved), 1)
        self.assertEqual(unresolved[0]["author"], "system")
        self.assertIn("Too many review threads", unresolved[0]["body_preview"])

    @patch("urllib.request.urlopen")
    def test_get_unresolved_threads_malformed_graphql_response(self, mock_urlopen: MagicMock) -> None:
        """Tests that _get_unresolved_threads raises RuntimeError if the JSON payload is missing key data structures."""
        mock_response = MagicMock()
        mock_response.read.return_value = json.dumps({
            "data": {
                "repository": {}
            }
        }).encode("utf-8")
        mock_urlopen.return_value.__enter__.return_value = mock_response

        mock_pr = self.create_mock_pr(1, "Test PR", "author", [], [])
        with self.assertRaises(RuntimeError) as ctx:
            PlatformMergeBot._get_unresolved_threads(self.bot, mock_pr)

        self.assertIn("missing PR repository/pullRequest data", str(ctx.exception))

    def test_check_and_process_pr_ci_insufficient_checks_does_not_merge(self) -> None:
        """Tests that a commit with fewer than 10 CI checks is treated as pending and does not merge."""
        files = [self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp")]
        reviews = [self.create_mock_review("doru91", "APPROVED")]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)

        # Mock only 2 check suites (fewer than the required 10)
        mock_suite = MagicMock()
        mock_suite.status = "completed"
        mock_suite.conclusion = "success"
        self.mock_commit.get_check_suites.return_value = [mock_suite] * 2
        self.mock_commit.get_combined_status.return_value.statuses = []

        self.bot.check_and_process_pr(mock_pr)

        mock_pr.merge.assert_not_called()
        mock_pr.create_issue_comment.assert_called_once()
        comment_body = mock_pr.create_issue_comment.call_args[0][0]
        self.assertIn("CI checks are pending or failed", comment_body)

    def test_check_and_process_pr_merge_conflicts_does_not_merge(self) -> None:
        """Tests that a PR with merge conflicts is blocked from auto-merging."""
        files = [self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp")]
        reviews = [self.create_mock_review("doru91", "APPROVED")]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)
        mock_pr.mergeable = False

        self.bot.check_and_process_pr(mock_pr)

        mock_pr.merge.assert_not_called()
        mock_pr.create_issue_comment.assert_called_once()
        comment_body = mock_pr.create_issue_comment.call_args[0][0]
        self.assertIn("PR has merge conflicts", comment_body)

    def test_check_and_process_pr_merge_conflicts_computing(self) -> None:
        """Tests that a PR whose mergeability state is still computing is blocked."""
        files = [self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp")]
        reviews = [self.create_mock_review("doru91", "APPROVED")]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)
        mock_pr.mergeable = None

        self.bot.check_and_process_pr(mock_pr)

        mock_pr.merge.assert_not_called()
        mock_pr.create_issue_comment.assert_called_once()
        comment_body = mock_pr.create_issue_comment.call_args[0][0]
        self.assertIn("Mergeability state is computing", comment_body)


if __name__ == "__main__":
    unittest.main()
