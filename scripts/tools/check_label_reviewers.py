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

Verifies that for each designated label attached to a pull request, at least one
designated SME username from the corresponding configuration has approved the PR.
Can be executed manually or as part of a GitHub Actions workflow.
"""

from __future__ import annotations

import argparse
import json
import logging
import os
import sys
import urllib.error
import urllib.request
from dataclasses import dataclass, field
from typing import Any

import yaml

DEFAULT_REPO = "project-chip/connectedhomeip"
DEFAULT_CONFIG_PATH = ".github/label_reviewers.yaml"


@dataclass
class LabelRule:
    display_name: str
    smes: set[str]
    display_smes: list[str]


@dataclass
class LabelEvaluation:
    rule: LabelRule
    present_on_pr: bool
    approvers: list[str] = field(default_factory=list)

    @property
    def satisfied(self) -> bool:
        return not self.present_on_pr or len(self.approvers) > 0


class GitHubApiClient:
    """Lightweight GitHub REST API client using standard library urllib."""

    def __init__(self, token: str | None = None) -> None:
        self.token = token

    def request(self, url: str) -> Any:
        headers = {
            "Accept": "application/vnd.github+json",
            "User-Agent": "chip-label-reviewer-action",
            "X-GitHub-Api-Version": "2022-11-28",
        }
        if self.token:
            headers["Authorization"] = f"Bearer {self.token}"

        req = urllib.request.Request(url, headers=headers)
        try:
            with urllib.request.urlopen(req, timeout=30) as resp:
                content = resp.read().decode("utf-8")
                return json.loads(content)
        except urllib.error.HTTPError as e:
            err_msg = e.read().decode("utf-8", errors="replace")
            if e.code == 404:
                raise RuntimeError(f"Resource not found at {url}") from e
            if e.code in (401, 403):
                raise RuntimeError(
                    f"GitHub API authorization/rate limit error (HTTP {e.code}): {err_msg}"
                ) from e
            raise RuntimeError(
                f"GitHub API request failed (HTTP {e.code}) for {url}: {err_msg}"
            ) from e
        except urllib.error.URLError as e:
            raise RuntimeError(f"Failed to reach GitHub API: {e.reason}") from e

    def get_pull_request(self, repo: str, pr_number: int) -> dict[str, Any]:
        url = f"https://api.github.com/repos/{repo}/pulls/{pr_number}"
        return self.request(url)

    def get_pull_request_reviews(
        self, repo: str, pr_number: int
    ) -> list[dict[str, Any]]:
        reviews = []
        page = 1
        while True:
            url = f"https://api.github.com/repos/{repo}/pulls/{pr_number}/reviews?per_page=100&page={page}"
            data = self.request(url)
            if not isinstance(data, list) or not data:
                break
            reviews.extend(data)
            if len(data) < 100:
                break
            page += 1
        return reviews

    def post(self, url: str, data: dict[str, Any]) -> Any:
        headers = {
            "Accept": "application/vnd.github+json",
            "User-Agent": "chip-label-reviewer-action",
            "X-GitHub-Api-Version": "2022-11-28",
            "Content-Type": "application/json",
        }
        if self.token:
            headers["Authorization"] = f"Bearer {self.token}"

        body = json.dumps(data).encode("utf-8")
        req = urllib.request.Request(url, data=body, headers=headers, method="POST")
        try:
            with urllib.request.urlopen(req, timeout=30) as resp:
                content = resp.read().decode("utf-8")
                return json.loads(content)
        except urllib.error.HTTPError as e:
            err_msg = e.read().decode("utf-8", errors="replace")
            raise RuntimeError(f"GitHub API POST failed (HTTP {e.code}): {err_msg}") from e


def parse_label_config(config_path: str) -> dict[str, LabelRule]:
    """Parses the YAML configuration file mapping labels to SME reviewers.

    Expected format is strictly a mapping of label names to lists of usernames:
      label_name:
        - username1
        - username2
    """
    if not os.path.exists(config_path):
        raise FileNotFoundError(f"Configuration file not found: {config_path}")

    with open(config_path, encoding="utf-8") as f:
        content = yaml.safe_load(f) or {}

    if not isinstance(content, dict):
        raise ValueError(
            f"Invalid format in {config_path}: expected a YAML mapping of label names to reviewer lists."
        )

    mapping: dict[str, LabelRule] = {}
    for label_raw, val in content.items():
        if not isinstance(label_raw, str):
            continue
        label_display = label_raw.strip()
        label_key = label_display.lower()

        if not isinstance(val, list):
            raise ValueError(
                f"Invalid format for label '{label_display}' in {config_path}: "
                f"expected a list of usernames, got {type(val).__name__}."
            )

        sme_normalized: set[str] = set()
        sme_display: list[str] = []
        for u in val:
            if isinstance(u, str) and u.strip():
                clean_name = u.strip().lstrip("@")
                sme_normalized.add(clean_name.lower())
                sme_display.append(clean_name)
            else:
                raise ValueError(
                    f"Invalid reviewer entry under label '{label_display}' in {config_path}: "
                    f"expected a username string, got {u!r}."
                )

        if not sme_normalized:
            raise ValueError(
                f"Label '{label_display}' in {config_path} must have at least one reviewer listed."
            )

        mapping[label_key] = LabelRule(
            display_name=label_display,
            smes=sme_normalized,
            display_smes=sme_display,
        )

    return mapping


def compute_effective_approvers(
    reviews: list[dict[str, Any]], author: str
) -> set[str]:
    """Computes currently approving users from chronological PR reviews.

    Respects state transitions: APPROVED -> CHANGES_REQUESTED -> APPROVED.
    PR author cannot approve their own pull request.
    """
    approvers: set[str] = set()
    change_requesters: set[str] = set()
    author_lower = author.lower() if author else ""

    for review in reviews:
        user_info = review.get("user")
        if not user_info or not user_info.get("login"):
            continue
        user = user_info["login"].lower()
        state = review.get("state")

        if state == "APPROVED":
            approvers.add(user)
            change_requesters.discard(user)
        elif state == "CHANGES_REQUESTED":
            change_requesters.add(user)
            approvers.discard(user)
        elif state == "DISMISSED":
            approvers.discard(user)
            change_requesters.discard(user)

    # Discard author from approvers
    approvers.discard(author_lower)
    return approvers


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
            matching_approvers = sorted(rule.smes.intersection(approvers))
            evaluations.append(
                LabelEvaluation(
                    rule=rule,
                    present_on_pr=True,
                    approvers=matching_approvers,
                )
            )

    return evaluations


def extract_pr_number_from_environment() -> int | None:
    """Tries to extract PR number from environment or GitHub event payload."""
    env_pr = os.environ.get("PR_NUMBER")
    if env_pr and env_pr.isdigit():
        return int(env_pr)

    event_path = os.environ.get("GITHUB_EVENT_PATH")
    if event_path and os.path.exists(event_path):
        try:
            with open(event_path, encoding="utf-8") as f:
                event_data = json.load(f)
            # From pull_request or pull_request_target event
            if "pull_request" in event_data and "number" in event_data["pull_request"]:
                return int(event_data["pull_request"]["number"])
            # From issue_comment event on a pull request
            if (
                "issue" in event_data
                and "pull_request" in event_data["issue"]
                and "number" in event_data["issue"]
            ):
                return int(event_data["issue"]["number"])
            # From workflow_dispatch inputs
            if (
                "inputs" in event_data
                and "pr_number" in event_data["inputs"]
                and str(event_data["inputs"]["pr_number"]).isdigit()
            ):
                return int(event_data["inputs"]["pr_number"])
        except Exception as e:
            logging.debug(f"Could not parse GITHUB_EVENT_PATH: {e}")

    return None


def generate_step_summary(
    evaluations: list[LabelEvaluation],
    pr_number: int,
    pr_title: str,
    pr_author: str,
    all_passed: bool,
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
        label_code = f"`{ev.rule.display_name}`"
        req_smes = ", ".join([f"@{u}" for u in ev.rule.display_smes])
        if ev.satisfied:
            status_icon = "✅ Approved"
            approvers_str = ", ".join([f"@{u}" for u in ev.approvers])
        else:
            status_icon = "❌ Missing"
            approvers_str = "*None*"
        md.append(f"| {label_code} | {status_icon} | {req_smes} | {approvers_str} |")

    md.append("")
    if all_passed:
        md.append(
            "> ✅ **All SME Review Requirements Met!**  \n"
            "> At least one designated reviewer from each required label has approved this PR."
        )
    else:
        missing_count = sum(1 for ev in evaluations if not ev.satisfied)
        md.append(
            f"> ❌ **Review Required**: {missing_count} label(s) are missing required SME approval.  \n"
            "> Please request review from at least one of the listed subject matter experts."
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


def sync_labels_to_github(
    repo: str, config_mapping: dict[str, LabelRule], client: GitHubApiClient
) -> list[str]:
    """Ensures all configured labels exist in the GitHub repository.

    Fetches existing labels and creates any missing ones via POST /repos/:owner/:repo/labels.
    """
    existing_labels = set()
    page = 1
    while True:
        url = f"https://api.github.com/repos/{repo}/labels?per_page=100&page={page}"
        data = client.request(url)
        if not isinstance(data, list) or not data:
            break
        for item in data:
            if isinstance(item, dict) and "name" in item:
                existing_labels.add(item["name"].strip().lower())
        if len(data) < 100:
            break
        page += 1

    created: list[str] = []
    for key, rule in config_mapping.items():
        if key not in existing_labels:
            logging.info(
                f"Label '{rule.display_name}' does not exist on {repo}. Creating..."
            )
            url = f"https://api.github.com/repos/{repo}/labels"
            payload = {
                "name": rule.display_name,
                "description": f"Requires SME review: {rule.display_name}",
                "color": "ededed",
            }
            try:
                client.post(url, payload)
                created.append(rule.display_name)
                logging.info(
                    f"✅ Successfully created label '{rule.display_name}' on GitHub."
                )
            except Exception as e:
                logging.warning(f"Could not create label '{rule.display_name}': {e}")

    return created


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Verify that PRs with designated labels are approved by designated SME reviewers."
    )
    parser.add_argument(
        "--pr",
        type=int,
        help="Pull request number (auto-detected if omitted in GitHub Actions)",
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
        "--token",
        default=os.environ.get("GITHUB_TOKEN") or os.environ.get("GH_TOKEN"),
        help="GitHub API token (reads GITHUB_TOKEN or GH_TOKEN env if not specified)",
    )
    parser.add_argument(
        "--sync-labels",
        action="store_true",
        help="Ensure all configured labels exist on GitHub, creating any that are missing.",
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
        logging.debug(f"  - '{rule.display_name}': {sorted(rule.smes)}")

    client = GitHubApiClient(token=args.token)

    if args.sync_labels:
        logging.info(f"Syncing labels from {args.config} to repository {args.repo}...")
        created = sync_labels_to_github(args.repo, config_mapping, client)
        if created:
            print(f"Created {len(created)} new label(s) on GitHub: {', '.join(created)}")
        else:
            print("All configured labels already exist on GitHub.")
        if not args.pr and not extract_pr_number_from_environment():
            return 0

    pr_number = args.pr or extract_pr_number_from_environment()
    if not pr_number:
        logging.error(
            "PR number not provided. Pass --pr <number> or run within a GitHub Action PR context."
        )
        return 2
    logging.info(f"Fetching PR #{pr_number} from {args.repo}...")
    try:
        pr_data = client.get_pull_request(args.repo, pr_number)
    except Exception as e:
        logging.error(f"Failed to fetch PR #{pr_number}: {e}")
        return 2

    pr_title = pr_data.get("title", "")
    pr_author = pr_data.get("user", {}).get("login", "")
    pr_state = pr_data.get("state", "")
    pr_labels = [l.get("name", "") for l in pr_data.get("labels", [])]

    print("\n" + "=" * 72)
    print(f"SME Label Review Check for PR #{pr_number}: '{pr_title}'")
    print(f"Repository: {args.repo} | Author: @{pr_author} | State: {pr_state}")
    print("=" * 72)
    print(f"Active PR labels: {pr_labels}\n")

    logging.info(f"Fetching reviews for PR #{pr_number}...")
    try:
        raw_reviews = client.get_pull_request_reviews(args.repo, pr_number)
    except Exception as e:
        logging.error(f"Failed to fetch reviews for PR #{pr_number}: {e}")
        return 2

    approvers = compute_effective_approvers(raw_reviews, author=pr_author)
    logging.info(f"Active approved reviews from: {sorted(approvers) or 'None'}")

    evaluations = evaluate_pr_labels(pr_labels, config_mapping, approvers)

    if not evaluations:
        print("ℹ️  No designated SME review labels attached to this PR.")
        print("   Check passed automatically.\n" + "=" * 72)
        all_passed = True
    else:
        print(f"Found {len(evaluations)} monitored SME label(s) on this PR:\n")
        all_passed = True
        for idx, ev in enumerate(evaluations, 1):
            req_smes = ", ".join([f"@{u}" for u in ev.rule.display_smes])
            if ev.satisfied:
                approver_mentions = ", ".join([f"@{u}" for u in ev.approvers])
                print(f"  [{idx}] Label: '{ev.rule.display_name}'")
                print(f"      Required SMEs: {req_smes}")
                print(f"      Status:        ✅ APPROVED by {approver_mentions}\n")
            else:
                all_passed = False
                print(f"  [{idx}] Label: '{ev.rule.display_name}'")
                print(f"      Required SMEs: {req_smes}")
                print("      Status:        ❌ MISSING APPROVAL")
                print(
                    f"      Action:        Requires at least one approval from: {req_smes}\n"
                )
        print("=" * 72)

    # Output parameters for GitHub Actions
    gh_output = os.environ.get("GITHUB_OUTPUT")
    if gh_output:
        missing_labels = [
            ev.rule.display_name for ev in evaluations if not ev.satisfied
        ]
        approved_labels = [ev.rule.display_name for ev in evaluations if ev.satisfied]
        write_github_outputs(
            gh_output,
            {
                "passed": "true" if all_passed else "false",
                "total_monitored_labels": str(len(evaluations)),
                "missing_count": str(len(missing_labels)),
                "approved_labels": ",".join(approved_labels),
                "missing_labels": ",".join(missing_labels),
            },
        )

    # Step summary for GitHub Actions
    gh_summary = os.environ.get("GITHUB_STEP_SUMMARY")
    if gh_summary:
        summary_md = generate_step_summary(
            evaluations, pr_number, pr_title, pr_author, all_passed
        )
        write_step_summary(gh_summary, summary_md)

    if all_passed:
        print("✅ SUCCESS: All required SME reviews have been satisfied.\n")
        return 0

    print("❌ FAILURE: Missing required SME review approval(s).\n")
    return 1


if __name__ == "__main__":
    sys.exit(main())
