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

from matter_idl.matter_idl_types import Idl

from .base import BaseHandler
from .context import Context
from .handlers import ClusterHandler


class DataModelXmlHandler(BaseHandler):
    """Handles the top level (/) of a data model xml file
    """

    def __init__(self, context: Context, idl: Idl):
        super().__init__(context)
        self._idl = idl

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'cluster':
            return ClusterHandler(self.context, self._idl, attrs)
        else:
            return BaseHandler(self.context)
