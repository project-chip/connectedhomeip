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
"""Nested dictionary utilities."""

from typing import (Any, Mapping, MutableMapping, Optional)

Key = list


def get(nd: Optional[Mapping], keys: Key, default: Any = None) -> Any:
    """Get a value from a nested dictionary."""
    d: Any = nd
    while d and keys:
        d = d.get(keys[0])
        keys = keys[1:]
    if d is not None and d != {}:
        return d
    if default is not None:
        return default
    return d


def put(nd: MutableMapping, keys: Key, value: Any) -> None:
    """Store a value in a nested dictionary."""
    while True:
        key = keys[0]
        keys = keys[1:]
        if not keys:
            break
        if key not in nd:
            nd[key] = {}
        nd = nd[key]
    nd[key] = value


def update(nd: MutableMapping, src: Mapping) -> None:
    """Update a nested dictionary."""
    for k, v in src.items():
        if k not in nd:
            nd[k] = v
        elif isinstance(nd[k], dict) and isinstance(v, dict):
            update(nd[k], v)
        elif isinstance(nd[k], list):
            if isinstance(v, list):
                nd[k] += v
            else:
                nd[k].append(v)
        elif type(nd[k]) == type(v):
            nd[k] = v
        else:
            raise TypeError("type mismatch")
