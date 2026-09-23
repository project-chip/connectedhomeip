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

"""SME (Subject Matter Expert) Label Reviewer Checker for Pull Requests.

Verifies that for PRs with designated labels attached, at least one designated
SME username from any of the matching labels' lists has approved the PR.
Uses the GitHub CLI (`gh`) to fetch PR reviews and labels.
"""

from __future__ import annotations

import argparse
import json
import logging
import os
import subprocess
import sys
from dataclasses import dataclass, field
from typing import Any

import yaml

DEFAULT_REPO = "project-chip/connectedhomeip"
DEFAULT_CONFIG_PATH = ".github/label_reviewers.yaml"
DEFAULT_OVERRIDE_LABELS = ("no-sme-check-required", "sdk-maintainer-approved")
DEFAULT_OVERRIDE_LABEL = DEFAULT_OVERRIDE_LABELS[0]

LOGGER = logging.getLogger(__name__)


class UniqueKeySafeLoader(yaml.SafeLoader):
    """YAML SafeLoader that raises ValueError on duplicate mapping keys."""


def _construct_mapping(
    loader: yaml.SafeLoader, node: yaml.MappingNode, deep: bool = False
) -> dict[Any, Any]:
    """Constructs a YAML mapping while rejecting duplicate keys."""
    mapping: dict[Any, Any] = {}
    for key_node, value_node in node.value:
        key = loader.construct_object(key_node, deep=deep)
        if key in mapping:
            raise ValueError(f"Duplicate YAML key detected: {key!r}")
        mapping[key] = loader.construct_object(value_node, deep=deep)
    return mapping


UniqueKeySafeLoader.add_constructor(
    yaml.resolver.BaseResolver.DEFAULT_MAPPING_TAG, _construct_mapping
)


@dataclass
class LabelRule:
    """Represents a configured label and its designated SME reviewers."""

    name: str
    smes: list[str]


@dataclass
class LabelEvaluation:
    """Represents the evaluation status of a single label on a pull request."""

    rule: LabelRule
    present_on_pr: bool
    approvers: list[str] = field(default_factory=list)

    @property
    def satisfied(self) -> bool:
        """Returns True if the label is either not on the PR or has at least one SME approver."""
        return not self.present_on_pr or len(self.approvers) > 0


def fetch_pull_request_data(repo: str, pr_number: int) -> dict[str, Any]:
    """Fetches pull request details and reviews using the gh CLI."""
    cmd = [
        "gh",
        "pr",
        "view",
        str(pr_number),
        "--repo",
        repo,
        "--json",
        "author,title,state,labels,reviews",
    ]
    try:
        proc = subprocess.run(cmd, capture_output=True, text=True, check=True)
        return json.loads(proc.stdout)
    except subprocess.CalledProcessError as e:
        raise RuntimeError(
            f"gh pr view failed (exit code {e.returncode}): {e.stderr.strip()}"
        ) from e
    except json.JSONDecodeError as e:
        raise RuntimeError(f"Failed to parse gh output as JSON: {e}") from e


def parse_label_config(config_path: str) -> dict[str, LabelRule]:
    """Parses the YAML configuration file mapping labels to SME reviewers.

    Expected format is strictly a mapping of label names to lists of usernames:
      label_name:
        - username1
        - username2
    """
    if not os.path.exists(config_path):
        raise FileNotFoundError(f"Configuration file not found: {config_path}")

    try:
        with open(config_path, encoding="utf-8") as f:
            content = yaml.load(f, Loader=UniqueKeySafeLoader)
            if content is None:
                content = {}
    except yaml.YAMLError as e:
        raise ValueError(f"YAML syntax error in {config_path}: {e}") from e

    if not isinstance(content, dict):
        raise ValueError(
            f"Invalid format in {config_path}: expected a YAML mapping of label names to reviewer lists."
        )

    mapping: dict[str, LabelRule] = {}
    for label_raw, val in content.items():
        if not isinstance(label_raw, str) or not label_raw.strip():
            raise ValueError(
                f"Invalid label key {label_raw!r} in {config_path}: expected a non-empty string."
            )
        label_name = label_raw.strip()
        label_key = label_name.lower()

        if label_key in mapping:
            raise ValueError(
                f"Duplicate normalized label key '{label_name}' in {config_path} "
                f"(conflicts with '{mapping[label_key].name}')."
            )

        if not isinstance(val, list):
            raise ValueError(
                f"Invalid format for label '{label_name}' in {config_path}: "
                f"expected a list of usernames, got {type(val).__name__}."
            )

        smes: list[str] = []
        for u in val:
            if not isinstance(u, str) or not u.strip():
                raise ValueError(
                    f"Invalid reviewer entry under label '{label_name}' in {config_path}: "
                    f"expected a username string, got {u!r}."
                )
            clean_name = u.strip()
            if clean_name.startswith("@"):
                raise ValueError(
                    f"Invalid reviewer '{clean_name}' under label '{label_name}' in {config_path}: "
                    f"do not include '@' in usernames."
                )
            smes.append(clean_name)

        if not smes:
            raise ValueError(
                f"Label '{label_name}' in {config_path} must have at least one reviewer listed."
            )

        mapping[label_key] = LabelRule(
            name=label_name,
            smes=smes,
        )

    return mapping


def extract_approvers(pr_data: dict[str, Any]) -> set[str]:
    """Extracts lowercase usernames of approved reviewers from PR JSON data, excluding the author."""
    pr_author = pr_data.get("author", {}).get("login", "")
    author_lower = pr_author.lower() if pr_author else ""
    raw_reviews = pr_data.get("reviews")
    if raw_reviews is None:
        raw_reviews = pr_data.get("latestReviews", [])
    reviews = sorted(
        raw_reviews,
        key=lambda review: review.get("submittedAt") or "",
    )
    latest_states: dict[str, str] = {}
    for review in reviews:
        reviewer = review.get("author", {}).get("login", "").lower()
        state = review.get("state")
        if reviewer and state not in {"COMMENTED", "PENDING"}:
            latest_states[reviewer] = state

    return {
        reviewer
        for reviewer, state in latest_states.items()
        if state == "APPROVED" and reviewer != author_lower
    }


def evaluate_pr_labels(
    pr_labels: list[str],
    config_mapping: dict[str, LabelRule],
    approvers: set[str],
) -> list[LabelEvaluation]:
    """Evaluates each matching label attached to the PR against active approvers."""
    pr_label_keys = {
        label.strip().lower()
        for label in pr_labels
        if label and label.strip()
    }
    evaluations: list[LabelEvaluation] = []

    for key, rule in config_mapping.items():
        if key in pr_label_keys:
            matching_approvers = [u for u in rule.smes if u.lower() in approvers]
            evaluations.append(
                LabelEvaluation(
                    rule=rule,
                    present_on_pr=True,
                    approvers=matching_approvers,
                )
            )

    return evaluations


def find_active_override(
    pr_labels: list[str],
    override_labels: str | list[str] | tuple[str, ...] = DEFAULT_OVERRIDE_LABELS,
) -> str | None:
    """Returns the matching override label name if present on the PR, or None."""
    if isinstance(override_labels, str):
        target_list = [override_labels]
    else:
        target_list = list(override_labels)

    clean_pr_labels = {
        label.strip().lower(): label.strip()
        for label in pr_labels
        if label and label.strip()
    }
    for target in target_list:
        key = target.strip().lower()
        if key in clean_pr_labels:
            return clean_pr_labels[key]
    return None


def check_override_present(
    pr_labels: list[str],
    override_labels: str | list[str] | tuple[str, ...] = DEFAULT_OVERRIDE_LABELS,
) -> bool:
    """Checks if any override label is attached to the PR (case-insensitive)."""
    return find_active_override(pr_labels, override_labels) is not None


def is_sme_review_satisfied(
    evaluations: list[LabelEvaluation],
    overridden: bool = False,
) -> bool:
    """Returns True if overridden, no monitored labels are present, or ANY label has at least one SME approval."""
    if overridden or not evaluations:
        return True
    return any(ev.satisfied for ev in evaluations)


def generate_step_summary(
    evaluations: list[LabelEvaluation],
    pr_number: int,
    pr_title: str,
    pr_author: str,
    passed: bool,
    overridden: bool = False,
    override_label: str = DEFAULT_OVERRIDE_LABEL,
) -> str:
    """Builds a GitHub Actions Markdown Step Summary."""
    md = []
    md.append(f"## 🏷️ SME Label Review Check for PR #{pr_number}\n")
    md.append(f"**Title**: {pr_title}  ")
    md.append(f"**Author**: @{pr_author}  \n")

    if overridden:
        md.append(
            f"> ⚠️ **SME Review Requirement Bypassed**: Override label `{override_label}` is attached to this PR.\n"
        )

    if not evaluations:
        if not overridden:
            md.append(
                "> ℹ️ **No monitored SME review labels attached.**  \n"
                "> This PR does not currently require specialized subject matter expert sign-off."
            )
        return "\n".join(md)

    md.append("| Label | Status | Required SMEs | Approved By |")
    md.append("| :--- | :---: | :--- | :--- |")

    for ev in evaluations:
        label_code = f"`{ev.rule.name}`"
        req_smes = ", ".join([f"@{u}" for u in ev.rule.smes])
        if ev.satisfied:
            status_icon = "✅ Approved"
            approvers_str = ", ".join([f"@{u}" for u in ev.approvers])
        elif overridden:
            status_icon = f"⚪ Overridden (`{override_label}`)"
            approvers_str = "*None*"
        else:
            status_icon = "⚪ Satisfied (by other label)" if passed else "❌ Missing"
            approvers_str = "*None*"
        md.append(f"| {label_code} | {status_icon} | {req_smes} | {approvers_str} |")

    md.append("")
    if overridden:
        md.append(
            "> ⚠️ **SME Review Requirement Bypassed via Override Label.**  \n"
            f"> The `{override_label}` label was applied to skip SME sign-off."
        )
    elif passed:
        all_matching_approvers = sorted({f"@{u}" for ev in evaluations for u in ev.approvers})
        if all_matching_approvers:
            md.append(
                "> ✅ **SME Review Requirement Met!**  \n"
                f"> At least one designated reviewer ({', '.join(all_matching_approvers)}) has approved this PR."
            )
        else:
            md.append(
                "> ✅ **SME Review Requirement Met!**  \n"
                "> This PR does not require SME approval."
            )
    else:
        all_req_smes = sorted({f"@{u}" for ev in evaluations for u in ev.rule.smes})
        md.append(
            "> ❌ **Review Required**: Missing SME approval.  \n"
            f"> Please request review from at least one SME from any of the required label lists: {', '.join(all_req_smes)}."
        )

    return "\n".join(md)


def write_github_outputs(output_path: str, outputs: dict[str, str]) -> None:
    """Writes key-value outputs to $GITHUB_OUTPUT file."""
    try:
        with open(output_path, "a", encoding="utf-8") as f:
            for k, v in outputs.items():
                f.write(f"{k}={v}\n")
    except OSError as e:
        LOGGER.warning("Failed writing to GITHUB_OUTPUT: %s", e)


def write_step_summary(summary_path: str, summary_markdown: str) -> None:
    """Appends markdown content to $GITHUB_STEP_SUMMARY file."""
    try:
        with open(summary_path, "a", encoding="utf-8") as f:
            f.write(summary_markdown + "\n")
    except OSError as e:
        LOGGER.warning("Failed writing to GITHUB_STEP_SUMMARY: %s", e)


def sync_labels_to_github(
    repo: str,
    config_mapping: dict[str, LabelRule],
    override_labels: str | list[str] | tuple[str, ...] = DEFAULT_OVERRIDE_LABELS,
) -> list[str]:
    """Ensures all configured labels and override labels exist in the GitHub repository using gh CLI."""
    cmd = ["gh", "label", "list", "--repo", repo, "--limit", "1000", "--json", "name"]
    try:
        proc = subprocess.run(cmd, capture_output=True, text=True, check=True)
        data = json.loads(proc.stdout)
        existing_labels = {
            item["name"].strip().lower() for item in data if "name" in item
        }
    except subprocess.CalledProcessError as e:
        LOGGER.error("Could not list existing labels on %s: %s", repo, e.stderr.strip())
        raise RuntimeError(
            f"Could not list existing labels on {repo}: {e.stderr.strip()}"
        ) from e
    except Exception as e:
        LOGGER.error("Could not list existing labels on %s: %s", repo, e)
        raise RuntimeError(f"Could not list existing labels on {repo}: {e}") from e

    created: list[str] = []

    target_override_labels = (
        [override_labels] if isinstance(override_labels, str) else list(override_labels)
    )

    for o_label in target_override_labels:
        if o_label and o_label.strip().lower() not in existing_labels:
            LOGGER.info(
                "Override label '%s' does not exist on %s. Creating...",
                o_label,
                repo,
            )
            create_cmd = [
                "gh",
                "label",
                "create",
                o_label,
                "--repo",
                repo,
                "--description",
                "Override to bypass required SME reviews",
                "--color",
                "fbca04",
            ]
            try:
                subprocess.run(create_cmd, capture_output=True, text=True, check=True)
                created.append(o_label)
                LOGGER.info(
                    "✅ Successfully created override label '%s' on GitHub.",
                    o_label,
                )
            except subprocess.CalledProcessError as e:
                LOGGER.error(
                    "Could not create override label '%s': %s",
                    o_label,
                    e.stderr.strip(),
                )
                raise RuntimeError(
                    f"Could not create override label '{o_label}': {e.stderr.strip()}"
                ) from e

    for key, rule in config_mapping.items():
        if key not in existing_labels:
            LOGGER.info(
                "Label '%s' does not exist on %s. Creating...",
                rule.name,
                repo,
            )
            create_cmd = [
                "gh",
                "label",
                "create",
                rule.name,
                "--repo",
                repo,
                "--description",
                f"Requires SME review: {rule.name}",
                "--color",
                "ededed",
            ]
            try:
                subprocess.run(create_cmd, capture_output=True, text=True, check=True)
                created.append(rule.name)
                LOGGER.info(
                    "✅ Successfully created label '%s' on GitHub.",
                    rule.name,
                )
            except subprocess.CalledProcessError as e:
                LOGGER.error(
                    "Could not create label '%s': %s",
                    rule.name,
                    e.stderr.strip(),
                )
                raise RuntimeError(
                    f"Could not create label '{rule.name}': {e.stderr.strip()}"
                ) from e

    return created


def main() -> int:
    """CLI entry point for checking SME label reviewers on pull requests."""
    parser = argparse.ArgumentParser(
        description="Verify that PRs with designated labels are approved by designated SME reviewers."
    )
    parser.add_argument(
        "--pr",
        type=int,
        help="Pull request number",
    )
    parser.add_argument(
        "--repo",
        default=os.environ.get("GITHUB_REPOSITORY", DEFAULT_REPO),
        help=f"GitHub repository in owner/repo format (default: {DEFAULT_REPO})",
    )
    parser.add_argument(
        "--config",
        default=DEFAULT_CONFIG_PATH,
        help=f"Path to the YAML label reviewers file (default: {DEFAULT_CONFIG_PATH})",
    )
    parser.add_argument(
        "--sync-labels",
        action="store_true",
        help="Ensure all configured labels exist on GitHub, creating any that are missing.",
    )
    parser.add_argument(
        "--validate-config",
        action="store_true",
        help="Validate the syntax and format of the configuration file and exit.",
    )
    parser.add_argument(
        "--override-label",
        action="append",
        dest="override_labels",
        help=f"Label name that bypasses SME review checks (can be specified multiple times; default: {', '.join(DEFAULT_OVERRIDE_LABELS)})",
    )
    parser.add_argument(
        "--log-level",
        default="INFO",
        choices=["DEBUG", "INFO", "WARNING", "ERROR"],
        help="Logging level",
    )

    args = parser.parse_args()

    override_labels: list[str] = []
    if args.override_labels:
        for item in args.override_labels:
            override_labels.extend([s.strip() for s in item.split(",") if s.strip()])
    else:
        override_labels = list(DEFAULT_OVERRIDE_LABELS)

    logging.basicConfig(
        level=getattr(logging, args.log_level.upper()),
        format="%(asctime)s [%(levelname)s] %(message)s",
    )

    LOGGER.info("Loading configuration from %s...", args.config)
    try:
        config_mapping = parse_label_config(args.config)
    except Exception as e:
        LOGGER.error("Failed to load config: %s", e)
        return 2

    LOGGER.info("Configured labels with SME reviewers (%d):", len(config_mapping))
    for rule in config_mapping.values():
        LOGGER.debug("  - '%s': %s", rule.name, rule.smes)

    if args.validate_config:
        print(
            f"✅ Configuration in {args.config} is valid ({len(config_mapping)} label(s) configured)."
        )
        return 0

    if args.sync_labels:
        LOGGER.info("Syncing labels from %s to repository %s...", args.config, args.repo)
        try:
            created = sync_labels_to_github(args.repo, config_mapping, override_labels)
        except Exception as e:
            LOGGER.error("Label synchronization failed: %s", e)
            return 1
        if created:
            print(f"Created {len(created)} new label(s) on GitHub: {', '.join(created)}")
        else:
            print("All configured labels already exist on GitHub.")
        if not args.pr:
            return 0

    if not args.pr:
        LOGGER.error("PR number not provided. Pass --pr <number>.")
        return 2

    pr_number = args.pr
    LOGGER.info("Fetching PR #%d from %s...", pr_number, args.repo)
    try:
        pr_data = fetch_pull_request_data(args.repo, pr_number)
    except Exception as e:
        LOGGER.error("Failed to fetch PR #%d: %s", pr_number, e)
        return 2

    pr_title = pr_data.get("title", "")
    pr_author = pr_data.get("author", {}).get("login", "")
    pr_state = pr_data.get("state", "")
    pr_labels = [label.get("name", "") for label in pr_data.get("labels", [])]

    print("\n" + "=" * 72)
    print(f"SME Label Review Check for PR #{pr_number}: '{pr_title}'")
    print(f"Repository: {args.repo} | Author: @{pr_author} | State: {pr_state}")
    print("=" * 72)
    print(f"Active PR labels: {pr_labels}\n")

    approvers = extract_approvers(pr_data)
    LOGGER.info("Active approved reviews from: %s", sorted(approvers) or "None")

    active_override = find_active_override(pr_labels, override_labels)
    overridden = active_override is not None
    if overridden:
        print(f"⚠️  Override label '{active_override}' is present on this PR.")
        print("   SME review check is bypassed.\n" + "=" * 72)

    evaluations = evaluate_pr_labels(pr_labels, config_mapping, approvers)

    if not evaluations:
        print("ℹ️  No designated SME review labels attached to this PR.")
        print("   Check passed automatically.\n" + "=" * 72)
        passed = True
    else:
        print(f"Found {len(evaluations)} monitored SME label(s) on this PR:\n")
        passed = is_sme_review_satisfied(evaluations, overridden=overridden)
        all_matching_approvers = sorted({u for ev in evaluations for u in ev.approvers})

        for idx, ev in enumerate(evaluations, 1):
            req_smes = ", ".join([f"@{u}" for u in ev.rule.smes])
            if ev.satisfied:
                approver_mentions = ", ".join([f"@{u}" for u in ev.approvers])
                print(f"  [{idx}] Label: '{ev.rule.name}'")
                print(f"      Required SMEs: {req_smes}")
                print(f"      Status:        ✅ APPROVED by {approver_mentions}\n")
            elif overridden:
                print(f"  [{idx}] Label: '{ev.rule.name}'")
                print(f"      Required SMEs: {req_smes}")
                print(f"      Status:        ⚪ OVERRIDDEN by '{active_override}'\n")
            else:
                print(f"  [{idx}] Label: '{ev.rule.name}'")
                print(f"      Required SMEs: {req_smes}")
                if passed:
                    print(
                        f"      Status:        ⚪ Not directly reviewed (requirement satisfied by {', '.join([f'@{u}' for u in all_matching_approvers])})\n"
                    )
                else:
                    print("      Status:        ❌ MISSING APPROVAL\n")

        if not passed:
            all_smes = sorted({f"@{u}" for ev in evaluations for u in ev.rule.smes})
            print(
                f"  Action: Requires at least one approval from any of: {', '.join(all_smes)}\n"
            )
        print("=" * 72)

    # Output parameters for GitHub Actions
    gh_output = os.environ.get("GITHUB_OUTPUT")
    if gh_output:
        missing_labels = [
            ev.rule.name for ev in evaluations if not ev.satisfied
        ]
        approved_labels = [ev.rule.name for ev in evaluations if ev.satisfied]
        write_github_outputs(
            gh_output,
            {
                "passed": "true" if passed else "false",
                "overridden": "true" if overridden else "false",
                "total_monitored_labels": str(len(evaluations)),
                "missing_count": "0" if passed else str(len(missing_labels)),
                "approved_labels": ",".join(approved_labels),
                "missing_labels": "" if passed else ",".join(missing_labels),
            },
        )

    # Step summary for GitHub Actions
    gh_summary = os.environ.get("GITHUB_STEP_SUMMARY")
    if gh_summary:
        summary_md = generate_step_summary(
            evaluations,
            pr_number,
            pr_title,
            pr_author,
            passed,
            overridden=overridden,
            override_label=active_override
            or (override_labels[0] if override_labels else DEFAULT_OVERRIDE_LABEL),
        )
        write_step_summary(gh_summary, summary_md)

    if overridden:
        print(f"⚠️ BYPASSED: SME review check overridden by '{active_override}'.\n")
        return 0

    if passed:
        print("✅ SUCCESS: SME review requirement has been satisfied.\n")
        return 0

    print("❌ FAILURE: Missing required SME review approval(s).\n")
    return 1


if __name__ == "__main__":
    sys.exit(main())
