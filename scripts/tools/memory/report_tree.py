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
"""Generate a tree representation of memory use.

This program reads memory usage information produces a textual tree
showing aggregate and/or individual memory usage by section.

Use `--collect-method=help` to see available collection methods.
Use `--limit=size` to truncate the tree.
"""

import os
import sys
from typing import Dict, Optional, Sequence

import anytree  # type: ignore
import memdf.collect
import memdf.name
import memdf.report
import memdf.select
from memdf import Config, DFs, SymbolDF


class SourceTree:
    """Representation of source tree with associated size."""

    class Node(anytree.NodeMixin):
        """Represents a source file, directory, or other user of memory."""

        def __init__(self,
                     name: str,
                     size: int = 0,
                     parent: Optional['SourceTree.Node'] = None):
            self.name = name
            self.parent = parent
            self.size = size

        def percentage(self) -> float:
            """Return percentage size of this node within its parent."""
            if not self.parent or self.parent.size == 0:
                return 100.0
            return 100 * self.size / self.parent.size

    def __init__(self, name: str):
        self.name = name
        self.root = self.Node(memdf.name.TOTAL)
        self.source_to_node: Dict[str, 'SourceTree.Node'] = {}
        self.symbol_to_node: Dict[str, 'SourceTree.Node'] = {}

    def source_node(self, source: str) -> 'SourceTree.Node':
        """Create a SourceTree.Node for a source file."""
        if not source or source == os.path.sep:
            return self.root
        head, tail = os.path.split(source)
        if not tail:
            return self.root
        if source not in self.source_to_node:
            self.source_to_node[source] = self.Node(
                tail, size=0, parent=self.source_node(head))
        return self.source_to_node[source]

    def symbol_node(self, source: str, symbol: str,
                    size: int) -> 'SourceTree.Node':
        """Create a SourceTree node for a symbol."""
        if source == symbol:
            parent = self.root
        else:
            parent = self.source_node(source)
        node = self.Node(symbol, size, parent=parent)
        self.symbol_to_node[symbol] = node
        return node

    def calculate_sizes(self) -> None:
        """Modify a newly read tree with sizes of non-leaf nodes."""
        for node in anytree.iterators.PostOrderIter(self.root):
            child_sizes = [child.size for child in node.children]
            child_size = sum(child_sizes)
            node.size += child_size

    def truncate(self, limit: int) -> None:
        """Truncate tree at size limit."""
        if limit:
            for node in anytree.iterators.PostOrderIter(self.root):
                if node.children:
                    shown_count = 0
                    hidden_size = 0
                    for child in node.children:
                        if child.size > limit:
                            shown_count += 1
                        else:
                            hidden_size += child.size
                            child.parent = None
                    if shown_count and hidden_size:
                        self.Node(memdf.name.OTHER,
                                  size=hidden_size,
                                  parent=node)

    @staticmethod
    def from_symbols(config: Config, symbols: SymbolDF,
                     tree_name: str) -> 'SourceTree':
        """Construct a SourceTree from a Memory Map DataFrame."""
        tree = SourceTree(tree_name)
        for row in symbols.itertuples():
            symbol = row.symbol
            if config['report.demangle']:
                symbol = memdf.report.demangle(symbol)
            tree.symbol_node(row.cu, symbol, row.size)
        tree.calculate_sizes()
        return tree

    def print(self) -> None:
        """Print tree hierarchically."""
        print(self.name)
        for pre, _, node in anytree.render.RenderTree(
                self.root, childiter=self._render_iter):
            print('{}{:2.0f}% {} {}'.format(pre, node.percentage(), node.size,
                                            node.name))

    @staticmethod
    def _render_iter(nodes: Sequence['SourceTree.Node']
                     ) -> Sequence['SourceTree.Node']:
        """Order for displaying child nodes: decreasing size, others at end."""
        return sorted(
            nodes,
            key=lambda n: -1 if n.name == memdf.name.OTHER else n.size,
            reverse=True)


def main(argv):
    status = 0
    try:
        config = memdf.collect.parse_args(
            {
                **memdf.select.CONFIG,
                **memdf.report.REPORT_CONFIG,
                **memdf.report.REPORT_BY_CONFIG,
            }, argv)
        config['args.need_cu'] = True
        dfs: DFs = memdf.collect.collect_files(config)

        symbols = dfs[SymbolDF.name]
        symbols = symbols[~(
            symbols.symbol.str.startswith(memdf.name.UNUSED_PREFIX)
            | symbols.symbol.str.startswith(memdf.name.OVERLAP_PREFIX))]
        by = config['report.by']
        for name in symbols[by].unique():
            tree = SourceTree.from_symbols(config,
                                           symbols.loc[symbols[by] == name],
                                           name)
            limit = (memdf.select.get_limit(config, by, name))
            tree.truncate(limit)
            print(f'\n{by.upper()}: ', end='')
            tree.print()

    except Exception as exception:
        raise exception

    return status


if __name__ == '__main__':
    sys.exit(main(sys.argv))
