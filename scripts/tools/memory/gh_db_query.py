#!/usr/bin/env python3
#
# Copyright (c) 2022 Project CHIP Authors
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
"""Common queries on a size database."""

import datetime
import logging
import sys
from typing import Dict, List, Mapping, Optional, Tuple, cast

import memdf.report
import memdf.util.config
import memdf.util.sqlite
import pandas as pd  # type: ignore
from memdf import Config
from memdf.sizedb import SizeDatabase

QUERY_CONFIG = {
    Config.group_map('query'): {
        'group': 'output'
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
    'query.where': {
        'help': 'SQL filter',
        'metavar': 'SQL-EXPR',
        'default': '',
        'argparse': {
            'alias': ['--where'],
        },
    },
    'query.order': {
        'help': 'sort order',
        'metavar': 'COLUMN[,COLUMN]*',
        'default': '',
        'argparse': {
            'alias': ['--order'],
        },
    },
    'query.limit': {
        'help': 'limit result size',
        'metavar': 'ROWS',
        'default': 0,
        'argparse': {
            'alias': ['--limit'],
        },
    },
}


def argsplit(metavar: str, value: str) -> Tuple[Optional[Tuple], Dict]:
    """Given comma-separated metavar and values, match them up."""
    values = tuple(value.split(','))
    names = metavar.split(',')
    if len(names) < len(values):
        logging.error('Too many values for %s', metavar)
        return (None, {})
    if len(names) > len(values):
        logging.error('Missing %s for %s', ','.join(names[len(values):]),
                      metavar)
        return (None, {})
    return (values, dict(zip(names, values)))


def postprocess_canned_sql_option(config: Config, key: str,
                                  info: Mapping) -> None:
    """Record information from simple SQL query options in one place."""
    value = config[key]
    if not value:
        return
    title = info['sql']['title']
    if isinstance(value, str):
        metavar = info.get('metavar', 'VALUE')
        if ',' in metavar:
            values, args = argsplit(metavar, value)
            if not values:
                return
        else:
            values = (value,)
            args = {metavar: value}
        title = title.format(**args)
    else:
        values = tuple()

    if config['queries'] is None:
        config['queries'] = []
    cast(list, config['queries']).append((title, key, values, info))


def make_query(config: Config, info: Mapping) -> str:
    """Construct an SQL query string for a simple SQL query option."""
    args = {'where': '', 'order': '', 'limit': ''}
    if where := config.get('query.where'):
        if kw := info['sql'].get('where'):
            args['where'] = f'{kw} {where}'
    if order := (config.get('query.order') or info['sql'].get('order')):
        args['order'] = f'ORDER BY {order}'
    if limit := config.get('query.limit'):
        args['limit'] = f'LIMIT {limit}'
    return info['sql']['query'].format(**args)


def postprocess_df_time(_config: Config, df: pd.DataFrame) -> pd.DataFrame:
    """Convert a DataFrame ‘time’ column from Unix timestamp to ISO."""
    df['time'] = df['time'].map(lambda t: datetime.datetime.utcfromtimestamp(t)
                                .isoformat())
    return df


def postprocess_df_changes(config: Config, df: pd.DataFrame) -> pd.DataFrame:
    """Given ‘parent_size’and ‘commit_size’ columns, add change columns."""
    df['change'] = df.apply(lambda row: row.commit_size - row.parent_size,
                            axis=1)
    df['% change'] = df.apply(lambda row: SizeDatabase.percent_change(
        row.parent_size, row.commit_size),
        axis=1)
    if threshold := config['report.increases']:
        df = df[df['% change'] > threshold]
    return df


QUERY_CONFIG |= {
    'query.platforms': {
        'help': 'List known platforms',
        'default': False,
        'postprocess': postprocess_canned_sql_option,
        'sql': {
            'title': 'Platforms',
            'query': '''
                SELECT DISTINCT platform FROM thing {where} {order} {limit}
                ''',
            'where': 'WHERE',
            'order': 'platform',
        },
        'argparse': {
            'alias': ['--platforms'],
        },
    },
    'query.platform-targets': {
        'help': 'List known targets for the given platform',
        'metavar': 'PLATFORM',
        'default': '',
        'postprocess': postprocess_canned_sql_option,
        'sql': {
            'title': 'Platform Targets',
            'query': '''
                SELECT DISTINCT platform, config, target
                  FROM thing
                  WHERE platform=? {where}
                  {order} {limit}
                ''',
                'where': 'AND',
                'order': 'platform, config, target',
        },
        'argparse': {
            'alias': ['--platform-targets'],
        },
    },
    'query.platform-sections': {
        'help': 'List known sections for the given platform',
        'metavar': 'PLATFORM',
        'default': '',
        'postprocess': postprocess_canned_sql_option,
        'sql': {
            'title': 'Platform Sections',
            'query': '''
                SELECT DISTINCT platform, s.name AS section
                  FROM thing t
                  INNER JOIN build b ON t.id == b.thing_id
                  INNER JOIN size s ON b.id == s.build_id
                  WHERE platform=? {where}
                  {order} {limit}
                ''',
            'where': 'AND',
            'order': 'platform, section',
        },
        'argparse': {
            'alias': ['--platform-sections'],
        },
    },
    'query.section-sizes': {
        'help': 'List size data for a given build section',
        'metavar': 'PLATFORM,CONFIG,TARGET,SECTION',
        'default': '',
        'postprocess': postprocess_canned_sql_option,
        'sql': {
            'title': 'Sizes for {PLATFORM} {CONFIG} {TARGET} {SECTION}',
            'query': '''
                SELECT DISTINCT time, hash, pr, size
                  FROM build b
                  INNER JOIN size s ON b.id == s.build_id
                  WHERE b.thing_id == (SELECT id FROM thing
                                       WHERE platform == ?
                                       AND config == ?
                                       AND target == ?)
                    AND name == ?
                    {where}
                  {order} {limit}
                ''',
            'where': 'AND',
            'order': 'time',
            'postprocess': [postprocess_df_time],
        },
    },
    'query.section-changes': {
        'help': 'List size changes for a given build section',
        'metavar': 'PLATFORM,CONFIG,TARGET,SECTION',
        'default': '',
        'postprocess': postprocess_canned_sql_option,
        'sql': {
            'title': 'Changes for {PLATFORM} {CONFIG} {TARGET} {SECTION}',
            'query': '''
                WITH builds (bid, pid, time, pr, hash) AS (
                SELECT DISTINCT b.id, p.id, b.time, b.pr, b.hash
                  FROM build b
                  INNER JOIN build p
                  ON p.hash = b.parent AND p.thing_id == b.thing_id
                  WHERE b.thing_id == (SELECT id FROM thing
                                       WHERE platform == ?
                                       AND config == ?
                                       AND target == ?)
                )
                SELECT DISTINCT
                  time, hash, pr,
                  ps.size as parent_size,
                  bs.size as commit_size
                FROM builds
                INNER JOIN size bs ON builds.bid == bs.build_id
                INNER JOIN size ps ON builds.pid == ps.build_id
                WHERE bs.name == ? AND ps.name == bs.name
                  {where}
                {order} {limit}
                ''',
            'where': 'AND',
            'order': 'time',
            'postprocess': [postprocess_df_time, postprocess_df_changes],
        },
    },
    'query.all-changes': {
        'help': 'List all size changes',
        'default': False,
        'postprocess': postprocess_canned_sql_option,
        'sql': {
            'title': 'Size Changes',
            'query': '''
                WITH
                builds (bid, pid, time, pr, hash, thing_id) AS (
                  SELECT DISTINCT b.id, p.id, b.time, b.pr, b.hash, b.thing_id
                  FROM build b
                  INNER JOIN build p
                    ON p.hash = b.parent AND p.thing_id == b.thing_id
                ),
                changes (bid, tid, name, parent_size, commit_size, change) AS (
                  SELECT DISTINCT
                    bs.build_id,
                    thing_id,
                    bs.name,
                    ps.size as parent_size,
                    bs.size as commit_size,
                    bs.size - ps.size as change
                  FROM builds
                  INNER JOIN size bs ON builds.bid == bs.build_id
                  INNER JOIN size ps ON builds.pid == ps.build_id
                  WHERE bs.name == ps.name
                )
                SELECT
                  time, hash,
                  platform, config, target, name,
                  parent_size, commit_size, change
                FROM changes
                INNER JOIN build ON bid == build.id
                INNER JOIN thing ON tid == thing.id
                {where} {order} {limit}
                ''',
            'where': 'AND',
            'order': 'time',
            'postprocess': [postprocess_df_time, postprocess_df_changes],
        },
    },
    'query.build-sizes': {
        # SQLite doesn't have PIVOT so we have to script this.
        'help': 'List size changes for a given build',
        'metavar': 'PLATFORM,CONFIG,TARGET',
        'default': '',
    },
}


def get_build_sections(db: SizeDatabase, build: str) -> Optional[Tuple]:
    """Split a build arg and get its thing_id and sections."""
    values, args = argsplit('PLATFORM,CONFIG,TARGET', build)
    if not values:
        return None

    platform = args['PLATFORM']
    pconfig = args['CONFIG']
    ptarget = args['TARGET']
    thing_id = db.select_thing_id(platform, pconfig, ptarget)
    if not thing_id:
        logging.error('No match for %s,%s,%s', platform, pconfig, ptarget)
        return None

    sections = db.select_sections_for_thing(thing_id)
    if not sections:
        logging.warning('No sections for %s,%s,%s', platform, pconfig, ptarget)
        return None

    return (platform, pconfig, ptarget, thing_id, sections)


def make_build_sizes_query(config: Config, thing_id: str,
                           sections: List[str]) -> Tuple[List[str], str]:
    """Construct and SQL query for all section sizes for a given thing."""
    # SQLite doesn't have PIVOT so we need to construct a query with
    # a column for each section.
    columns = ['time', 'hash', 'pr']
    cols = ', '.join(columns)
    joins = ''
    where = f' WHERE b.thing_id == {thing_id}'
    for i, s in enumerate(sections):
        columns.append(s)
        cols += f', s{i}.size AS s{i}z'
        joins += f' INNER JOIN size s{i} ON b.id == s{i}.build_id'
        where += f' AND s{i}.name == "{s}"'
    if qw := config['query.where']:
        where += f' AND {qw}'
    query = f'''SELECT {cols}
                  FROM build b
                  {joins}
                  {where}
                  ORDER BY {config.get('query.order') or 'time'}'''
    if limit := config['query.limit']:
        query += f' LIMIT {limit}'
    return (columns, query)


def query_build_sizes(config: Config, db: SizeDatabase,
                      build: str) -> Optional[pd.DataFrame]:
    """Get all sizes for the given build."""
    t = get_build_sections(db, build)
    if not t:
        return None
    platform, pconfig, ptarget, thing_id, sections = t

    columns, query = make_build_sizes_query(config, thing_id, sections)
    logging.debug('Query: %s', query)

    cur = db.execute(query)
    rows = cur.fetchall()
    if rows:
        df = pd.DataFrame(rows, columns=columns)
        df.attrs = {
            'name': f'qbs-{build}',
            'title': f'Sizes for {platform} {pconfig} {ptarget}',
        }
        return postprocess_df_time(config, df)

    return None


def main(argv):
    status = 0
    try:
        cfg = {
            **memdf.util.config.CONFIG,
            **memdf.util.sqlite.CONFIG,
            **memdf.report.OUTPUT_CONFIG,
            **QUERY_CONFIG,
        }
        cfg['database.file']['argparse']['required'] = True

        config = Config().init(cfg)
        config.parse(argv)

        db = SizeDatabase(config['database.file'], writable=False)
        db.open()

        dfs = {}

        q = 0
        for title, key, values, info in config.get('queries', []):
            q += 1
            query = make_query(config, info)
            logging.debug('Option: %s', key)
            logging.debug('Title: %s', title)
            logging.debug('Query: %s', query.strip())
            logging.debug('With: %s', values)
            cur = db.execute(query, values)
            columns = [i[0] for i in cur.description]
            rows = cur.fetchall()
            if rows:
                df = pd.DataFrame(rows, columns=columns)
                df.attrs = {'name': f'query{q}', 'title': title}
                for f in info['sql'].get('postprocess', []):
                    df = f(config, df)
                dfs[df.attrs['name']] = df

        if build := config['query.build-sizes']:
            q += 1
            if (df := query_build_sizes(config, db, build)) is not None:
                dfs[df.attrs['name']] = df

        if q == 0:
            config.argparse.print_help()
            return 1

        memdf.report.write_dfs(config,
                               dfs,
                               hierify=config['hierify'],
                               title=True,
                               floatfmt='5.1f')

    except Exception as exception:
        raise exception

    return status


if __name__ == '__main__':
    sys.exit(main(sys.argv))
