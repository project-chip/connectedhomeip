/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>

EmberAfStatus OtaRequestorServerSetUpdateState(chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum value);
EmberAfStatus OtaRequestorServerGetUpdateState(chip::EndpointId endpointId,
                                               chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum & value);
EmberAfStatus OtaRequestorServerSetUpdateStateProgress(chip::app::DataModel::Nullable<uint8_t> value);
EmberAfStatus OtaRequestorServerGetUpdateStateProgress(chip::EndpointId endpointId,
                                                       chip::app::DataModel::Nullable<uint8_t> & value);

void OtaRequestorServerOnStateTransition(chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum previousState,
                                         chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum newState,
                                         chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum reason,
                                         chip::app::DataModel::Nullable<uint32_t> const & targetSoftwareVersion);
void OtaRequestorServerOnVersionApplied(uint32_t softwareVersion, uint16_t productId);
void OtaRequestorServerOnDownloadError(uint32_t softwareVersion, uint64_t bytesDownloaded,
                                       chip::app::DataModel::Nullable<uint8_t> progressPercent,
                                       chip::app::DataModel::Nullable<int64_t> platformCode);
