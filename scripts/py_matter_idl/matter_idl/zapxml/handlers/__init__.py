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

from .base import BaseHandler
from .context import Context
from .handlers import ConfiguratorHandler

from matter_idl.matter_idl_types import Idl


class ZapXmlHandler(BaseHandler):
    """Handles the top level (/) of a zap xml file.

       Generally these files only contain a 'configurator' element in them
    """

    def __init__(self, context: Context, idl: Idl):
        super().__init__(context)
        self._idl = idl

    def GetNextProcessor(self, name, attrs):
        if name.lower() == 'configurator':
            return ConfiguratorHandler(self.context, self._idl)
        else:
            return BaseHandler(self.context)
