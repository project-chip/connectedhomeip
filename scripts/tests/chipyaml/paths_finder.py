#!/usr/bin/env -S python3 -B
# Copyright (c) 2023 Project CHIP Authors
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


import os
import tempfile
from pathlib import Path

import click
from diskcache import Cache

_PATHS_CACHE_NAME = 'yaml_runner_cache'
_PATHS_CACHE = Cache(os.path.join(tempfile.gettempdir(), _PATHS_CACHE_NAME))

DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..', '..'))


class PathsFinder:
    def __init__(self, root_dir: str = DEFAULT_CHIP_ROOT):
        self.__root_dir = root_dir

    def get(self, target_name: str) -> str:
        path = _PATHS_CACHE.get(target_name)
        if path and Path(path).is_file():
            return path

        if path:
            del _PATHS_CACHE[target_name]

        for path in Path(self.__root_dir).rglob(target_name):
            if not path.is_file() or path.name != target_name:
                continue

            _PATHS_CACHE[target_name] = str(path)
            return str(path)

        return None


@click.group()
def finder():
    pass


@finder.command()
def view():
    """View the cache entries."""
    for name in _PATHS_CACHE:
        print(click.style(f'{name}', bold=True) + f':\t{_PATHS_CACHE[name]}')


@finder.command()
@click.argument('key', type=str)
@click.argument('value', type=str)
def add(key: str, value: str):
    """Add a cache entry."""
    _PATHS_CACHE[key] = value


@finder.command()
@click.argument('name', type=str)
def delete(name: str):
    """Delete a cache entry."""
    if name in _PATHS_CACHE:
        del _PATHS_CACHE[name]


@finder.command()
def reset():
    """Delete all cache entries."""
    for name in _PATHS_CACHE:
        del _PATHS_CACHE[name]


@finder.command()
@click.argument('name', type=str)
def search(name: str):
    """Search for a target and add it to the cache."""
    paths_finder = PathsFinder()
    path = paths_finder.get(name)
    if path:
        print(f'The target "{name}" has been added with the value "{path}".')
    else:
        print(f'The target "{name}" was not found.')


if __name__ == '__main__':
    finder()
