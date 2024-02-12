/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#pragma once

#include <app/AttributeAccessInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/WriteHandler.h>
#include <app/util/attribute-metadata.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLVReader.h>

namespace chip {
namespace app {

/**
 * Returns the metadata of the attribute for the given path.
 *
 * @retval The metadata of the attribute, will return null if the given attribute does not exists.
 */
const EmberAfAttributeMetadata * GetAttributeMetadata(const ConcreteAttributePath & aPath);

/**
 *  Get the registered attribute access override. nullptr when attribute access override is not found.
 *
 * TODO(#16806): This function and registerAttributeAccessOverride can be member functions of InteractionModelEngine.
 */
AttributeAccessInterface * GetAttributeAccessOverride(EndpointId aEndpointId, ClusterId aClusterId);

/**
 * TODO: Document.
 */
CHIP_ERROR WriteSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor,
                                  const ConcreteDataAttributePath & aAttributePath, TLV::TLVReader & aReader,
                                  WriteHandler * apWriteHandler);

} // namespace app
} // namespace chip
