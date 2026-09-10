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
import sys
import tempfile
import unittest

# Ensure the parent directory is in the path so we can import check_label_reviewers
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

# isort: split

# pylint: disable=wrong-import-position
from check_label_reviewers import (  # noqa: E402
    LabelEvaluation,
    LabelRule,
    evaluate_pr_labels,
    extract_approvers,
    generate_step_summary,
    parse_label_config,
)


class TestParseLabelConfig(unittest.TestCase):
    """Tests parsing and validation of .github/label_reviewers.yaml."""

    def _write_temp_config(self, content: str) -> str:
        temp = tempfile.NamedTemporaryFile(delete=False, mode="w", suffix=".yaml", encoding="utf-8")
        temp.write(content)
        temp.close()
        self.addCleanup(os.unlink, temp.name)
        return temp.name

    def test_valid_config_single_label(self) -> None:
        yaml_content = """
security:
  - alice
  - bob
"""
        config_path = self._write_temp_config(yaml_content)
        mapping = parse_label_config(config_path)

        self.assertIn("security", mapping)
        self.assertEqual(mapping["security"].name, "security")
        self.assertEqual(mapping["security"].smes, ["alice", "bob"])

    def test_valid_config_multiple_labels(self) -> None:
        yaml_content = """
Security:
  - alice
Data-Model:
  - bzbarsky-apple
  - Boris-Virk
"""
        config_path = self._write_temp_config(yaml_content)
        mapping = parse_label_config(config_path)

        self.assertIn("security", mapping)
        self.assertEqual(mapping["security"].name, "Security")
        self.assertEqual(mapping["security"].smes, ["alice"])

        self.assertIn("data-model", mapping)
        self.assertEqual(mapping["data-model"].name, "Data-Model")
        self.assertEqual(mapping["data-model"].smes, ["bzbarsky-apple", "Boris-Virk"])

    def test_empty_config(self) -> None:
        yaml_content = "# Only comments in this file\n"
        config_path = self._write_temp_config(yaml_content)
        mapping = parse_label_config(config_path)
        self.assertEqual(mapping, {})

    def test_missing_config_file(self) -> None:
        with self.assertRaises(FileNotFoundError):
            parse_label_config("/path/does/not/exist.yaml")

    def test_invalid_yaml_structure_not_dict(self) -> None:
        yaml_content = """
- item1
- item2
"""
        config_path = self._write_temp_config(yaml_content)
        with self.assertRaises(ValueError) as ctx:
            parse_label_config(config_path)
        self.assertIn("expected a YAML mapping", str(ctx.exception))

    def test_invalid_label_value_not_list(self) -> None:
        yaml_content = """
security: "alice"
"""
        config_path = self._write_temp_config(yaml_content)
        with self.assertRaises(ValueError) as ctx:
            parse_label_config(config_path)
        self.assertIn("expected a list of usernames", str(ctx.exception))

    def test_empty_reviewer_list(self) -> None:
        yaml_content = """
security: []
"""
        config_path = self._write_temp_config(yaml_content)
        with self.assertRaises(ValueError) as ctx:
            parse_label_config(config_path)
        self.assertIn("must have at least one reviewer listed", str(ctx.exception))

    def test_invalid_reviewer_entry_not_string(self) -> None:
        yaml_content = """
security:
  - 12345
"""
        config_path = self._write_temp_config(yaml_content)
        with self.assertRaises(ValueError) as ctx:
            parse_label_config(config_path)
        self.assertIn("expected a username string", str(ctx.exception))

    def test_reviewer_with_leading_at_symbol_rejected(self) -> None:
        yaml_content = """
security:
  - "@cecille"
"""
        config_path = self._write_temp_config(yaml_content)
        with self.assertRaises(ValueError) as ctx:
            parse_label_config(config_path)
        self.assertIn("do not include '@' in usernames", str(ctx.exception))

    def test_unquoted_at_symbol_raises_yaml_syntax_error(self) -> None:
        yaml_content = """
security:
  - @cecille
"""
        config_path = self._write_temp_config(yaml_content)
        with self.assertRaises(ValueError) as ctx:
            parse_label_config(config_path)
        self.assertIn("YAML syntax error", str(ctx.exception))


class TestExtractApprovers(unittest.TestCase):
    """Tests extracting valid approvers from GitHub PR JSON payloads."""

    def test_extract_approved_reviews(self) -> None:
        pr_data = {
            "author": {"login": "author_user"},
            "latestReviews": [
                {"author": {"login": "alice"}, "state": "APPROVED"},
                {"author": {"login": "bob"}, "state": "APPROVED"},
            ],
        }
        approvers = extract_approvers(pr_data)
        self.assertEqual(approvers, {"alice", "bob"})

    def test_ignores_non_approved_review_states(self) -> None:
        pr_data = {
            "author": {"login": "author_user"},
            "latestReviews": [
                {"author": {"login": "alice"}, "state": "APPROVED"},
                {"author": {"login": "charlie"}, "state": "CHANGES_REQUESTED"},
                {"author": {"login": "david"}, "state": "COMMENTED"},
                {"author": {"login": "eve"}, "state": "DISMISSED"},
            ],
        }
        approvers = extract_approvers(pr_data)
        self.assertEqual(approvers, {"alice"})

    def test_excludes_author_self_approval(self) -> None:
        pr_data = {
            "author": {"login": "author_user"},
            "latestReviews": [
                {"author": {"login": "author_user"}, "state": "APPROVED"},
                {"author": {"login": "reviewer1"}, "state": "APPROVED"},
            ],
        }
        approvers = extract_approvers(pr_data)
        self.assertNotIn("author_user", approvers)
        self.assertEqual(approvers, {"reviewer1"})

    def test_case_insensitive_logins(self) -> None:
        pr_data = {
            "author": {"login": "Author_User"},
            "latestReviews": [
                {"author": {"login": "AUTHOR_USER"}, "state": "APPROVED"},
                {"author": {"login": "Alice-Expert"}, "state": "APPROVED"},
            ],
        }
        approvers = extract_approvers(pr_data)
        self.assertNotIn("author_user", approvers)
        self.assertEqual(approvers, {"alice-expert"})

    def test_empty_reviews_and_missing_author(self) -> None:
        pr_data = {"latestReviews": []}
        self.assertEqual(extract_approvers(pr_data), set())

        empty_data: dict[str, object] = {}
        self.assertEqual(extract_approvers(empty_data), set())


class TestEvaluatePrLabels(unittest.TestCase):
    """Tests evaluating PR labels against configured rules and approvers."""

    def setUp(self) -> None:
        self.config = {
            "security": LabelRule(name="Security", smes=["alice", "bob"]),
            "data-model": LabelRule(name="Data-Model", smes=["charlie", "david"]),
        }

    def test_no_monitored_labels_on_pr(self) -> None:
        pr_labels = ["bug", "documentation", "enhancement"]
        approvers = {"anyone"}
        evaluations = evaluate_pr_labels(pr_labels, self.config, approvers)
        self.assertEqual(evaluations, [])

    def test_single_monitored_label_approved(self) -> None:
        pr_labels = ["Security", "bug"]
        approvers = {"alice"}
        evaluations = evaluate_pr_labels(pr_labels, self.config, approvers)

        self.assertEqual(len(evaluations), 1)
        ev = evaluations[0]
        self.assertEqual(ev.rule.name, "Security")
        self.assertTrue(ev.satisfied)
        self.assertEqual(ev.approvers, ["alice"])

    def test_single_monitored_label_not_approved(self) -> None:
        pr_labels = ["security"]
        approvers = {"someone_else"}
        evaluations = evaluate_pr_labels(pr_labels, self.config, approvers)

        self.assertEqual(len(evaluations), 1)
        ev = evaluations[0]
        self.assertEqual(ev.rule.name, "Security")
        self.assertFalse(ev.satisfied)
        self.assertEqual(ev.approvers, [])

    def test_multi_label_all_approved(self) -> None:
        pr_labels = ["Security", "data-model"]
        approvers = {"alice", "david"}
        evaluations = evaluate_pr_labels(pr_labels, self.config, approvers)

        self.assertEqual(len(evaluations), 2)
        self.assertTrue(all(ev.satisfied for ev in evaluations))

    def test_multi_label_partial_approval_fails(self) -> None:
        pr_labels = ["Security", "data-model"]
        approvers = {"alice"}  # Security approved, data-model missing
        evaluations = evaluate_pr_labels(pr_labels, self.config, approvers)

        self.assertEqual(len(evaluations), 2)
        eval_dict = {ev.rule.name: ev.satisfied for ev in evaluations}
        self.assertTrue(eval_dict["Security"])
        self.assertFalse(eval_dict["Data-Model"])

    def test_case_insensitive_label_and_username_matching(self) -> None:
        pr_labels = ["sEcUrItY"]
        approvers = {"ALICE".lower()}
        evaluations = evaluate_pr_labels(pr_labels, self.config, approvers)

        self.assertEqual(len(evaluations), 1)
        self.assertTrue(evaluations[0].satisfied)
        self.assertEqual(evaluations[0].approvers, ["alice"])


class TestGenerateStepSummary(unittest.TestCase):
    """Tests generating GitHub Actions Markdown step summaries."""

    def test_summary_no_monitored_labels(self) -> None:
        summary = generate_step_summary([], 100, "Fix typo", "author1", True)
        self.assertIn("No monitored SME review labels attached", summary)
        self.assertIn("Fix typo", summary)

    def test_summary_approved_label(self) -> None:
        rule = LabelRule(name="Security", smes=["alice", "bob"])
        evals = [LabelEvaluation(rule=rule, present_on_pr=True, approvers=["alice"])]
        summary = generate_step_summary(evals, 100, "Security patch", "author1", True)

        self.assertIn("| `Security` | ✅ Approved | @alice, @bob | @alice |", summary)
        self.assertIn("All SME Review Requirements Met!", summary)

    def test_summary_missing_approval_label(self) -> None:
        rule = LabelRule(name="Security", smes=["alice", "bob"])
        evals = [LabelEvaluation(rule=rule, present_on_pr=True, approvers=[])]
        summary = generate_step_summary(evals, 100, "Security patch", "author1", False)

        self.assertIn("| `Security` | ❌ Missing | @alice, @bob | *None* |", summary)
        self.assertIn("Review Required", summary)


class TestEndToEndJsonEvaluation(unittest.TestCase):
    """Tests end-to-end evaluation using simulated GitHub CLI JSON output."""

    def test_full_evaluation_flow(self) -> None:
        # 1. Mock JSON output matching `gh pr view --json author,title,state,labels,latestReviews`
        mock_gh_json = {
            "title": "Add secure channel encryption",
            "author": {"login": "contributor"},
            "state": "OPEN",
            "labels": [
                {"name": "security"},
                {"name": "core"},
                {"name": "enhancement"},
            ],
            "latestReviews": [
                {"author": {"login": "sme_alice"}, "state": "APPROVED"},
                {"author": {"login": "contributor"}, "state": "APPROVED"},  # self-approval
                {"author": {"login": "peer_reviewer"}, "state": "APPROVED"},  # not an SME
            ],
        }

        # 2. Config mapping
        config = {
            "security": LabelRule(name="security", smes=["sme_alice", "sme_bob"]),
            "core": LabelRule(name="core", smes=["lead_dev"]),
        }

        # 3. Extract approvers from JSON
        approvers = extract_approvers(mock_gh_json)
        self.assertIn("sme_alice", approvers)
        self.assertIn("peer_reviewer", approvers)
        self.assertNotIn("contributor", approvers)  # author excluded

        # 4. Extract labels from JSON
        pr_labels = [l["name"] for l in mock_gh_json.get("labels", [])]

        # 5. Evaluate
        evaluations = evaluate_pr_labels(pr_labels, config, approvers)
        all_passed = all(ev.satisfied for ev in evaluations)

        # "security" is approved by sme_alice; "core" is missing approval from lead_dev
        self.assertFalse(all_passed)
        eval_status = {ev.rule.name: ev.satisfied for ev in evaluations}
        self.assertTrue(eval_status["security"])
        self.assertFalse(eval_status["core"])


if __name__ == "__main__":
    unittest.main()
