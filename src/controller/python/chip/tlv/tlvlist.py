#!/usr/bin/env python3
# coding=utf-8

#
#   Copyright (c) 2023 Project CHIP Authors
#   All rights reserved.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

import dataclasses
import enum
from typing import Any, Iterator, List, Tuple, Union


class TLVList:
    """Represents a list in CHIP TLV.

    A TLVList can be constructed from a `list` of tuples of tag and value. `None` stands for "anonymous tag".

    e.g.
    ```
    l = TLVList([(1, 'a'), (2, 'b'), (None, 'c')])
    ```

    Constructs a list of three items, tag 1 is 'a', tag 2 is 'b' and with an anonymous item 'c'.

    Since TLVLists are ordered, it is meanful to iterate over an list:

    e.g.
    ```
    for tag, val in l:
        print(f"tag={tag}, val={val}")
    ```

    Outputs:
    ```
    tag=1, val=a
    tag=2, val=b
    tag=None, val=c
    ```

    One can also append items into an list:

    e.g.
    ```
    l.append(3, 'd')
    ```

    The content of `l` will be `[(1, 'a'), (2, 'b'), (None, 'c'), (3, 'd')]`

    One can access an item in the list via the tag.

    e.g.
    ```
    val = l[1]
    # val is 'a'
    ```

    It is also possible to get an item via the index since it is ordered:

    e.g.
    ```
    tag, val = l[TLVList.IndexMethod.Tag:2]
    # tag is None, val is 'c'
    ```
    """

    @dataclasses.dataclass
    class TLVListItem:
        tag: Union[None, int]
        value: Any

        def as_tuple(self):
            return (self.tag, self.value)

        def as_rich_repr_tuple(self):
            if self.tag is None:
                return "Anonymous", repr(self.value)
            else:
                return str(self.tag), repr(self.value)

        def __repr__(self):
            if self.tag is None:
                return "Anonymous: " + repr(self.value)
            else:
                return str(self.tag) + ": " + repr(self.value)

        def __rich_repr__(self):
            yield self.as_rich_repr_tuple()

    class IndexMethod(enum.Enum):
        Index = 0
        Tag = 1

    class Iterator:
        def __init__(self, iter: Iterator):
            self._iterator = iter

        def __iter__(self):
            return self

        def __next__(self):
            res = next(self._iterator)
            return res.tag, res.value

    def __init__(self, items: List[Tuple[Union[int, None], Any]] = []):
        """Constructs a TLVList.

        items: A list of tuples for the tag and value for the items in the TLVList.
        """
        self._data: List[TLVList.TLVListItem] = []

        for tag, val in items:
            self.append(tag, val)

    def _get_item_by_tag(self, tag) -> Any:
        if not isinstance(tag, int):
            raise ValueError("Tag should be a integer for non-anonymous fields.")
        for data in self._data:
            if data.tag == tag:
                return data.value
        raise KeyError(f"Tag {tag} not found in the list.")

    def __getitem__(self, access) -> Any:
        """Gets a item in the list by the tag or the index.

        Examples:
        ```
            tlv_list[1]                           # returns the item in the list with tag `1`
            tlv_list[TLVList.IndexMethod.Tag:2]   # returns the item in the list with tag `2`
            tlv_list[TLVList.IndexMethod.Index:0] # returns the tag and value of the first item in the list
        ```
        """
        if isinstance(access, slice):
            tag, index = access.start, access.stop
            if tag == TLVList.IndexMethod.Tag:
                return self._get_item_by_tag(index)
            elif tag == TLVList.IndexMethod.Index:
                return self._data[index].as_tuple()
            raise ValueError("Method should be TLVList.IndexMethod.Tag or TLVList.IndexMethod.Index")
        elif isinstance(access, int):
            return self._get_item_by_tag(access)
        raise ValueError("Invalid access method")

    def append(self, tag: Union[None, int], value: Any) -> None:
        """Appends an item to the list."""
        if (tag is not None) and (not isinstance(tag, int)):
            raise KeyError(f"Tag should be a integer or none for anonymous tag, {type(tag)} got")
        self._data.append(TLVList.TLVListItem(tag, value))

    def __repr__(self):
        return "TLVList" + repr(self._data)

    def __rich_repr__(self):
        for items in self._data:
            yield items.as_rich_repr_tuple()

    def __iter__(self) -> """TLVList.Iterator""":
        return TLVList.Iterator(iter(self._data))

    def __eq__(self, rhs: "TLVList") -> bool:
        if not isinstance(rhs, TLVList):
            return False
        return self._data == rhs._data
