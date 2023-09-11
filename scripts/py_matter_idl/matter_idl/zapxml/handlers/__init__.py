# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

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
