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


@dataclass
class LabelRule:
    name: str
    smes: list[str]


@dataclass
class LabelEvaluation:
    rule: LabelRule
    present_on_pr: bool
    approvers: list[str] = field(default_factory=list)

    @property
    def satisfied(self) -> bool:
        return not self.present_on_pr or len(self.approvers) > 0


def fetch_pull_request_data(repo: str, pr_number: int) -> dict[str, Any]:
    """Fetches pull request details and latest reviews using the gh CLI."""
    cmd = [
        "gh",
        "pr",
        "view",
        str(pr_number),
        "--repo",
        repo,
        "--json",
        "author,title,state,labels,latestReviews",
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
            content = yaml.safe_load(f) or {}
    except yaml.YAMLError as e:
        raise ValueError(f"YAML syntax error in {config_path}: {e}") from e

    if not isinstance(content, dict):
        raise ValueError(
            f"Invalid format in {config_path}: expected a YAML mapping of label names to reviewer lists."
        )

    mapping: dict[str, LabelRule] = {}
    for label_raw, val in content.items():
        if not isinstance(label_raw, str):
            continue
        label_name = label_raw.strip()
        label_key = label_name.lower()

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
    return {
        r.get("author", {}).get("login", "").lower()
        for r in pr_data.get("latestReviews", [])
        if r.get("state") == "APPROVED"
        and r.get("author", {}).get("login", "").lower() != author_lower
    }


def evaluate_pr_labels(
    pr_labels: list[str],
    config_mapping: dict[str, LabelRule],
    approvers: set[str],
) -> list[LabelEvaluation]:
    """Evaluates each matching label attached to the PR against active approvers."""
    pr_label_keys = {l.strip().lower() for l in pr_labels if l and l.strip()}
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


def is_sme_review_satisfied(evaluations: list[LabelEvaluation]) -> bool:
    """Returns True if no monitored labels are present, or if ANY label has at least one SME approval."""
    if not evaluations:
        return True
    return any(ev.satisfied for ev in evaluations)


def generate_step_summary(
    evaluations: list[LabelEvaluation],
    pr_number: int,
    pr_title: str,
    pr_author: str,
    passed: bool,
) -> str:
    """Builds a GitHub Actions Markdown Step Summary."""
    md = []
    md.append(f"## 🏷️ SME Label Review Check for PR #{pr_number}\n")
    md.append(f"**Title**: {pr_title}  ")
    md.append(f"**Author**: @{pr_author}  \n")

    if not evaluations:
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
        else:
            status_icon = "⚪ Satisfied (by other label)" if passed else "❌ Missing"
            approvers_str = "*None*"
        md.append(f"| {label_code} | {status_icon} | {req_smes} | {approvers_str} |")

    md.append("")
    if passed:
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
        logging.warning(f"Failed writing to GITHUB_OUTPUT: {e}")


def write_step_summary(summary_path: str, summary_markdown: str) -> None:
    """Appends markdown content to $GITHUB_STEP_SUMMARY file."""
    try:
        with open(summary_path, "a", encoding="utf-8") as f:
            f.write(summary_markdown + "\n")
    except OSError as e:
        logging.warning(f"Failed writing to GITHUB_STEP_SUMMARY: {e}")


def sync_labels_to_github(repo: str, config_mapping: dict[str, LabelRule]) -> list[str]:
    """Ensures all configured labels exist in the GitHub repository using gh CLI."""
    cmd = ["gh", "label", "list", "--repo", repo, "--limit", "1000", "--json", "name"]
    try:
        proc = subprocess.run(cmd, capture_output=True, text=True, check=True)
        data = json.loads(proc.stdout)
        existing_labels = {
            item["name"].strip().lower() for item in data if "name" in item
        }
    except Exception as e:
        logging.warning(f"Could not list existing labels on {repo}: {e}")
        existing_labels = set()

    created: list[str] = []
    for key, rule in config_mapping.items():
        if key not in existing_labels:
            logging.info(
                f"Label '{rule.name}' does not exist on {repo}. Creating..."
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
                logging.info(
                    f"✅ Successfully created label '{rule.name}' on GitHub."
                )
            except subprocess.CalledProcessError as e:
                logging.warning(
                    f"Could not create label '{rule.name}': {e.stderr.strip()}"
                )

    return created


def main() -> int:
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
        "--log-level",
        default="INFO",
        choices=["DEBUG", "INFO", "WARNING", "ERROR"],
        help="Logging level",
    )

    args = parser.parse_args()

    logging.basicConfig(
        level=getattr(logging, args.log_level.upper()),
        format="%(asctime)s [%(levelname)s] %(message)s",
    )

    logging.info(f"Loading configuration from {args.config}...")
    try:
        config_mapping = parse_label_config(args.config)
    except Exception as e:
        logging.error(f"Failed to load config: {e}")
        return 2

    logging.info(f"Configured labels with SME reviewers ({len(config_mapping)}):")
    for key, rule in config_mapping.items():
        logging.debug(f"  - '{rule.name}': {rule.smes}")

    if args.validate_config:
        print(
            f"✅ Configuration in {args.config} is valid ({len(config_mapping)} label(s) configured)."
        )
        return 0

    if args.sync_labels:
        logging.info(f"Syncing labels from {args.config} to repository {args.repo}...")
        created = sync_labels_to_github(args.repo, config_mapping)
        if created:
            print(f"Created {len(created)} new label(s) on GitHub: {', '.join(created)}")
        else:
            print("All configured labels already exist on GitHub.")
        if not args.pr:
            return 0

    if not args.pr:
        logging.error("PR number not provided. Pass --pr <number>.")
        return 2

    pr_number = args.pr
    logging.info(f"Fetching PR #{pr_number} from {args.repo}...")
    try:
        pr_data = fetch_pull_request_data(args.repo, pr_number)
    except Exception as e:
        logging.error(f"Failed to fetch PR #{pr_number}: {e}")
        return 2

    pr_title = pr_data.get("title", "")
    pr_author = pr_data.get("author", {}).get("login", "")
    pr_state = pr_data.get("state", "")
    pr_labels = [l.get("name", "") for l in pr_data.get("labels", [])]

    print("\n" + "=" * 72)
    print(f"SME Label Review Check for PR #{pr_number}: '{pr_title}'")
    print(f"Repository: {args.repo} | Author: @{pr_author} | State: {pr_state}")
    print("=" * 72)
    print(f"Active PR labels: {pr_labels}\n")

    approvers = extract_approvers(pr_data)
    logging.info(f"Active approved reviews from: {sorted(approvers) or 'None'}")

    evaluations = evaluate_pr_labels(pr_labels, config_mapping, approvers)

    if not evaluations:
        print("ℹ️  No designated SME review labels attached to this PR.")
        print("   Check passed automatically.\n" + "=" * 72)
        passed = True
    else:
        print(f"Found {len(evaluations)} monitored SME label(s) on this PR:\n")
        passed = is_sme_review_satisfied(evaluations)
        all_matching_approvers = sorted({u for ev in evaluations for u in ev.approvers})

        for idx, ev in enumerate(evaluations, 1):
            req_smes = ", ".join([f"@{u}" for u in ev.rule.smes])
            if ev.satisfied:
                approver_mentions = ", ".join([f"@{u}" for u in ev.approvers])
                print(f"  [{idx}] Label: '{ev.rule.name}'")
                print(f"      Required SMEs: {req_smes}")
                print(f"      Status:        ✅ APPROVED by {approver_mentions}\n")
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
            evaluations, pr_number, pr_title, pr_author, passed
        )
        write_step_summary(gh_summary, summary_md)

    if passed:
        print("✅ SUCCESS: SME review requirement has been satisfied.\n")
        return 0

    print("❌ FAILURE: Missing required SME review approval(s).\n")
    return 1


if __name__ == "__main__":
    sys.exit(main())
