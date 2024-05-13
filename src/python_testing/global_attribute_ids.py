#
#    Copyright (c) 2023 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

# This file should be removed once we have a good way to get this from the codegen or XML

from enum import IntEnum


class GlobalAttributeIds(IntEnum):
    ATTRIBUTE_LIST_ID = 0xFFFB
    ACCEPTED_COMMAND_LIST_ID = 0xFFF9
    GENERATED_COMMAND_LIST_ID = 0xFFF8
    FEATURE_MAP_ID = 0xFFFC
    CLUSTER_REVISION_ID = 0xFFFD

    def to_name(self) -> str:
        if self == GlobalAttributeIds.ATTRIBUTE_LIST_ID:
            return "AttributeList"
        if self == GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID:
            return "AcceptedCommandList"
        if self == GlobalAttributeIds.GENERATED_COMMAND_LIST_ID:
            return "GeneratedCommandList"
        if self == GlobalAttributeIds.FEATURE_MAP_ID:
            return "FeatureMap"
        if self == GlobalAttributeIds.CLUSTER_REVISION_ID:
            return "ClusterRevision"
