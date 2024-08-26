#!/usr/bin/env python3
#
# Copyright (c) 2021 Project CHIP Authors
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
"""Generate reports from size artifacts."""

import io
import logging
import re
import sys
import traceback
from typing import Dict

import fastcore  # type: ignore
import memdf.report
import memdf.sizedb
import memdf.util.config
import memdf.util.markdown
import memdf.util.sqlite
import pandas as pd  # type: ignore
from memdf import Config, ConfigDescription
from memdf.util.github import Gh

DB_CONFIG: ConfigDescription = {
    Config.group_def('database'): {
        'title': 'database options',
    },
    'database.readonly': {
        'help': 'Open database read only',
        'default': False,
        'argparse': {
            'alias': ['--db-readonly'],
        },
    },
}

GITHUB_CONFIG: ConfigDescription = {
    Config.group_def('github'): {
        'title': 'github options',
    },
    'github.comment': {
        'help': 'Send output as github PR comments',
        'default': False,
        'argparse': {
            'alias': ['--comment'],
        },
    },
    'github.limit-comments': {
        'help': 'Send no more than COUNT comments',
        'metavar': 'COUNT',
        'default': 0,
        'argparse': {
            'type': int,
        },
    },
    'github.limit-artifacts': {
        'help': 'Download no more than COUNT artifacts',
        'metavar': 'COUNT',
        'default': 0,
        'argparse': {
            'type': int,
        },
    },
    'github.limit-pr': {
        'help': 'Report only on PR, if present.',
        'metavar': 'PR',
        'default': 0,
        'argparse': {
            'type': int,
        },
    },
}

REPORT_CONFIG: ConfigDescription = {
    Config.group_map('report'): {
        'group': 'output'
    },
    'report.pr': {
        'help': 'Report on pull requests',
        'default': False,
        'argparse': {
            'alias': ['--pr', '--pull-request'],
        },
    },
    'report.push': {
        'help': 'Report on pushes',
        'default': False,
        'argparse': {
            'alias': ['--push']
        },
    },
    'report.increases': {
        'help': 'Highlight large increases',
        'metavar': 'PERCENT',
        'default': 0.0,
        'argparse': {
            'alias': ['--threshold'],
            'type': float,
        },
    },
}


class SizeContext:
    """Generate reports from size artifacts."""

    comment_format_re = re.compile(r"^<!--ghr-comment-format:(\d+)-->")

    def __init__(self, config: Config):
        self.config = config
        self.gh = Gh(config)
        db_file = config.get('database.file', ':memory:')
        self.db = memdf.sizedb.SizeDatabase(db_file,
                                            not config['database.readonly'])
        self.db.open()

    def add_sizes_from_github(self):
        """Read size report artifacts from github."""

        # Record size artifacts keyed by group and commit_hash to match them up
        # after we have the entire list.
        size_artifacts: Dict[str, Dict[str, fastcore.basics.AttrDict]] = {}
        for a in self.gh.get_size_artifacts():
            if a.group not in size_artifacts:
                size_artifacts[a.group] = {}
            size_artifacts[a.group][a.commit] = a
            logging.debug('ASG: artifact %d %s', a.id, a.name)

        # Determine required size artifacts.
        artifact_limit = self.config['github.limit-artifacts']
        required_artifact_ids: set[int] = set()
        for group, group_reports in size_artifacts.items():
            logging.debug('ASG: group %s', group)
            for report in group_reports.values():
                if self.should_report(report.event):
                    if report.parent not in group_reports:
                        logging.debug('ASN:  No match for %s', report.name)
                        continue
                    if (artifact_limit
                            and len(required_artifact_ids) >= artifact_limit):
                        continue
                    # We have size information for both this report and its
                    # parent, so ensure that both artifacts are downloaded.
                    parent = group_reports[report.parent]
                    required_artifact_ids.add(report.id)
                    required_artifact_ids.add(parent.id)
                    logging.debug('ASM:  Match %s', report.parent)
                    logging.debug('ASR:    %s %s', report.id, report.name)
                    logging.debug('ASP:    %s %s', parent.id, parent.name)

        # Download and add required artifacts.
        for i in required_artifact_ids:
            blob = self.gh.download_artifact(i)
            if blob:
                try:
                    self.db.add_sizes_from_zipfile(io.BytesIO(blob),
                                                   {'artifact': i})
                except Exception:
                    # Report in case the zipfile is invalid, however do not fail
                    # all the rest (behave as if artifact download has failed)
                    traceback.print_exc()

    def read_inputs(self):
        """Read size report from github and/or local files."""
        if self.gh:
            self.add_sizes_from_github()
        for filename in self.config['args.inputs']:
            self.db.add_sizes_from_file(filename)
        self.db.commit()
        return self

    def should_report(self, event: str = '') -> bool:
        """Return true if reporting is enabled for the action event."""
        if not event:
            return self.config['report.pr'] or self.config['report.push']
        if event == 'pull_request':
            return self.config['report.pr']
        return self.config['report.push']

    def get_existing_comment(self, pr: int, title: str):
        """Check for an existing comment."""
        existing_comment = None
        existing_comment_format = 0
        for comment in self.gh.get_comments_for_pr(pr):
            comment_parts = comment.body.partition('\n')
            if comment_parts[0].strip() == title:
                existing_comment = comment
                if m := self.comment_format_re.match(comment_parts[2]):
                    existing_comment_format = int(m.group(1))
                break
        return (existing_comment, existing_comment_format)

    def get_newest_commit(self, pr: int) -> str:
        """Get the hash of the most recent commit on the PR."""
        commits = sorted(
            self.gh.get_commits_for_pr(pr),
            key=lambda c: f'{c.commit.committer.date}{c.commit.author.date}',
            reverse=True)
        return commits[0].sha if commits else ''

    def post_change_report(self, df: pd.DataFrame) -> bool:
        """Send a change report as a github comment."""
        if not self.gh:
            return False
        pr = df.attrs['pr']

        # Check for an existing size report comment. If one exists, we'll add
        # the new information to it.
        existing_comment, existing_comment_format = self.get_existing_comment(
            pr, df.attrs['title'])

        if not existing_comment:
            # Check the most recent commit on the PR, so that we don't comment
            # for commits that are already outdated.
            commit = df.attrs['commit']
            latest = self.get_newest_commit(pr)
            if commit != latest:
                logging.info(
                    'SCS: PR #%s: not commenting for stale %s; newest is %s',
                    pr, commit, latest)
                # Return True so that the obsolete artifacts get removed.
                return True

        if existing_comment_format == 1:
            df = V1Comment.merge(df, existing_comment)
        else:
            existing_comment = None
        text = V1Comment.format(self.config, df)

        if existing_comment:
            return self.gh.update_comment(existing_comment.id, text)
        return self.gh.create_comment(pr, text)

    def report_matching_commits(self) -> Dict[str, pd.DataFrame]:
        """Report on all new comparable commits."""
        if not self.should_report():
            return {}

        comment_count = 0
        comment_limit = self.config['github.limit-comments']
        comment_enabled = (self.config['github.comment']
                           or self.config['github.dryrun-comment'])

        only_pr = self.config['github.limit-pr']

        dfs = {}
        commits = self.db.select_matching_commits()
        for event, pr, commit, parent in commits.fetchall():
            if not self.should_report(event):
                continue

            # Github doesn't have a way to fetch artifacts associated with a
            # particular PR. For testing purposes, filter to a single PR here.
            if only_pr and pr != only_pr:
                continue

            changes = self.db.select_changes(parent, commit)

            self.db.delete_builds(changes.stale_builds)
            self.gh.delete_artifacts(changes.stale_artifacts)

            if not changes.rows:
                # Matching commits had no new matching builds.
                continue

            df = pd.DataFrame(changes.rows, columns=changes.columns)

            # Filter down to region reports only.
            df = df[df['kind'] == 'region'].drop('kind', axis=1)

            df.attrs = {
                'name': f'{pr},{parent},{commit}',
                'title': (f'PR #{pr}: ' if pr else '') +
                f'Size comparison from {parent} to {commit}',
                'things': changes.things,
                'builds': changes.builds,
                'artifacts': changes.artifacts,
                'pr': pr,
                'commit': commit,
                'parent': parent,
            }
            dfs[df.attrs['name']] = df

            if (event == 'pull_request' and comment_enabled
                    and (comment_limit == 0 or comment_limit > comment_count)):
                if self.post_change_report(df):
                    # Mark the originating builds, and remove the originating
                    # artifacts, so that they don't generate duplicate report
                    # comments.
                    self.db.set_commented(df.attrs['builds'])
                    self.gh.delete_artifacts(df.attrs['artifacts'])
                    comment_count += 1
        return dfs


class V1Comment:
    """Format of a GitHub comment."""

    @staticmethod
    def format(config: Config, df: pd.DataFrame):
        """Format a GitHub comment."""

        threshold_df = None
        increase_df = df[df['change'] > 0]
        if increase_df.empty:
            increase_df = None
        elif threshold := config['report.increases']:
            threshold_df = df[df['% change'] > threshold]
            if threshold_df.empty:
                threshold_df = None

        with io.StringIO() as md:
            md.write(df.attrs['title'])
            md.write('\n<!--ghr-comment-format:1-->\n\n')

            if threshold_df is not None:
                md.write(f'**Increases above {threshold:.2g}%:**\n\n')
                md.write('<!--ghr-report:threshold-->\n\n')
                V1Comment.write_df(config, threshold_df, md)

            summary = V1Comment.summary(df)
            md.write('<details>\n')
            md.write(f'<summary>Full report ({summary})</summary>\n')
            md.write('<!--ghr-report:full-->\n\n')
            V1Comment.write_df(config, df, md)
            md.write('\n</details>\n')

            return md.getvalue()

    @staticmethod
    def summary(df: pd.DataFrame) -> str:
        count = df[['platform', 'target', 'config']].drop_duplicates().shape[0]
        platforms = ', '.join(sorted(list(set(df['platform']))))
        return f'{count} build{"" if count == 1 else "s"} for {platforms}'

    @staticmethod
    def write_df(config: Config, df: pd.DataFrame,
                 out: memdf.report.OutputOption):
        memdf.report.write_df(config,
                              df,
                              out,
                              'pipe',
                              hierify=True,
                              title=False,
                              floatfmt='5.1f')

    @staticmethod
    def merge(df: pd.DataFrame, comment) -> pd.DataFrame:
        """Merge an existing comment into the DataFrame."""
        with io.StringIO(comment.body) as body:
            for line in body:
                if line.startswith('<!--ghr-report:full-->'):
                    body.readline()  # Blank line before table.
                    cols, rows = memdf.util.markdown.read_hierified(body)
                    break
        logging.debug('REC: read %d rows', len(rows))
        attrs = df.attrs
        df = pd.concat([df, pd.DataFrame(data=rows, columns=cols).astype(df.dtypes)],
                       ignore_index=True)
        df.attrs = attrs
        return df.sort_values(
            by=['platform', 'target', 'config', 'section']).drop_duplicates()


def main(argv):
    status = 0
    try:
        config = Config().init({
            **memdf.util.config.CONFIG,
            **memdf.util.github.CONFIG,
            **memdf.util.sqlite.CONFIG,
            **memdf.report.OUTPUT_CONFIG,
            **GITHUB_CONFIG,
            **DB_CONFIG,
            **REPORT_CONFIG,
        })
        config.argparse.add_argument('inputs', metavar='FILE', nargs='*')
        config.parse(argv)

        szc = SizeContext(config)
        szc.read_inputs()
        dfs = szc.report_matching_commits()

        memdf.report.write_dfs(config,
                               dfs,
                               hierify=True,
                               title=True,
                               floatfmt='5.1f')

    except Exception as exception:
        raise exception

    return status


if __name__ == '__main__':
    sys.exit(main(sys.argv))
