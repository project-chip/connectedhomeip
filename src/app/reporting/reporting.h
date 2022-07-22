/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/****************************************************************************
 * @file
 * @brief Definitions for the Reporting plugin.
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include <app/ConcreteAttributePath.h>
#include <app/util/af-types.h>

/** @brief Reporting Attribute Change
 *
 * This function is called by the framework when an attribute managed by the
 * framework changes.  The application should call this function when an
 * externally-managed attribute changes.  The application should use the change
 * notification to inform its reporting decisions.
 */
void MatterReportingAttributeChangeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId,
                                            EmberAfAttributeType type, uint8_t * data);

/*
 * Same but with just an attribute path and no data available.
 */
void MatterReportingAttributeChangeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId);

/*
 * Same but with a nicer attribute path.
 */
void MatterReportingAttributeChangeCallback(const chip::app::ConcreteAttributePath & aPath);

/*
 * Same but only with an EndpointId, this is used when adding / enabling an endpoint during runtime.
 */
void MatterReportingAttributeChangeCallback(chip::EndpointId endpoint);
