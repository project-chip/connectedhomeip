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
from .parsing import AttrsToAccessPrivilege, AttrsToAttribute, ParseInt

LOGGER = logging.getLogger('data-model-xml-parser')


class ConfiguratorHandler(BaseHandler):
    """ Handling /configurator elements."""

    def __init__(self, context: Context, idl: Idl):
        super().__init__(context, handled=HandledDepth.SINGLE_TAG)
        self._idl = idl

    def GetNextProcessor(self, name: str, attrs):
        # FIXME: implement
        return BaseHandler(self.context)
