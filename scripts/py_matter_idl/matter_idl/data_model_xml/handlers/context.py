# Copyright (c) 2023 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import logging
import xml.sax.xmlreader
from typing import List, Optional

from matter_idl.matter_idl_types import Cluster, Idl, ParseMetaData


class IdlPostProcessor:
    """Defines a callback that will apply after an entire parsing
       is complete.
    """

    def FinalizeProcessing(self, idl: Idl):
        """Update idl with any post-processing directives."""
        pass


class ProcessingPath:
    """Maintains the current path of tags within xml processing.

    As processing descents into an xml like `<configurator><cluster>....`
    paths will have contents like ['configurator', 'cluster', ...].

    The main purpose for this is to log and keep track of what was visited
    and in general to report things like 'this path found but was not handled'.
    """

    def __init__(self, paths: Optional[List[str]] = None):
        if paths is None:
            paths = []
        self.paths = paths

    def push(self, name: str):
        self.paths.append(name)

    def pop(self):
        self.paths.pop()

    def __str__(self):
        return '::'.join(self.paths)

    def __repr__(self):
        return 'ProcessingPath(%r)' % self.paths


class Context:
    """
    Contains a processing state during XML reading.

    The purpose of this is to allow elements to interact with each other, share
    data and defer processing.

    Usage:
      - globally shared data:
         > locator: parsing location, for error reporting
         > path: current ProcessingPath for any logging of where we are located
      - post-processing support:
         > can register AddIdlPostProcessor to perform some processing once
           a full parsing pass has been done

    More data may be added in time if it involves separate XML parse handlers
    needing to interact with each other.
    """

    def __init__(self, locator: Optional[xml.sax.xmlreader.Locator] = None):
        self.path = ProcessingPath()
        self.locator = locator
        self.file_name = None
        self._not_handled: set[str] = set()
        self._idl_post_processors: list[IdlPostProcessor] = []
        self.abstract_base_clusters: dict[str, Cluster] = {}

    def AddAbstractBaseCluster(self, name: str, parse_meta: Optional[ParseMetaData] = None) -> Cluster:
        """Creates a new cluster entry for the given name in the list of known
           base clusters.
        """
        assert name not in self.abstract_base_clusters  # be unique

        cluster = Cluster(name=name, code=-1, parse_meta=parse_meta)
        self.abstract_base_clusters[name] = cluster

        return cluster

    def GetCurrentLocationMeta(self) -> Optional[ParseMetaData]:
        if not self.locator:
            return None

        return ParseMetaData(line=self.locator.getLineNumber(), column=self.locator.getColumnNumber())

    def ParseLogLocation(self) -> Optional[str]:
        if not self.file_name:
            return None
        meta = self.GetCurrentLocationMeta()
        if not meta:
            return None

        return f"{self.file_name}:{meta.line}:{meta.column}"

    def MarkTagNotHandled(self):
        path = str(self.path)
        if path not in self._not_handled:
            msg = "TAG %s was not handled/recognized" % path

            where = self.ParseLogLocation()
            if where:
                msg = msg + " at " + where

            logging.warning(msg)
            self._not_handled.add(path)

    def AddIdlPostProcessor(self, processor: IdlPostProcessor, has_priority: bool = False):
        if has_priority:
            self._idl_post_processors.insert(0, processor)
        else:
            self._idl_post_processors.append(processor)

    def PostProcess(self, idl: Idl):
        for p in self._idl_post_processors:
            p.FinalizeProcessing(idl)

        self._idl_post_processors = []
