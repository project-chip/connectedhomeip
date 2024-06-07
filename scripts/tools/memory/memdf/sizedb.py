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
"""Sqlite3 database of binary sizes over time."""

import collections
import json
import logging
import sqlite3
import zipfile
from pathlib import Path
from typing import IO, Dict, Iterable, List, Optional, Union

import memdf.util.sqlite

ChangeInfo = collections.namedtuple('ChangeInfo', [
    'columns', 'rows', 'things', 'builds', 'stale_builds', 'artifacts',
    'stale_artifacts'
])


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
        -- A ‘size’ entry gives the size of an area for a particular build.
        CREATE TABLE IF NOT EXISTS size (
            build_id    INTEGER REFERENCES build(id),
            kind        TEXT NOT NULL,      -- Area kind
            name        TEXT NOT NULL,      -- Area name
            size        INTEGER NOT NULL,   -- Size in bytes
            PRIMARY KEY (build_id, name)
        )
        """
    ]

    def __init__(self, filename: str, writable: bool = True):
        super().__init__(filename, writable)

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
        # Add section and region sizes.
        for frame in ['section', 'region']:
            for i in r['frames'].get(frame, []):
                r['sizes'].append({
                    'name': i[frame],
                    'size': i['size'],
                    'kind': frame
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
            with open(path, encoding='utf-8') as f:
                self.add_sizes_from_json(f.read(), origin)
        elif path.suffix == '.zip':
            logging.info('ASZ: reading ZIP %s', path)
            self.add_sizes_from_zipfile(path, origin)
        else:
            logging.warning('Unknown file type "%s" ignored', filename)

    def select_thing_id(self, platform: str, config: str,
                        target: str) -> Optional[str]:
        cur = self.execute(
            'SELECT id FROM thing WHERE platform=? AND config=? AND target=?',
            (platform, config, target))
        row = cur.fetchone()
        return row[0] if row else None

    def select_sections_for_thing(self, thing: str) -> List[str]:
        cur = self.execute(
            '''
            SELECT DISTINCT name FROM size WHERE build_id = (
            SELECT DISTINCT id FROM build WHERE thing_id == ?)
            ORDER BY name
            ''', (thing,))
        return [row[0] for row in cur.fetchall()]

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

    def select_changes(self, parent: str, commit: str) -> ChangeInfo:
        """Returns size changes between the given commits."""
        cur = self.execute(
            '''
        SELECT DISTINCT
            t.id AS thing,
            cb.artifact AS artifact,
            pb.id AS parent_build,
            cb.id AS commit_build,
            t.platform, t.config, t.target,
            cs.kind AS kind,
            cs.name AS name,
            ps.size AS parent_size,
            cs.size AS commit_size,
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

        keep = ('platform', 'target', 'config', 'kind', 'name', 'parent_size',
                'commit_size')
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
                parent_size = row['parent_size']
                commit_size = row['commit_size']
                new.append(commit_size - parent_size)
                new.append(self.percent_change(parent_size, commit_size))
                rows.append(new)
                artifacts.add(row['artifact'])
                builds.add(row['commit_build'])

        return ChangeInfo(('platform', 'target', 'config', 'kind', 'section',
                           parent[:8], commit[:8], 'change', '% change'), rows,
                          things, builds, stale_builds, artifacts,
                          stale_artifacts)

    def set_commented(self, build_ids: Iterable[int]):
        """Set the commented flag for the given builds."""
        if not build_ids:
            return
        for build_id in build_ids:
            self.execute('UPDATE build SET commented = 1 WHERE id = ?',
                         (build_id, ))
        self.commit()

    def delete_builds(self, build_ids: Iterable[int]):
        """Delete the given builds."""
        if not build_ids:
            return
        for build_id in build_ids:
            self.execute('DELETE FROM size WHERE build_id = ?', (build_id, ))
            self.execute('DELETE FROM build WHERE id = ?', (build_id, ))
        self.commit()

    @staticmethod
    def percent_change(a: int, b: int) -> float:
        if a == 0:
            return 0.0 if b == 0 else float('inf')
        return 100. * (b - a) / a
