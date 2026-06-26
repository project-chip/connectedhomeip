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

import os
# Ensure scripts/tools/ is in path so we can import platform_merge_bot
import sys
import tempfile
import unittest
from unittest.mock import MagicMock, patch

import platform_merge_bot
from platform_merge_bot import PlatformMergeBot

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))


class TestPlatformMergeBot(unittest.TestCase):
    def setUp(self):
        # Create a temporary config file for testing
        self.temp_config = tempfile.NamedTemporaryFile(delete=False, mode='w', suffix='.yaml')
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
        self.temp_config.write(self.config_content)
        self.temp_config.close()

        # Patch Github initialization so we don't hit the network
        self.github_patcher = patch('platform_merge_bot.Github')
        self.mock_github_class = self.github_patcher.start()
        self.mock_github_instance = MagicMock()
        self.mock_github_class.return_value = self.mock_github_instance
        self.mock_repo = MagicMock()
        self.mock_github_instance.get_repo.return_value = self.mock_repo

        # Initialize bot under test
        self.bot = PlatformMergeBot(
            token="dummy_token",
            repo_name="dummy/repo",
            config_path=self.temp_config.name,
            dry_run=False
        )

    def tearDown(self):
        self.github_patcher.stop()
        os.unlink(self.temp_config.name)

    def create_mock_file(self, filename: str) -> MagicMock:
        mock_file = MagicMock()
        mock_file.filename = filename
        return mock_file

    def create_mock_review(self, username: str, state: str) -> MagicMock:
        mock_review = MagicMock()
        mock_review.user.login = username
        mock_review.state = state
        return mock_review

    def create_mock_pr(self, number: int, title: str, author: str, files: list, reviews: list, comments: list = None, draft: bool = False) -> MagicMock:
        mock_pr = MagicMock()
        mock_pr.number = number
        mock_pr.title = title
        mock_pr.user.login = author
        mock_pr.draft = draft
        mock_pr.get_files.return_value = files
        mock_pr.get_reviews.return_value = reviews
        mock_pr.get_issue_comments.return_value = comments or []
        return mock_pr

    def test_config_loading(self):
        self.assertEqual(len(self.bot.groups), 2)
        self.assertIn("nxp", self.bot.groups)
        self.assertIn("esp32", self.bot.groups)

        nxp_group = self.bot.groups["nxp"]
        self.assertEqual(nxp_group.maintainers, ["doru91", "nxpdev"])
        self.assertEqual(nxp_group.paths, ["src/platform/nxp/**", "examples/**/nxp/**"])

    def test_file_matching(self):
        nxp_group = self.bot.groups["nxp"]

        self.assertTrue(nxp_group.matches_file("src/platform/nxp/SystemTimeSupport.cpp"))
        self.assertTrue(nxp_group.matches_file("src/platform/nxp/rt/rt1060/SystemTimeSupport.cpp"))
        self.assertTrue(nxp_group.matches_file("examples/all-clusters-app/nxp/main.cpp"))

        self.assertFalse(nxp_group.matches_file("src/platform/ESP32/SystemTimeSupport.cpp"))
        self.assertFalse(nxp_group.matches_file("src/app/Command.cpp"))

    def test_get_pr_approvers(self):
        reviews = [
            self.create_mock_review("doru91", "COMMENTED"),
            self.create_mock_review("nxpdev", "APPROVED"),
            self.create_mock_review("doru91", "CHANGES_REQUESTED"),
            self.create_mock_review("doru91", "APPROVED"),  # approved later
            self.create_mock_review("nxpdev", "CHANGES_REQUESTED"),  # requested changes later
        ]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", [], reviews)
        approvers = self.bot.get_pr_approvers(mock_pr)

        # doru91 should be in approvers (latest was approved)
        # nxpdev should NOT be in approvers (latest was changes requested)
        self.assertIn("doru91", approvers)
        self.assertNotIn("nxpdev", approvers)

        # test dismissed
        reviews2 = [
            self.create_mock_review("doru91", "APPROVED"),
            self.create_mock_review("doru91", "DISMISSED"),
        ]
        mock_pr2 = self.create_mock_pr(2, "Test PR 2", "author", [], reviews2)
        approvers2 = self.bot.get_pr_approvers(mock_pr2)
        self.assertNotIn("doru91", approvers2)

    def test_analyze_pr_files_fully_covered(self):
        files = [
            self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp"),
            self.create_mock_file("examples/all-clusters-app/nxp/main.cpp"),
        ]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, [])
        is_fully_covered, matched_files, uncovered = self.bot.analyze_pr_files(mock_pr)

        self.assertTrue(is_fully_covered)
        self.assertEqual(len(uncovered), 0)
        self.assertEqual(len(matched_files["nxp"]), 2)
        self.assertEqual(len(matched_files["esp32"]), 0)

    def test_analyze_pr_files_not_fully_covered(self):
        files = [
            self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp"),
            self.create_mock_file("src/app/Command.cpp"),  # Uncovered
        ]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, [])
        is_fully_covered, matched_files, uncovered = self.bot.analyze_pr_files(mock_pr)

        self.assertFalse(is_fully_covered)
        self.assertEqual(uncovered, ["src/app/Command.cpp"])
        self.assertEqual(len(matched_files["nxp"]), 1)

    def test_check_and_process_pr_draft(self):
        mock_pr = self.create_mock_pr(1, "Test PR", "author", [], [], draft=True)
        self.bot.check_and_process_pr(mock_pr)

        # Verify no files were fetched
        mock_pr.get_files.assert_not_called()

    def test_check_and_process_pr_not_eligible(self):
        files = [
            self.create_mock_file("src/app/Command.cpp"),
        ]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, [])
        self.bot.check_and_process_pr(mock_pr)

        # Should not merge
        mock_pr.merge.assert_not_called()
        mock_pr.create_issue_comment.assert_not_called()

    def test_check_and_process_pr_needs_approval(self):
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
        self.assertIn(platform_merge_bot.ELIGIBILITY_COMMENT_MARKER, mock_pr.create_issue_comment.call_args[0][0])
        self.assertIn("Needs approval", mock_pr.create_issue_comment.call_args[0][0])

    def test_check_and_process_pr_already_commented_no_duplicate(self):
        files = [
            self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp"),
        ]
        reviews = []
        # Existing eligibility comment
        mock_comment = MagicMock()
        mock_comment.body = f"{platform_merge_bot.ELIGIBILITY_COMMENT_MARKER}\nInfo..."
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews, comments=[mock_comment])

        self.bot.check_and_process_pr(mock_pr)

        # Should not merge, and should NOT create another comment
        mock_pr.merge.assert_not_called()
        mock_pr.create_issue_comment.assert_not_called()

    def test_check_and_process_pr_fully_approved_merge(self):
        files = [
            self.create_mock_file("src/platform/nxp/SystemTimeSupport.cpp"),
        ]
        reviews = [
            self.create_mock_review("doru91", "APPROVED"),
        ]
        mock_pr = self.create_mock_pr(1, "Test PR", "author", files, reviews)

        self.bot.run_command_mock = MagicMock()  # just in case
        self.bot.check_and_process_pr(mock_pr)

        # Should merge and comment
        self.assertEqual(mock_pr.create_issue_comment.call_count, 1)
        self.assertIn("Platform Maintainers Auto-Merge Executed", mock_pr.create_issue_comment.call_args[0][0])
        self.assertIn("approved by @doru91", mock_pr.create_issue_comment.call_args[0][0])

        mock_pr.merge.assert_called_once_with(merge_method="squash", commit_title="Test PR (Auto-merged by platform-bot)")

    def test_check_and_process_pr_multi_group_approved(self):
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
        self.assertIn("approved by @doru91", mock_pr.create_issue_comment.call_args[0][0])
        self.assertIn("approved by @esp32dev", mock_pr.create_issue_comment.call_args[0][0])

    def test_check_and_process_pr_multi_group_missing_one_approval(self):
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
        self.assertIn("- **esp32**: @esp32dev (❌ Needs approval)", mock_pr.create_issue_comment.call_args[0][0])
        self.assertIn("- **nxp**: @doru91, @nxpdev (✅ Approved)", mock_pr.create_issue_comment.call_args[0][0])


if __name__ == '__main__':
    unittest.main()
