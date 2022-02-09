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
import itertools
import json
import logging
import os
import os.path
import re
import sqlite3
import sys
import zipfile

from pathlib import Path
from typing import Dict, IO, Iterable, Optional, Union

import dateutil  # type: ignore
import fastcore  # type: ignore
import ghapi.all  # type: ignore
import pandas as pd  # type: ignore

import memdf.report
import memdf.util.config
import memdf.util.sqlite
from memdf import Config, ConfigDescription

GITHUB_CONFIG: ConfigDescription = {
    Config.group_def('github'): {
        'title': 'github options',
    },
    'github.token': {
        'help': 'Github API token, or "SKIP" to suppress connecting to github',
        'metavar': 'TOKEN',
        'default': '',
        'argparse': {
            'alias': ['--github-api-token', '--token'],
        },
    },
    'github.repository': {
        'help': 'Github repostiory',
        'metavar': 'OWNER/REPO',
        'default': '',
        'argparse': {
            'alias': ['--repo'],
        },
    },
    'github.comment': {
        'help': 'Send output as github PR comments',
        'default': False,
        'argparse': {
            'alias': ['--comment'],
        },
    },
    'github.keep': {
        'help': 'Leave PR artifacts after commenting',
        'default': False,
        'argparse': {
            'alias': ['--keep'],
        },
    },
    'github.dryrun-comment': {
        'help': 'Dry run for sending output as github PR comments',
        'default': False,
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
    'github.limit-artifact-pages': {
        'help': 'Examine no more than COUNT pages of artifacts',
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
    'report.query': {
        'help': 'Run an SQL query',
        'default': [],
        'argparse': {
            'alias': ['--query', '--sql']
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


class SizeDatabase(memdf.util.sqlite.Database):
    """A database for recording and comparing size reports."""
    on_open = ["PRAGMA foreign_keys = ON", "PRAGMA encoding = 'UTF-8'"]
    on_writable = [
        """
        -- A ‘thing’ identifies the kind of built object.
        -- Builds of the same thing are comparable.
        CREATE TABLE IF NOT EXISTS thing (
            id          INTEGER PRIMARY KEY,
            platform    TEXT NOT NULL,  -- Build platform
            config      TEXT NOT NULL,  -- Build configuration discriminator
            target      TEXT NOT NULL,  -- Build target
            UNIQUE(platform, config, target)
        )
        """, """
        -- A ‘build’ identifies a built instance of a thing at some point.
        CREATE TABLE IF NOT EXISTS build (
            id          INTEGER PRIMARY KEY,
            thing_id    INTEGER REFERENCES thing(id),
            hash        TEXT NOT NULL,      -- Commit hash
            parent      TEXT NOT NULL,      -- Parent commit hash
            pr          INTEGER DEFAULT 0,  -- Github PR number
            time        INTEGER NOT NULL,   -- Unix-epoch timestamp
            artifact    INTEGER DEFAULT 0,  -- Github artifact ID
            commented   INTEGER DEFAULT 0,  -- 1 if recorded in a GH comment
            ref         TEXT,               -- Target git ref
            event       TEXT,               -- Github build trigger event
            UNIQUE(thing_id, hash, parent, pr, time, artifact)
        )
        """, """
        -- A ‘size’ entry gives the size of a section for a particular build.
        CREATE TABLE IF NOT EXISTS size (
            build_id    INTEGER REFERENCES build(id),
            name        TEXT NOT NULL,      -- Section name
            size        INTEGER NOT NULL,   -- Section size in bytes
            PRIMARY KEY (build_id, name)
        )
        """
    ]

    def __init__(self, config: Config):
        super().__init__(config['database.file'])
        self.config = config
        self.gh = gh_open(config)
        self.deleted_artifacts: set[int] = set()

    def add_sizes(self, **kwargs):
        """
        Add a size report to the database.

        The incoming arguments must contain the required non-ID column names
        from ‘thing’ and ‘build’ tables, plus a 'sizes' entry that is a
        sequence of mappings containing 'name' and 'size'.
        """
        td = {k: kwargs[k] for k in ('platform', 'config', 'target')}
        thing = self.store_and_return_id('thing', **td)
        bd = {k: kwargs[k] for k in ('hash', 'parent', 'time', 'event')}
        if 'ref' in kwargs:
            bd['ref'] = kwargs['ref']
        cd = {k: kwargs.get(k, 0) for k in ('pr', 'artifact', 'commented')}
        build = self.store_and_return_id('build', thing_id=thing, **bd, **cd)
        if build is None:
            logging.error('Failed to store %s %s %s', thing, bd, cd)
        else:
            for d in kwargs['sizes']:
                self.store('size', build_id=build, **d)

    def add_sizes_from_json(self, s: Union[bytes, str], origin: Dict):
        """Add sizes from a JSON size report."""
        r = origin.copy()
        r.update(json.loads(s))
        r['sizes'] = []
        # Add section sizes.
        for i in r['frames'].get('section', []):
            r['sizes'].append({'name': i['section'], 'size': i['size']})
        # Add segment sizes.
        for i in r['frames'].get('wr', []):
            r['sizes'].append({
                'name': ('(read only)', '(read/write)')[int(i['wr'])],
                'size': i['size']
            })
        self.add_sizes(**r)

    def add_sizes_from_zipfile(self, f: Union[IO, Path], origin: Dict):
        """Add size reports from a zip."""
        with zipfile.ZipFile(f, 'r') as zip_file:
            for i in zip_file.namelist():
                if i.endswith('-sizes.json'):
                    origin['member'] = i
                    with zip_file.open(i) as member:
                        self.add_sizes_from_json(member.read(), origin)

    def add_sizes_from_file(self, filename: str):
        """Add size reports from a file."""
        origin = {'file': filename}
        path = Path(filename)
        if path.suffix == '.json':
            logging.info('ASJ: reading JSON %s', path)
            with open(path) as f:
                self.add_sizes_from_json(f.read(), origin)
        elif path.suffix == '.zip':
            logging.info('ASZ: reading ZIP %s', path)
            self.add_sizes_from_zipfile(path, origin)
        else:
            logging.warning('Unknown file type "%s" ignored', filename)

    def add_sizes_from_github(self):
        """Read size report artifacts from github."""
        if not self.gh:
            return

        artifact_limit = self.config['github.limit-artifacts']
        artifact_pages = self.config['github.limit-artifact-pages']

        # Size artifacts have names of the form:
        #   Size,{group},{pr},{commit_hash},{parent_hash}[,{event}]
        # Record them keyed by group and commit_hash to match them up
        # after we have the entire list.
        page = 0
        size_artifacts: Dict[str, Dict[str, fastcore.basics.AttrDict]] = {}
        for i in ghapi.all.paged(self.gh.actions.list_artifacts_for_repo, 100):
            if not i.artifacts:
                break
            for a in i.artifacts:
                if a.name.startswith('Size,') and a.name.count(',') >= 4:
                    _, group, pr, commit, parent, *etc = a.name.split(',')
                    a.parent = parent
                    a.pr = pr
                    a.created_at = dateutil.parser.isoparse(a.created_at)
                    # Old artifact names don't include the event.
                    if etc:
                        event = etc[0]
                    else:
                        event = 'push' if pr == '0' else 'pull_request'
                    a.event = event
                    if group not in size_artifacts:
                        size_artifacts[group] = {}
                    size_artifacts[group][commit] = a
                    logging.debug('ASG: artifact %d %s', a.id, a.name)
            page += 1
            logging.debug('ASP: artifact page %d of %d', page, artifact_pages)
            if artifact_pages and page >= artifact_pages:
                break

        # Determine required size artifacts.
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
            logging.debug('ASD: download artifact %d', i)
            try:
                blob = self.gh.actions.download_artifact(i, 'zip')
            except Exception as e:
                blob = None
                logging.error('Failed to download artifact %d: %s', i, e)
            if blob:
                self.add_sizes_from_zipfile(io.BytesIO(blob), {'artifact': i})

    def read_inputs(self):
        """Read size report from github and/or local files."""
        self.add_sizes_from_github()
        for filename in self.config['args.inputs']:
            self.add_sizes_from_file(filename)
        self.commit()

    def select_matching_commits(self):
        """Find matching builds, where one's commit is the other's parent."""
        return self.execute('''
            SELECT DISTINCT
                c.event as event,
                c.pr AS pr,
                c.hash AS hash,
                p.hash AS parent
              FROM build c
              INNER JOIN build p ON p.hash = c.parent
              WHERE c.commented = 0
              ORDER BY c.time DESC, c.pr, c.hash, p.hash
            ''')

    def set_commented(self, build_ids: Iterable[int]):
        """Set the commented flag for the given builds."""
        if not build_ids:
            return
        for build_id in build_ids:
            self.execute('UPDATE build SET commented = 1 WHERE id = ?',
                         (build_id, ))
        self.commit()

    def delete_stale_builds(self, build_ids: Iterable[int]):
        """Delete stale builds."""
        if not build_ids:
            return
        for build_id in build_ids:
            logging.info('DSB: deleting obsolete build %d', build_id)
            self.execute('DELETE FROM size WHERE build_id = ?', (build_id, ))
            self.execute('DELETE FROM build WHERE id = ?', (build_id, ))
        self.commit()

    def delete_artifact(self, artifact_id: int):
        if (self.gh and artifact_id
                and artifact_id not in self.deleted_artifacts):
            self.deleted_artifacts.add(artifact_id)
            try:
                self.gh.actions.delete_artifact(artifact_id)
            except Exception:
                # During manual testing we sometimes lose the race against CI.
                logging.error('Failed to delete artifact %d', artifact_id)

    def delete_stale_artifacts(self, stale_artifacts: Iterable[int]):
        if not self.config['github.keep']:
            for artifact_id in stale_artifacts:
                logging.info('DSA: deleting obsolete artifact %d', artifact_id)
                self.delete_artifact(artifact_id)

    def should_report(self, event: Optional[str] = None) -> bool:
        """Return true if reporting is enabled for the event."""
        if event is None:
            return self.config['report.pr'] or self.config['report.push']
        if event == 'pull_request':
            return self.config['report.pr']
        return self.config['report.push']


def gh_open(config: Config) -> Optional[ghapi.core.GhApi]:
    """Return a GhApi, if so configured."""
    gh: Optional[ghapi.core.GhApi] = None
    if config['github.repository']:
        owner, repo = config.get('github.repository').split('/', 1)
        config.put('github.owner', owner)
        config.put('github.repo', repo)
        if not config['github.token']:
            config['github.token'] = os.environ.get('GITHUB_TOKEN')
            if not config['github.token']:
                logging.error('Missing --github-token')
                return None
        token = config['github.token']
        if token != 'SKIP':
            gh = ghapi.all.GhApi(owner=owner,
                                 repo=repo,
                                 token=config['github.token'])
    return gh


def gh_get_comments_for_pr(gh: ghapi.core.GhApi, pr: int):
    return itertools.chain.from_iterable(
        ghapi.all.paged(gh.issues.list_comments, pr))


def gh_get_commits_for_pr(gh: ghapi.core.GhApi, pr: int):
    return itertools.chain.from_iterable(
        ghapi.all.paged(gh.pulls.list_commits, pr))


def percent_change(a: int, b: int) -> float:
    if a == 0:
        return 0.0 if b == 0 else float('inf')
    return 100. * (b - a) / a


def changes_for_commit(db: SizeDatabase, pr: int, commit: str,
                       parent: str) -> pd.DataFrame:
    """Return a DataFrame with size changes between the given commits."""
    cur = db.execute(
        '''
       SELECT DISTINCT
           t.id AS thing,
           cb.artifact AS artifact,
           pb.id AS parent_build,
           cb.id AS commit_build,
           t.platform, t.config, t.target,
           cs.name AS name,
           ps.size AS parent_size,
           cs.size AS commit_size,
           cs.size - ps.size AS change,
           cb.time AS time
         FROM thing t
         INNER JOIN build cb ON cb.thing_id = t.id
         INNER JOIN build pb ON pb.thing_id = t.id AND pb.hash = cb.parent
         INNER JOIN size cs ON cs.build_id = cb.id
         INNER JOIN size ps ON ps.build_id = pb.id AND cs.name = ps.name
         WHERE cb.hash = ? AND pb.hash = ?
         ORDER BY t.platform, t.config, t.target,
                  cs.name, cb.time DESC, pb.time DESC
       ''', (commit, parent))

    keep = ('platform', 'target', 'config', 'name', 'parent_size',
            'commit_size', 'change')
    things: set[int] = set()
    artifacts: set[int] = set()
    builds: set[int] = set()
    stale_builds: set[int] = set()
    stale_artifacts: set[int] = set()
    previous: Optional[sqlite3.Row] = None
    rows = []

    for row in cur.fetchall():
        row = sqlite3.Row(cur, row)
        things.add(row['thing'])
        if (previous is not None and row['thing'] == previous['thing']
                and row['name'] == previous['name']):
            # This is duplicate build, older because we sort descending,
            # presumably from a partial workflow re-run.
            if row['parent_build'] != previous['parent_build']:
                stale_builds.add(row['parent_build'])
            if row['commit_build'] != previous['commit_build']:
                stale_builds.add(row['commit_build'])
                stale_artifacts.add(row['artifact'])
        else:
            previous = row
            new = [row[k] for k in keep]
            new.append(percent_change(row['parent_size'], row['commit_size']))
            rows.append(new)
            artifacts.add(row['artifact'])
            builds.add(row['commit_build'])

    db.delete_stale_builds(stale_builds)
    db.delete_stale_artifacts(stale_artifacts)

    df = pd.DataFrame(rows,
                      columns=('platform', 'target', 'config', 'section',
                               parent[:8], commit[:8], 'change', '% change'))
    df.attrs = {
        'name': f'{pr},{parent},{commit}',
        'title': (f'PR #{pr}: ' if pr else '') +
        f'Size comparison from {parent} to {commit}',
        'things': things,
        'builds': builds,
        'artifacts': artifacts,
        'pr': pr,
        'commit': commit,
        'parent': parent,
    }
    return df


comment_format_re = re.compile(r"^<!--ghr-comment-format:(\d+)-->")


def gh_send_change_report(db: SizeDatabase, df: pd.DataFrame) -> bool:
    """Send a change report as a github comment."""

    if not db.gh:
        return False

    # Look for an existing comment for this change.
    pr = df.attrs['pr']

    # Check the most recent commit on the PR, so that we don't comment on
    # builds that are already outdated.
    commit = df.attrs['commit']
    commits = sorted(
        gh_get_commits_for_pr(db.gh, pr),
        key=lambda c: f'{c.commit.committer.date}{c.commit.author.date}',
        reverse=True)
    if commits and commit != commits[0].sha:
        logging.info('SCS: PR #%s: not commenting for stale %s; newest is %s',
                     pr, commit, commits[0].sha)
        # Return True so that the obsolete artifacts get removed.
        return True

    # Check for an existing size report comment. If one exists, we'll add
    # the new report to it.
    title = df.attrs['title']
    existing_comment = None
    existing_comment_format = 0
    for comment in gh_get_comments_for_pr(db.gh, pr):
        comment_parts = comment.body.partition('\n')
        if comment_parts[0].strip() == title:
            existing_comment = comment
            if m := comment_format_re.match(comment_parts[2]):
                existing_comment_format = int(m.group(1))
            break

    if existing_comment_format != 1:
        existing_comment = None
    text = gh_comment_v1(db, df, existing_comment)

    logging.info(
        'SCR: %s %s', df.attrs['title'],
        f'updating comment {existing_comment.id}'
        if existing_comment else 'as new comment')

    if db.config['github.dryrun-comment']:
        logging.debug('%s', text)
        return False

    try:
        if existing_comment:
            db.gh.issues.update_comment(existing_comment.id, text)
        else:
            db.gh.issues.create_comment(pr, text)
        return True
    except Exception:
        return False


def gh_comment_v1(db: SizeDatabase, df: pd.DataFrame, existing_comment) -> str:
    """Format a github comment."""

    if existing_comment:
        df = v1_comment_merge(df, existing_comment)

    threshold_df = None
    increase_df = df[df['change'] > 0]
    if increase_df.empty:
        increase_df = None
    elif threshold := db.config['report.increases']:
        threshold_df = df[df['% change'] > threshold]
        if threshold_df.empty:
            threshold_df = None
    decrease_df = df[df['change'] < 0]
    if decrease_df.empty:
        decrease_df = None

    with io.StringIO() as md:
        md.write(df.attrs['title'])
        md.write('\n<!--ghr-comment-format:1-->\n\n')

        if threshold_df is not None:
            md.write(f'**Increases above {threshold:.2g}%:**\n\n')
            md.write('<!--ghr-report:threshold-->\n\n')
            v1_comment_write_df(db, threshold_df, md)

        if increase_df is not None:
            summary = v1_comment_summary(increase_df)
            md.write('<details>\n')
            md.write(f'<summary>Increases ({summary})</summary>\n')
            md.write('<!--ghr-report:increases-->\n\n')
            v1_comment_write_df(db, increase_df, md)
            md.write('</details>\n\n')

        if decrease_df is not None:
            summary = v1_comment_summary(decrease_df)
            md.write('<details>\n')
            md.write(f'<summary>Decreases ({summary})</summary>\n')
            md.write('<!--ghr-report:decreases-->\n\n')
            v1_comment_write_df(db, decrease_df, md)
            md.write('</details>\n\n')

        summary = v1_comment_summary(df)
        md.write('<details>\n')
        md.write(f'<summary>Full report ({summary})</summary>\n')
        md.write('<!--ghr-report:full-->\n\n')
        v1_comment_write_df(db, df, md)
        md.write('\n</details>\n')

        return md.getvalue()


def v1_comment_merge(df: pd.DataFrame, comment) -> pd.DataFrame:
    with io.StringIO(comment.body) as body:
        for line in body:
            if line.startswith('<!--ghr-report:full-->'):
                body.readline()  # Blank line before table.
                header, rows = read_hierified(body)
                break
    logging.debug('REC: read %d rows', len(rows))
    df = df.append(pd.DataFrame(data=rows, columns=header).astype(df.dtypes))
    return df.sort_values(
        by=['platform', 'target', 'config', 'section']).drop_duplicates()


def read_hierified(f):
    """Read a markdown table in ‘hierified’ format."""

    line = f.readline()
    header = tuple((s.strip() for s in line.split('|')[1:-1]))

    _ = f.readline()  # The line under the header.

    rows = []
    for line in f:
        line = line.strip()
        if not line:
            break
        row = []
        columns = line.split('|')
        for i in range(0, len(header)):
            column = columns[i + 1].strip()
            if not column:
                column = rows[-1][i]
            row.append(column)
        rows.append(tuple(row))

    return (header, rows)


def v1_comment_write_df(db: SizeDatabase, df: pd.DataFrame,
                        out: memdf.report.OutputOption):
    memdf.report.write_df(db.config,
                          df,
                          out,
                          'pipe',
                          hierify=True,
                          title=False,
                          floatfmt='5.1f')


def v1_comment_summary(df: pd.DataFrame) -> str:
    count = df[['platform', 'target', 'config']].drop_duplicates().shape[0]
    platforms = ', '.join(sorted(list(set(df['platform']))))
    return f'{count} build{"" if count == 1 else "s"} for {platforms}'


def report_matching_commits(db: SizeDatabase) -> Dict[str, pd.DataFrame]:
    """Report on all new comparable commits."""
    if not db.should_report():
        return {}

    comment_count = 0
    comment_limit = db.config['github.limit-comments']
    comment_enabled = (db.config['github.comment']
                       or db.config['github.dryrun-comment'])

    only_pr = db.config['github.limit-pr']

    dfs = {}
    for event, pr, commit, parent in db.select_matching_commits().fetchall():
        if not db.should_report(event):
            continue

        # Github doesn't have a way to fetch artifacts associated with a
        # particular PR. For testing purposes, filter to a single PR here.
        if only_pr and pr != only_pr:
            continue

        df = changes_for_commit(db, pr, commit, parent)
        if df.empty:
            # Matching commits had no new matching builds.
            continue
        dfs[df.attrs['name']] = df

        if (event == 'pull_request' and comment_enabled
                and (comment_limit == 0 or comment_limit > comment_count)):
            if gh_send_change_report(db, df):
                # Mark the originating builds, and remove the originating
                # artifacts, so that they don't generate duplicate report
                # comments.
                db.set_commented(df.attrs['builds'])
                if not db.config['github.keep']:
                    for artifact_id in df.attrs['artifacts']:
                        logging.info('RMC: deleting artifact %d', artifact_id)
                        db.delete_artifact(artifact_id)
                comment_count += 1
    return dfs


def report_queries(db: SizeDatabase) -> Dict[str, pd.DataFrame]:
    """Perform any requested SQL queries."""
    dfs = {}
    q = 0
    for query in db.config['report.query']:
        q += 1
        cur = db.execute(query)
        columns = [i[0] for i in cur.description]
        rows = cur.fetchall()
        if rows:
            df = pd.DataFrame(rows, columns=columns)
            df.attrs = {'name': f'query{q}', 'title': query}
            dfs[df.attrs['name']] = df
        db.commit()
    return dfs


def main(argv):
    status = 0
    try:
        config = Config().init({
            **memdf.util.config.CONFIG,
            **memdf.util.sqlite.CONFIG,
            **memdf.report.OUTPUT_CONFIG,
            **GITHUB_CONFIG,
        })
        config.argparse.add_argument('inputs', metavar='FILE', nargs='*')
        config.parse(argv)

        dfs = {}
        with SizeDatabase(config) as db:
            db.read_inputs()
            dfs.update(report_matching_commits(db))
            dfs.update(report_queries(db))

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
