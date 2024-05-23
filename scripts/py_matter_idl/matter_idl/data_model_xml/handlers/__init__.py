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
from xml.sax.xmlreader import AttributesImpl

from matter_idl.matter_idl_types import Idl

from .base import BaseHandler
from .context import Context
from .handlers import ClusterHandler
from .parsing import NormalizeName

LOGGER = logging.getLogger('data-model-xml-data-parsing')


def contains_valid_cluster_id(attrs: AttributesImpl) -> bool:
    # Does not check numeric format ... assuming scraper is smart enough for that
    return 'id' in attrs and len(attrs['id']) > 0


class DataModelXmlHandler(BaseHandler):
    """Handles the top level (/) of a data model xml file
    """

    def __init__(self, context: Context, idl: Idl):
        super().__init__(context)
        self._idl = idl

    def GetNextProcessor(self, name, attrs: AttributesImpl):
        if name.lower() == 'cluster':
            if contains_valid_cluster_id(attrs):
                return ClusterHandler.ForAttributes(self.context, self._idl, attrs)

            LOGGER.info(
                "Found an abstract base cluster (no id): '%s'", attrs['name'])

            return ClusterHandler.IntoCluster(self.context, self._idl, self.context.AddAbstractBaseCluster(NormalizeName(attrs['name']), self.context.GetCurrentLocationMeta()))
        else:
            return BaseHandler(self.context)
