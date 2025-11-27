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
"""Wrapper and utility functions around sqlite3"""

import sqlite3
from typing import List, Optional

import pandas as pd  # type: ignore
from memdf import Config, ConfigDescription

CONFIG: ConfigDescription = {
    Config.group_def('database'): {
        'title': 'database options',
    },
    'database.file': {
        'help': 'Sqlite3 file',
        'metavar': 'FILENAME',
        'default': None,
        'argparse': {
            'alias': ['--db'],
        },
    },
}


class Database:
    """Wrapper and utility functions around sqlite3"""
    on_open: Optional[List[str]] = None
    on_writable: Optional[List[str]] = None

    def __init__(self, filename: str, writable: bool = True):
        self.filename = filename
        self.writable = writable
        self.con: Optional[sqlite3.Connection] = None

    def __enter__(self):
        return self.open()

    def __exit__(self, et, ev, traceback):
        self.close()
        return False

    def open(self):
        """Open and initialize the database connection."""
        if not self.con:
            db = 'file:' + self.filename
            if not self.writable:
                db += '?mode=ro'
            self.con = sqlite3.connect(db, uri=True)
            if self.on_open:
                for i in self.on_open:
                    self.con.execute(i)
            if self.writable and self.on_writable:
                for i in self.on_writable:
                    self.con.execute(i)
        return self

    def close(self):
        if self.con:
            self.con.close()
            self.con = None
        return self

    def connection(self) -> sqlite3.Connection:
        assert self.con
        return self.con

    def execute(self, query, parameters=None):
        if parameters:
            return self.con.execute(query, parameters)
        return self.con.execute(query)

    def commit(self):
        self.con.commit()
        return self

    def store(self, table: str, **kwargs):
        """Insert the data if it does not already exist."""
        q = (f"INSERT INTO {table} ({','.join(kwargs.keys())})"
             f"  VALUES ({','.join('?' * len(kwargs))})"
             f"  ON CONFLICT DO NOTHING")
        v = list(kwargs.values())
        self.connection().execute(q, v)

    def get_matching(self, table: str, columns: List[str], **kwargs):
        q = (f"SELECT {','.join(columns)} FROM {table}"
             f"  WHERE {'=? AND '.join(kwargs.keys())}=?")
        v = list(kwargs.values())
        return self.connection().execute(q, v)

    def get_matching_id(self, table: str, **kwargs):
        cur = self.get_matching(table, ['id'], **kwargs)
        row = cur.fetchone()
        if row:
            return row[0]
        return None

    def store_and_return_id(self, table: str, **kwargs) -> Optional[int]:
        self.store(table, **kwargs)
        return self.get_matching_id(table, **kwargs)

    def data_frame(self, query, parameters=None) -> pd.DataFrame:
        """Return the results of a query as a DataFrame."""
        cur = self.execute(query, parameters)
        columns = [i[0] for i in cur.description]
        df = pd.DataFrame(cur.fetchall(), columns=columns)
        self.commit()
        df.attrs = {'title': query}
        return df
