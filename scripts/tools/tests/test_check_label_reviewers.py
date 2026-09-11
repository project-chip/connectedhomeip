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
from check_label_reviewers import LabelEvaluation  # noqa: E402
from check_label_reviewers import (LabelRule, check_override_present, evaluate_pr_labels, extract_approvers, find_active_override,
                                   generate_step_summary, is_sme_review_satisfied, parse_label_config)


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

    def test_valid_config_quoted_and_unquoted_usernames(self) -> None:
        yaml_content = """
security:
  - alice
  - 'bob'
  - "charlie"
"""
        config_path = self._write_temp_config(yaml_content)
        mapping = parse_label_config(config_path)

        self.assertEqual(mapping["security"].smes, ["alice", "bob", "charlie"])

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


class TestCheckOverridePresent(unittest.TestCase):
    """Tests checking for the presence of override labels."""

    def test_default_override_label_exact_match(self) -> None:
        pr_labels = ["bug", "no-sme-check-required", "security"]
        self.assertTrue(check_override_present(pr_labels))

    def test_sdk_maintainer_approved_exact_match(self) -> None:
        pr_labels = ["bug", "sdk-maintainer-approved", "security"]
        self.assertTrue(check_override_present(pr_labels))

    def test_override_label_case_insensitive(self) -> None:
        pr_labels = ["No-SME-Check-Required"]
        self.assertTrue(check_override_present(pr_labels))
        pr_labels_maintainer = ["SDK-Maintainer-Approved"]
        self.assertTrue(check_override_present(pr_labels_maintainer))

    def test_override_label_with_surrounding_whitespace(self) -> None:
        pr_labels = ["  no-sme-check-required  "]
        self.assertTrue(check_override_present(pr_labels))
        pr_labels_maintainer = ["  sdk-maintainer-approved  "]
        self.assertTrue(check_override_present(pr_labels_maintainer))

    def test_override_label_not_present(self) -> None:
        pr_labels = ["bug", "security", "enhancement"]
        self.assertFalse(check_override_present(pr_labels))

    def test_override_label_empty_labels(self) -> None:
        self.assertFalse(check_override_present([]))

    def test_custom_override_label(self) -> None:
        pr_labels = ["exempt-from-sme"]
        self.assertTrue(check_override_present(pr_labels, override_labels="exempt-from-sme"))
        self.assertTrue(check_override_present(pr_labels, override_labels=["exempt-from-sme", "other"]))
        self.assertFalse(check_override_present(pr_labels, override_labels="other-label"))

    def test_find_active_override(self) -> None:
        self.assertIsNone(find_active_override(["bug", "feature"]))
        self.assertEqual(
            find_active_override(["bug", "sdk-maintainer-approved"]),
            "sdk-maintainer-approved",
        )
        self.assertEqual(
            find_active_override(["SDK-Maintainer-Approved"]),
            "SDK-Maintainer-Approved",
        )
        self.assertEqual(
            find_active_override(["no-sme-check-required", "sdk-maintainer-approved"]),
            "no-sme-check-required",
        )


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
        self.assertTrue(is_sme_review_satisfied(evaluations))

    def test_multi_label_one_approval_satisfies_overall_check(self) -> None:
        pr_labels = ["Security", "data-model"]
        approvers = {"alice"}  # Security approved, data-model unreviewed
        evaluations = evaluate_pr_labels(pr_labels, self.config, approvers)

        self.assertEqual(len(evaluations), 2)
        eval_dict = {ev.rule.name: ev.satisfied for ev in evaluations}
        self.assertTrue(eval_dict["Security"])
        self.assertFalse(eval_dict["Data-Model"])
        # Overall check passes because at least one SME from ANY label approved
        self.assertTrue(is_sme_review_satisfied(evaluations))

    def test_multi_label_no_sme_approvals_fails_overall_check(self) -> None:
        pr_labels = ["Security", "data-model"]
        approvers = {"external_reviewer"}
        evaluations = evaluate_pr_labels(pr_labels, self.config, approvers)

        self.assertEqual(len(evaluations), 2)
        self.assertFalse(is_sme_review_satisfied(evaluations))

    def test_case_insensitive_label_and_username_matching(self) -> None:
        pr_labels = ["sEcUrItY"]
        approvers = {"ALICE".lower()}
        evaluations = evaluate_pr_labels(pr_labels, self.config, approvers)

        self.assertEqual(len(evaluations), 1)
        self.assertTrue(evaluations[0].satisfied)
        self.assertEqual(evaluations[0].approvers, ["alice"])

    def test_overridden_satisfies_check_even_without_approvals(self) -> None:
        rule = LabelRule(name="Security", smes=["alice"])
        ev = LabelEvaluation(rule=rule, present_on_pr=True, approvers=[])
        self.assertFalse(is_sme_review_satisfied([ev]))
        self.assertTrue(is_sme_review_satisfied([ev], overridden=True))

    def test_overridden_satisfies_check_with_no_evaluations(self) -> None:
        self.assertTrue(is_sme_review_satisfied([], overridden=True))


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
        self.assertIn("SME Review Requirement Met!", summary)

    def test_summary_multi_label_partially_approved(self) -> None:
        rule1 = LabelRule(name="Security", smes=["alice"])
        rule2 = LabelRule(name="Data-Model", smes=["bob"])
        evals = [
            LabelEvaluation(rule=rule1, present_on_pr=True, approvers=["alice"]),
            LabelEvaluation(rule=rule2, present_on_pr=True, approvers=[]),
        ]
        summary = generate_step_summary(evals, 100, "Security patch", "author1", True)

        self.assertIn("| `Security` | ✅ Approved | @alice | @alice |", summary)
        self.assertIn("| `Data-Model` | ⚪ Satisfied (by other label) | @bob | *None* |", summary)
        self.assertIn("SME Review Requirement Met!", summary)

    def test_summary_missing_approval_label(self) -> None:
        rule = LabelRule(name="Security", smes=["alice", "bob"])
        evals = [LabelEvaluation(rule=rule, present_on_pr=True, approvers=[])]
        summary = generate_step_summary(evals, 100, "Security patch", "author1", False)

        self.assertIn("| `Security` | ❌ Missing | @alice, @bob | *None* |", summary)
        self.assertIn("Review Required", summary)

    def test_summary_overridden_with_monitored_labels(self) -> None:
        rule = LabelRule(name="Security", smes=["alice"])
        evals = [LabelEvaluation(rule=rule, present_on_pr=True, approvers=[])]
        summary = generate_step_summary(
            evals, 100, "Security patch", "author1", True, overridden=True, override_label="no-sme-check-required"
        )
        self.assertIn("SME Review Requirement Bypassed", summary)
        self.assertIn("no-sme-check-required", summary)
        self.assertIn("| `Security` | ⚪ Overridden (`no-sme-check-required`) | @alice | *None* |", summary)

    def test_summary_overridden_no_monitored_labels(self) -> None:
        summary = generate_step_summary(
            [], 100, "No labels PR", "author1", True, overridden=True, override_label="no-sme-check-required"
        )
        self.assertIn("SME Review Requirement Bypassed", summary)
        self.assertIn("no-sme-check-required", summary)


class TestEndToEndJsonEvaluation(unittest.TestCase):
    """Tests end-to-end evaluation using simulated GitHub CLI JSON output."""

    def test_full_evaluation_flow_with_one_matching_sme(self) -> None:
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
        # "security" is approved by sme_alice; under ANY-reviewer logic, check passes!
        self.assertTrue(is_sme_review_satisfied(evaluations))

    def test_full_evaluation_flow_missing_all_smes(self) -> None:
        mock_gh_json = {
            "title": "Add secure channel encryption",
            "author": {"login": "contributor"},
            "state": "OPEN",
            "labels": [
                {"name": "security"},
                {"name": "core"},
            ],
            "latestReviews": [
                {"author": {"login": "contributor"}, "state": "APPROVED"},  # self-approval
                {"author": {"login": "peer_reviewer"}, "state": "APPROVED"},  # not an SME
            ],
        }
        config = {
            "security": LabelRule(name="security", smes=["sme_alice", "sme_bob"]),
            "core": LabelRule(name="core", smes=["lead_dev"]),
        }
        approvers = extract_approvers(mock_gh_json)
        pr_labels = [l["name"] for l in mock_gh_json.get("labels", [])]
        evaluations = evaluate_pr_labels(pr_labels, config, approvers)

        self.assertFalse(is_sme_review_satisfied(evaluations))

    def test_full_evaluation_flow_with_override_label(self) -> None:
        mock_gh_json = {
            "title": "Emergency hotfix for crypto issue",
            "author": {"login": "contributor"},
            "state": "OPEN",
            "labels": [
                {"name": "security"},
                {"name": "no-sme-check-required"},
            ],
            "latestReviews": [],
        }
        config = {
            "security": LabelRule(name="security", smes=["sme_alice", "sme_bob"]),
        }
        approvers = extract_approvers(mock_gh_json)
        pr_labels = [l["name"] for l in mock_gh_json.get("labels", [])]
        overridden = check_override_present(pr_labels)
        self.assertTrue(overridden)

        evaluations = evaluate_pr_labels(pr_labels, config, approvers)
        self.assertTrue(is_sme_review_satisfied(evaluations, overridden=overridden))

    def test_full_evaluation_flow_with_sdk_maintainer_approved(self) -> None:
        mock_gh_json = {
            "title": "Core maintenance refactor approved by SDK maintainer",
            "author": {"login": "contributor"},
            "state": "OPEN",
            "labels": [
                {"name": "core"},
                {"name": "sdk-maintainer-approved"},
            ],
            "latestReviews": [],
        }
        config = {
            "core": LabelRule(name="core", smes=["lead_dev"]),
        }
        approvers = extract_approvers(mock_gh_json)
        pr_labels = [l["name"] for l in mock_gh_json.get("labels", [])]
        active_override = find_active_override(pr_labels)
        self.assertEqual(active_override, "sdk-maintainer-approved")

        evaluations = evaluate_pr_labels(pr_labels, config, approvers)
        self.assertTrue(is_sme_review_satisfied(evaluations, overridden=bool(active_override)))


if __name__ == "__main__":
    unittest.main()
