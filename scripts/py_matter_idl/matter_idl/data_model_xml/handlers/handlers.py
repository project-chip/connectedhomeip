# Copyright (c) 2022 Project CHIP Authors
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
from typing import Any, Optional

from matter_idl.matter_idl_types import (Attribute, Bitmap, Cluster, ClusterSide, Command, CommandQuality, ConstantEntry, DataType,
                                         Enum, Event, EventPriority, EventQuality, Field, FieldQuality, Idl, Struct, StructQuality,
                                         StructTag)

from .base import BaseHandler, HandledDepth
from .context import Context, IdlPostProcessor
from .parsing import ParseInt

LOGGER = logging.getLogger('data-model-xml-parser')


class ClusterHandler(BaseHandler):
    """ Handling /cluster elements."""

    def __init__(self, context: Context, idl: Idl, attrs):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._idl = idl

        assert("name" in attrs)
        assert("id" in attrs)

        self._cluster = Cluster(
            side=ClusterSide.CLIENT,
            name=attrs["name"],
            code=ParseInt(attrs["id"]),
            parse_meta=context.GetCurrentLocationMeta()
        )

    def EndProcessing(self):
        self._idl.clusters.append(self._cluster)

    def GetNextProcessor(self, name: str, attrs):
        # TODO:
        #   - features
        #   - dataTypes (enum, bitmap, list???, struct)
        #   - attributes
        #   - commands
        #   - events
        #   - ??? classification
        #   - ??? identifiers
        if name == "revisionHistory":
            # Revision history COULD be used to find the latest revision of a cluster
            # however current IDL files do NOT have a revision info field
            return BaseHandler(self.context, handled=HandledDepth.ENTIRE_TREE)
        else:
            return BaseHandler(self.context)
