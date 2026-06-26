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

import logging
import os

import click
import coloredlogs
import pathspec
import yaml
from github import Github
from github.PullRequest import PullRequest

log = logging.getLogger(__name__)

__LOG_LEVELS__ = logging.getLevelNamesMapping()

DEFAULT_REPOSITORY = "project-chip/connectedhomeip"
DEFAULT_CONFIG_PATH = ".github/platform_maintainers.yaml"

ELIGIBILITY_COMMENT_MARKER = "<!-- platform-merge-bot-eligibility-marker -->"


class PlatformGroup:
    def __init__(self, name: str, maintainers: list[str], paths: list[str]):
        self.name = name
        self.maintainers = [m.lower() for m in maintainers]
        self.paths = paths
        self.spec = pathspec.PathSpec.from_lines('gitignore', paths)

    def matches_file(self, filepath: str) -> bool:
        return self.spec.match_file(filepath)


class PlatformMergeBot:
    def __init__(self, token: str, repo_name: str, config_path: str, dry_run: bool):
        self.api = Github(token)
        self.repo = self.api.get_repo(repo_name)
        self.config_path = config_path
        self.dry_run = dry_run
        self.groups: dict[str, PlatformGroup] = {}
        self.load_config()

    def load_config(self):
        if not os.path.exists(self.config_path):
            raise FileNotFoundError(f"Config file not found at {self.config_path}")

        with open(self.config_path) as f:
            content = yaml.safe_load(f)

        if not content or not isinstance(content, dict):
            raise ValueError("Invalid config file format. Expected a YAML dictionary of groups.")

        for group_name, group_data in content.items():
            if not isinstance(group_data, dict):
                raise ValueError(f"Invalid data format for group '{group_name}'. Expected a dictionary.")
            maintainers = group_data.get('maintainers', [])
            paths = group_data.get('paths', [])
            if not isinstance(maintainers, list) or not isinstance(paths, list):
                raise ValueError(f"Group '{group_name}' must contain 'maintainers' and 'paths' lists.")
            self.groups[group_name] = PlatformGroup(group_name, maintainers, paths)

        log.info("Loaded %d platform groups from config.", len(self.groups))

    def get_pr_approvers(self, pr: PullRequest) -> set[str]:
        """Returns the set of users who have currently approved the PR."""
        approvers = set()
        change_requesters = set()

        # get_reviews() returns reviews chronologically
        for review in pr.get_reviews():
            if not review.user or not review.user.login:
                continue
            user = review.user.login.lower()
            if review.state == 'APPROVED':
                approvers.add(user)
                change_requesters.discard(user)
            elif review.state == 'CHANGES_REQUESTED':
                change_requesters.add(user)
                approvers.discard(user)
            elif review.state == 'DISMISSED':
                approvers.discard(user)
                change_requesters.discard(user)

        return approvers

    def analyze_pr_files(self, pr: PullRequest) -> tuple[bool, dict[str, list[str]], list[str]]:
        """
        Analyzes the files changed in the PR.
        Returns:
          - is_fully_covered: True if all files match at least one group.
          - matched_files_per_group: Map of group_name -> list of files matching it.
          - uncovered_files: List of files that matched no group.
        """
        matched_files_per_group: dict[str, list[str]] = {name: [] for name in self.groups}
        uncovered_files = []
        is_fully_covered = True

        # pr.get_files() returns PaginatedList of File objects
        for pr_file in pr.get_files():
            filepaths_to_check = [pr_file.filename]
            prev_filepath = getattr(pr_file, "previous_filename", None)
            if prev_filepath:
                filepaths_to_check.append(prev_filepath)

            for filepath in filepaths_to_check:
                matched_any = False
                for group_name, group in self.groups.items():
                    if group.matches_file(filepath):
                        matched_files_per_group[group_name].append(filepath)
                        matched_any = True

                if not matched_any:
                    uncovered_files.append(filepath)
                    is_fully_covered = False

        return is_fully_covered, matched_files_per_group, uncovered_files

    def check_and_process_pr(self, pr: PullRequest):
        log.info("Checking PR #%d: '%s' (Author: %s)", pr.number, pr.title, pr.user.login)

        if pr.draft:
            log.info("PR #%d is a draft. Skipping.", pr.number)
            return

        is_fully_covered, matched_files, uncovered_files = self.analyze_pr_files(pr)

        if not is_fully_covered:
            log.info(
                "PR #%d contains files outside the platform-maintained scope. Skipping. Uncovered files: %s",
                pr.number, uncovered_files[:5]
            )
            return

        # Determine which groups are active (i.e. have changed files)
        active_groups = {name: files for name, files in matched_files.items() if files}
        if not active_groups:
            log.info("PR #%d has no changed files? Skipping.", pr.number)
            return

        log.info("PR #%d is fully covered by platform groups: %s", pr.number, list(active_groups.keys()))

        # Get current approvals
        approvers = self.get_pr_approvers(pr)
        log.debug("PR #%d current approvers: %s", pr.number, list(approvers))

        # Check approvals for each active group
        missing_approvals: dict[str, PlatformGroup] = {}
        valid_approvals_per_group: dict[str, tuple[str, list[str]]] = {}  # group_name -> (approver, matched_files)

        for group_name, files in active_groups.items():
            group = self.groups[group_name]
            group_approvers = approvers.intersection(group.maintainers)
            if not group_approvers:
                missing_approvals[group_name] = group
            else:
                # Pick the first matched approver for documentation
                valid_approvals_per_group[group_name] = (list(group_approvers)[0], files)

        if missing_approvals:
            log.info(
                "PR #%d is missing approvals from platform maintainers for groups: %s",
                pr.number, list(missing_approvals.keys())
            )
            self.post_eligibility_comment(pr, active_groups, missing_approvals)
            return

        # If we got here, all active groups have at least one approval. We can merge!
        log.info("PR #%d is fully approved and ready to merge!", pr.number)
        self.merge_pr(pr, valid_approvals_per_group)

    def post_eligibility_comment(self, pr: PullRequest, active_groups: dict[str, list[str]], missing_approvals: dict[str, PlatformGroup]):
        # Check if eligibility comment already exists
        for comment in pr.get_issue_comments():
            if comment.body and ELIGIBILITY_COMMENT_MARKER in comment.body:
                log.debug("PR #%d already has an eligibility comment. Not posting duplicate.", pr.number)
                return

        # Generate comment body
        comment_body = f"{ELIGIBILITY_COMMENT_MARKER}\n"
        comment_body += "### Platform Maintainers Auto-Merge Info\n"
        comment_body += "This PR is restricted to platform-maintained paths and is eligible for auto-merging upon approval from the designated maintainers.\n\n"
        comment_body += "To merge, we require at least one approval from each of these groups:\n"

        for group_name in active_groups:
            group = self.groups[group_name]
            maintainer_mentions = ", ".join([f"@{m}" for m in group.maintainers])
            status = "❌ Needs approval" if group_name in missing_approvals else "✅ Approved"
            comment_body += f"- **{group_name}**: {maintainer_mentions} ({status})\n"
            comment_body += "  *Paths matched:*\n"
            for glob in group.paths:
                comment_body += f"    * `{glob}`\n"

        if self.dry_run:
            log.info("[Dry Run] Would post eligibility comment to PR #%d:\n%s", pr.number, comment_body)
        else:
            log.info("Posting eligibility comment to PR #%d", pr.number)
            pr.create_issue_comment(comment_body)

    def merge_pr(self, pr: PullRequest, valid_approvals_per_group: dict[str, tuple[str, list[str]]]):
        # Generate merge comment explaining reasons
        merge_reason_comment = "### Platform Maintainers Auto-Merge Executed\n"
        merge_reason_comment += "This PR has been automatically merged. It contains changes restricted to platform-maintained paths and received the required maintainer approvals:\n\n"

        for group_name, (approver, files) in valid_approvals_per_group.items():
            group = self.groups[group_name]
            # Try to find which glob patterns matched the files in this group
            matched_globs = set()
            for f in files:
                for glob in group.paths:
                    # quick check of which pattern matched this file
                    spec = pathspec.PathSpec.from_lines('gitignore', [glob])
                    if spec.match_file(f):
                        matched_globs.add(glob)

            globs_str = ", ".join([f"`{g}`" for g in matched_globs])
            merge_reason_comment += f"- **{group_name}** changes (matching {globs_str}) approved by @{approver}\n"

        if self.dry_run:
            log.info("[Dry Run] Would post merge comment to PR #%d:\n%s", pr.number, merge_reason_comment)
            log.info("[Dry Run] Would merge PR #%d (method: squash)", pr.number)
        else:
            log.info("Posting merge explanation comment to PR #%d", pr.number)
            pr.create_issue_comment(merge_reason_comment)

            log.info("Merging PR #%d", pr.number)
            # Use squash merge
            pr.merge(merge_method="squash", commit_title=f"{pr.title} (Auto-merged by platform-bot)")

    def run(self):
        log.info("Scanning open pull requests...")
        open_prs = self.repo.get_pulls(state='open')
        for pr in open_prs:
            try:
                self.check_and_process_pr(pr)
            except Exception as e:
                log.error("Error processing PR #%d: %s", pr.number, e, exc_info=True)


@click.command()
@click.option(
    "--log-level",
    default="INFO",
    type=click.Choice(list(__LOG_LEVELS__.keys()), case_sensitive=False),
    help="Determines the verbosity of script output.",
)
@click.option("--token-env", default="GH_TOKEN", help="Environment variable containing the GitHub token")
@click.option("--token-file", help="Read github token from the given file")
@click.option(
    "--repo",
    default=DEFAULT_REPOSITORY,
    help=f"Github repository name (default: {DEFAULT_REPOSITORY})",
)
@click.option(
    "--config",
    default=DEFAULT_CONFIG_PATH,
    help=f"Path to the platform maintainers yaml config (default: {DEFAULT_CONFIG_PATH})",
)
@click.option("--dry-run", default=False, is_flag=True, help="Simulate merging and commenting without executing")
def main(
    log_level,
    token_env,
    token_file,
    repo,
    config,
    dry_run,
):
    coloredlogs.install(
        level=__LOG_LEVELS__[log_level], fmt="%(asctime)s %(levelname)-7s %(message)s"
    )

    gh_token = None
    if token_file:
        with open(token_file) as f:
            gh_token = f.read().strip()
        if not gh_token:
            raise Exception(f"Token file {token_file} is empty")
    else:
        env_var = token_env or "GH_TOKEN"
        gh_token = os.environ.get(env_var)
        if not gh_token and env_var == "GH_TOKEN":
            gh_token = os.environ.get("GITHUB_TOKEN")

        if not gh_token:
            raise Exception(f"Require a token. Set environment variable '{env_var}' (or 'GITHUB_TOKEN') or provide --token-file")

    bot = PlatformMergeBot(gh_token, repo, config, dry_run)
    bot.run()


if __name__ == "__main__":
    main()
