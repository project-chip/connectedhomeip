# Copyright (c) 2024 Project CHIP Authors
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
set(BASE_DIR ${CMAKE_CURRENT_LIST_DIR})

# If you change this list, please ALSO CHANGE model.gni
SET(CODEGEN_DATA_MODEL_SOURCES
  "${BASE_DIR}/CodegenDataModel.cpp"
  "${BASE_DIR}/CodegenDataModel.h"
  "${BASE_DIR}/CodegenDataModel_Read.cpp"
  "${BASE_DIR}/CodegenDataModel_Write.cpp"
  "${BASE_DIR}/EmberMetadata.cpp"
  "${BASE_DIR}/EmberMetadata.h"
  "${BASE_DIR}/Instance.cpp"
)

