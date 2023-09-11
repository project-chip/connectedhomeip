#!/usr/bin/env -S python3 -B
# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0


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
