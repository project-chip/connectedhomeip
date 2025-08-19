#!/usr/bin/env python3
#
# Copyright (c) 2024 Project CHIP Authors
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

import datetime
import logging
import re
from typing import Optional, Set

import click
import coloredlogs
from dateutil.tz import tzlocal
from github import Github
from github.Commit import Commit
from github.PullRequest import PullRequest

__LOG_LEVELS__ = {
    "debug": logging.DEBUG,
    "info": logging.INFO,
    "warn": logging.WARN,
    "fatal": logging.FATAL,
}

REPOSITORY = "project-chip/connectedhomeip"


class Canceller:
    def __init__(self, token: str, dry_run: bool):
        self.api = Github(token)
        self.repo = self.api.get_repo(REPOSITORY)
        self.dry_run = dry_run

    def check_all_pending_prs(self, max_age, required_runs):
        cutoff = datetime.datetime.now(tzlocal()) - max_age
        logging.info("Searching PRs updated after %s", cutoff)
        for pr in self.repo.get_pulls(state="open", sort="updated", direction="desc"):
            pr_update = pr.updated_at if pr.updated_at else pr.created_at
            pr_update = pr_update.astimezone(tzlocal())

            if pr_update < cutoff:
                logging.warning(
                    "PR is too old (since %s, cutoff at %s). Skipping the rest...",
                    pr_update,
                    cutoff,
                )
                break
            logging.info(
                "Examining PR %d updated at %s: %s", pr.number, pr_update, pr.title
            )
            self.check_pr(pr, required_runs)

    def check_pr(self, pr: PullRequest, required_runs):

        last_commit: Optional[Commit] = None

        for commit in pr.get_commits():
            logging.debug("  Found commit %s", commit.sha)
            if pr.head.sha == commit.sha:
                last_commit = commit
                break

        if last_commit is None:
            logging.error("Could not find any commit in the pull request.")
            return

        logging.info("Last commit is: %s", last_commit.sha)

        in_progress_workflows: Set[int] = set()
        failed_check_names: Set[str] = set()

        # Gather all workflows along with failed workflow names
        for check_suite in last_commit.get_check_suites():
            if check_suite.conclusion == "success":
                # Finished without errors. Nothing to do here
                continue
            for run in check_suite.get_check_runs():
                if run.conclusion is not None and run.conclusion != "failure":
                    logging.debug(
                        "    Run %s is not interesting: (state %s, conclusion %s)",
                        run.name,
                        run.status,
                        run.conclusion,
                    )
                    continue

                # TODO: I am unclear how to really find the workflow id, however the
                #       HTML URL is like https://github.com/project-chip/connectedhomeip/actions/runs/11261874698/job/31316278705
                #       so need whatever is after run.
                m = re.match(r".*/actions/runs/([\d]+)/job/.*", run.html_url)
                if not m:
                    logging.error(
                        "Failed to extract workflow number from %s", run.html_url
                    )
                    continue

                workflow_id = int(m.group(1))
                if run.conclusion is None:
                    logging.info(
                        "    Workflow %d (i.e. %s) still pending: %s",
                        workflow_id,
                        run.name,
                        run.status,
                    )
                    in_progress_workflows.add(workflow_id)
                elif run.conclusion == "failure":
                    workflow = self.repo.get_workflow_run(workflow_id)
                    logging.warning("    Workflow %s Failed", workflow.name)
                    failed_check_names.add(workflow.name)

        if not any([name in failed_check_names for name in required_runs]):
            logging.info("No critical failures found")
            return

        for id in in_progress_workflows:
            workflow = self.repo.get_workflow_run(id)
            if self.dry_run:
                logging.warning("DRY RUN: Will not stop %s", workflow.name)
            else:
                workflow.cancel()
                logging.warning("Stopping workflow %s", workflow.name)


@click.command()
@click.option(
    "--log-level",
    default="INFO",
    type=click.Choice(list(__LOG_LEVELS__.keys()), case_sensitive=False),
    help="Determines the verbosity of script output.",
)
@click.option("--gh-api-token", help="Github token to use")
@click.option("--token-file", help="Read github token from the given file")
@click.option("--dry-run", default=False, is_flag=True, help="Actually cancel or not")
@click.option(
    "--max-pr-age-days", default=0, type=int, help="How many days to look at PRs"
)
@click.option(
    "--max-pr-age-minutes", default=0, type=int, help="How many minutes to look at PRs"
)
@click.option("--require", multiple=True, default=[], help="Name of required runs")
def main(
    log_level,
    gh_api_token,
    token_file,
    dry_run,
    max_pr_age_days,
    max_pr_age_minutes,
    require,
):
    coloredlogs.install(
        level=__LOG_LEVELS__[log_level], fmt="%(asctime)s %(levelname)-7s %(message)s"
    )

    if gh_api_token:
        gh_token = gh_api_token
    elif token_file:
        gh_token = open(token_file, "rt").read().strip()
    else:
        raise Exception("Require a --gh-api-token or --token-file to access github")

    max_age = datetime.timedelta(days=max_pr_age_days, minutes=max_pr_age_minutes)
    if max_age == datetime.timedelta():
        raise Exception(
            "Please specifiy a max age of minutes or days (--max-pr-age-days or --max-pr-age-minutes)"
        )

    Canceller(gh_token, dry_run).check_all_pending_prs(max_age, require)


if __name__ == "__main__":
    main()
