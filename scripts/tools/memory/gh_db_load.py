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
"""Fetch data from GitHub size artifacts."""

import io
import logging
import sys

import memdf.sizedb
import memdf.util.config
import memdf.util.markdown
import memdf.util.sqlite
from memdf import Config, ConfigDescription
from memdf.util.github import Gh

GITHUB_CONFIG: ConfigDescription = {
    Config.group_def('github'): {
        'title': 'github options',
    },
    'github.event': {
        'help': 'Download only event type(s) (default ‘push’)',
        'metavar': 'EVENT',
        'default': [],
        'argparse': {
            'alias': ['--event']
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
    'github.label': {
        'help': 'Download artifacts for one label only',
        'metavar': 'LABEL',
        'default': '',
    },
}


def main(argv):
    status = 0
    try:
        sqlite_config = memdf.util.sqlite.CONFIG
        sqlite_config['database.file']['argparse']['required'] = True

        config = Config().init({
            **memdf.util.config.CONFIG,
            **memdf.util.github.CONFIG,
            **sqlite_config,
            **GITHUB_CONFIG,
        })
        config.argparse.add_argument('inputs', metavar='FILE', nargs='*')
        config.parse(argv)

        db = memdf.sizedb.SizeDatabase(config['database.file']).open()

        if gh := Gh(config):

            artifact_limit = config['github.limit-artifacts']
            artifacts_added = 0
            events = config['github.event']
            if not events:
                events = ['push']
            for a in gh.get_size_artifacts(label=config['github.label']):
                if events and a.event not in events:
                    logging.debug('Skipping %s artifact %d', a.event, a.id)
                    continue
                cur = db.execute('SELECT id FROM build WHERE artifact = ?',
                                 (a.id,))
                if cur.fetchone():
                    logging.debug('Skipping known artifact %d', a.id)
                    continue
                blob = gh.download_artifact(a.id)
                if blob:
                    logging.info('Adding artifact %d %s %s %s %s',
                                 a.id, a.commit[:12], a.pr, a.event, a.group)
                    db.add_sizes_from_zipfile(io.BytesIO(blob),
                                              {'artifact': a.id})
                    db.commit()
                    artifacts_added += 1
                    if artifact_limit and artifact_limit <= artifacts_added:
                        break

        for filename in config['args.inputs']:
            db.add_sizes_from_file(filename)
            db.commit()

    except Exception as exception:
        raise exception

    return status


if __name__ == '__main__':
    sys.exit(main(sys.argv))
